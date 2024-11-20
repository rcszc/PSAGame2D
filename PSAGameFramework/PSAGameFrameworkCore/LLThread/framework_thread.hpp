// framework_thread. THREAD_POOL:[移植模块]
// EasyWorksPool(改), https://github.com/rcszc/EasyWorksPool
// Source: RCSZ 2023_08_28, Modify: RCSZ 2024_04_05.

#ifndef _FRAMEWORK_THREAD_HPP
#define _FRAMEWORK_THREAD_HPP
// LLThread PSA-L.5 (=>logger).
#include "../LLLogger/framework_logger.hpp"

// PSAG thread(task) pool.
namespace PSAG_THREAD_POOL {
    StaticStrLABEL PSAG_THREAD_LABEL = "PSAG_THREAD";

    template<typename TRES>
    struct DoubleBufferResource {
        std::atomic<bool> SwapBufferIndex = false;

        TRES* GetProduceBuffer() { return &DoubleBuffer[(size_t)SwapBufferIndex];  }
        TRES* GetConsumeBuffer() { return &DoubleBuffer[(size_t)!SwapBufferIndex]; }

        void SwapResourceBuffers() {
            SwapBufferIndex = !SwapBufferIndex;
        }
        TRES DoubleBuffer[2] = {};
    };

    // run_time_type_information.
    struct RTTI_OBJECT_INFO {
        std::string ObjectName;
        std::size_t ObjectHash;
    };

    // get this_thread hash unique_id.
    size_t ThisThreadID();

    namespace Error {
        typedef const char* ERRINFO;

        // 异常处理. std::exception::what, std::exception::name
        class TPerror :public std::exception {
        protected:
            std::string ErrorMessage = {};
            std::string ErrorCompName = {};
        public:
            TPerror(ERRINFO message, size_t pid, ERRINFO compname);

            ERRINFO what() const noexcept override { return ErrorMessage.c_str(); }
            ERRINFO name() const noexcept { return ErrorCompName.c_str(); }
        };
    }

    template<typename nClass>
    inline RTTI_OBJECT_INFO _OBJECT_RTTI(nClass TASK_OBJ) {
        RTTI_OBJECT_INFO ReturnInfo = {};

        const std::type_info& InfoGet = typeid(TASK_OBJ);

        ReturnInfo.ObjectName = InfoGet.name();
        ReturnInfo.ObjectHash = InfoGet.hash_code();

        return ReturnInfo;
    }

    class PsagThreadTasks {
    protected:
        std::vector<std::thread>          ThreadWorkers;
        std::queue<std::function<void()>> PoolTasks;
        std::mutex                        PoolMutex;
        std::condition_variable           WorkersCondition;
        std::atomic<uint32_t>             WorkingThreadsCount{NULL};

        void ThreadsTaskExecution(uint32_t workers_num);
        void ThreadsTaskFree();

        bool PauseFlag = false;
        // current creation object_info.
        RTTI_OBJECT_INFO OBJECT_INFO = {};

    public:
        PsagThreadTasks(uint32_t init_workers) {
            ThreadsTaskExecution(init_workers);
            PSAG_LOGGER::PushLogger(LogInfo, PSAG_THREAD_LABEL, "create thread_pool workers: %u", init_workers);
        };
        ~PsagThreadTasks() {
            ThreadsTaskFree();
            PSAG_LOGGER::PushLogger(LogInfo, PSAG_THREAD_LABEL, "close(free) thread_pool workers.");
        };

        // thread_pool: push => tasks queue.
        template<typename InClass, typename... ArgsParam>
        std::future<std::shared_ptr<InClass>> PushTask(ArgsParam... Args) {
            auto TaskObject = std::make_shared<std::packaged_task<std::shared_ptr<InClass>()>>(
                [Args = std::make_tuple(std::forward<ArgsParam>(Args)...), this]() mutable {
                    try {
                        return std::apply([](auto&&... Args) {
                            return std::make_shared<InClass>(std::forward<decltype(Args)>(Args)...);
                            }, std::move(Args));
                    }
                    catch (...) {
                        throw Error::TPerror("failed create object.", ThisThreadID(), "CREATE_OBJ");
                        return std::shared_ptr<InClass>(nullptr);
                    }
                });
            // create object => get object_info.
            OBJECT_INFO = _OBJECT_RTTI(TaskObject);

            std::future<std::shared_ptr<InClass>> ResultAsync = TaskObject->get_future();
            {
                std::unique_lock<std::mutex> Lock(PoolMutex);
                if (PauseFlag) {
                    // disable push task.
                    throw Error::TPerror("failed thread pool stop.", ThisThreadID(), "CREATE_OBJ");
                    return ResultAsync;
                }
                else
                    PoolTasks.emplace([TaskObject]() { (*TaskObject)(); });
            }
            WorkersCondition.notify_one();

            return ResultAsync;
        }

