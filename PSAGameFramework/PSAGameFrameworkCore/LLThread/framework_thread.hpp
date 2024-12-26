// framework_thread. THREAD_POOL:[??????]
// EasyWorksPool(??), https://github.com/rcszc/EasyWorksPool
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

        // ??????. std::exception::what, std::exception::name
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

#endif