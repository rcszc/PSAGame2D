// framework_thread. THREAD_POOL:[移植模块]
// EasyWorksPool(改), https://github.com/rcszc/EasyWorksPool
// Source: RCSZ 2023_08_28, Modify: RCSZ 2024_04_05.

#ifndef _FRAMEWORK_THREAD_HPP
#define _FRAMEWORK_THREAD_HPP
// LLThread PSA-L.5 (=>logger).
#include "../LLLogger/framework_logger.hpp"

#define PSAG_THREAD_LABEL "PSAG_THREAD" 

// PSAG thread(task) pool.
namespace PSAG_THREAD_POOL {
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

// PSAG thread(calc) function: SIMD AVX-256.
namespace PSAG_THREAD_SIMD {
#ifdef ENABLE_AVX256_CALC_FUNC
    // device intel-avx-isa.
#include <immintrin.h>
#define AVX256_32B_COUNT 8

    // imm register 256bit float32 * 8, 32bytes.
    using PsagFp32Simd256b = __m256;

    PsagFp32Simd256b PsagSimdLoad8Fp32(float* dataptr, size_t offset, size_t bytes = 32);
    PsagFp32Simd256b PsagSimdLoad8Fp32Fill(float* dataptr, size_t count_32b);

    PsagFp32Simd256b PsagSimdCalcADD(PsagFp32Simd256b A, PsagFp32Simd256b B);
    PsagFp32Simd256b PsagSimdCalcSUB(PsagFp32Simd256b A, PsagFp32Simd256b B);
    PsagFp32Simd256b PsagSimdCalcMUL(PsagFp32Simd256b A, PsagFp32Simd256b B);
    PsagFp32Simd256b PsagSimdCalcDIV(PsagFp32Simd256b A, PsagFp32Simd256b B);

    // fma(fused multiply-add), res = a * b + c, [matrix calc].
    PsagFp32Simd256b PsagSimdCalcFMADD(PsagFp32Simd256b A, PsagFp32Simd256b B, PsagFp32Simd256b C);

    PsagFp32Simd256b PsagSimdCalcFABS(PsagFp32Simd256b A);
    PsagFp32Simd256b PsagSimdCalcSQRT(PsagFp32Simd256b A);
    enum LIMIT_CALC_TYPE {
        ModeMax = 1 << 1,
        ModeMin = 1 << 2
    };
    PsagFp32Simd256b PsagSimdCalcLIMIT(PsagFp32Simd256b A, PsagFp32Simd256b B, LIMIT_CALC_TYPE MODE);
#endif
}

#endif