        RTTI_OBJECT_INFO GetCreateObjectInfo() {
            std::unique_lock<std::mutex> Lock(PoolMutex);
            return OBJECT_INFO;
        }

        uint32_t GetWorkingThreadsCount();
        uint32_t GetTaskQueueCount();
        void     ResizeWorkers(uint32_t resize);
    };
}

// PSAG thread(calc) function: SIMD AVX-256, X86-64.
namespace PSAG_THREAD_SIMD {
#ifdef ENABLE_AVX256_CALC_FUNC
    // device intel-avx-isa.
#include <immintrin.h>
#define AVX256_32B_COUNT 8

    // imm register 256bit float32 * 8, 32bytes.
    using PsagFP32SIMD256 = __m256;

    PsagFP32SIMD256 PsagSimd8Fp32Load    (float* dataptr, size_t offset, size_t bytes = 32);
    PsagFP32SIMD256 PsagSimd8Fp32FillLoad(float* dataptr, size_t count_32b);
    void             PsagSimd8Fp32Store  (const PsagFP32SIMD256& vec, float* dataptr, size_t offset, size_t bytes = 32);

    class PSAG_SIMD_LOAD {
    protected:
        std::vector<float>* DatasetPointer = nullptr;
    public:
        PSAG_SIMD_LOAD(std::vector<float>* ref) : DatasetPointer(ref) {};
        size_t GET_SIMD256_ITEMS_NUMBER() {
            return DatasetPointer->size() / AVX256_32B_COUNT;
        }
        PsagFP32SIMD256 LOAD_SIMD256_ITEM(size_t index) {
            return PsagSimd8Fp32Load(DatasetPointer->data(), index * AVX256_32B_COUNT);
        }
    };

    class PSAG_SIMD_STORE {
    protected:
        std::vector<float>* DatasetPointer = nullptr;
    public:
        PSAG_SIMD_STORE(std::vector<float>* ref) : DatasetPointer(ref) {};
        size_t GET_SIMD256_ITEMS_NUMBER() {
            return DatasetPointer->size() / AVX256_32B_COUNT;
        }
        void STORE_SIMD256_ITEM(const PsagFP32SIMD256& value, size_t index) {
            return PsagSimd8Fp32Store(value, DatasetPointer->data(), index * AVX256_32B_COUNT);
        }
    };

    PsagFP32SIMD256 operator+(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);
    PsagFP32SIMD256 operator-(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);
    PsagFP32SIMD256 operator*(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);
    PsagFP32SIMD256 operator/(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);

    PsagFP32SIMD256& operator+=(PsagFP32SIMD256& a, PsagFP32SIMD256 b);
    PsagFP32SIMD256& operator-=(PsagFP32SIMD256& a, PsagFP32SIMD256 b);
    PsagFP32SIMD256& operator*=(PsagFP32SIMD256& a, PsagFP32SIMD256 b);
    PsagFP32SIMD256& operator/=(PsagFP32SIMD256& a, PsagFP32SIMD256 b);

    // fma(fused multiply-add), res = a * b + c, [matrix calc].
    PsagFP32SIMD256 PsagSimdCalcFMADD(PsagFP32SIMD256 A, PsagFP32SIMD256 B, PsagFP32SIMD256 C);

    PsagFP32SIMD256 PsagSimdCalcFABS(PsagFP32SIMD256 A);
    PsagFP32SIMD256 PsagSimdCalcSQRT(PsagFP32SIMD256 A);
    enum LIMIT_CALC_TYPE {
        ModeMax = 1 << 1,
        ModeMin = 1 << 2
    };
    PsagFP32SIMD256 PsagSimdCalcLIMIT(PsagFP32SIMD256 A, PsagFP32SIMD256 B, LIMIT_CALC_TYPE MODE);

    template <int mask>
    // avx-isa: blend values based mask [0,255].
    PsagFP32SIMD256 PsagSimdCalcBLEND(PsagFP32SIMD256 A, PsagFP32SIMD256 B) {
        return _mm256_blend_ps(A, B, mask);
    }

    PsagFP32SIMD256 PsagSimdCalcRSQRT(PsagFP32SIMD256 A);
    PsagFP32SIMD256 PsagSimdCalcRECI (PsagFP32SIMD256 A);
    PsagFP32SIMD256 PsagSimdCalcNEG  (PsagFP32SIMD256 A);
    PsagFP32SIMD256 PsagSimdCalcHADD (PsagFP32SIMD256 A, PsagFP32SIMD256 B);
    PsagFP32SIMD256 PsagSimdCalcDOT  (PsagFP32SIMD256 A, PsagFP32SIMD256 B);

    bool operator<(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);
    bool operator>(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);

    bool operator<=(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);
    bool operator>=(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);
    bool operator==(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s);
#endif
}

#endif