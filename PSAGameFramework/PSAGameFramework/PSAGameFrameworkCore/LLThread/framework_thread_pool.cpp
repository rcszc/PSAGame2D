// framework_thread_pool.
#include "framework_thread.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_THREAD_POOL {

    size_t ThisThreadID() {
        size_t ReturnThreadID = NULL;

        thread::id ThreadID = this_thread::get_id();
        hash<thread::id> Hasher;
        ReturnThreadID = Hasher(ThreadID);

        return ReturnThreadID;
    }

    namespace Error {
        TPerror::TPerror(ERRINFO message, size_t pid, ERRINFO compname) {
            ErrorMessage = message + to_string(pid);
            ErrorCompName = compname;
        }
    }

    void PsagThreadTasks::ThreadsTaskExecution(uint32_t workers_num) {
        // start threads(workers).
        for (size_t i = 0; i < workers_num; ++i) {
            try {
                ThreadWorkers.emplace_back([this] {
                    // loop execution task.
                    while (true) {
                        function<void()> WorkTaskExecution;
                        unique_lock<mutex> Lock(PoolMutex);

                        WorkersCondition.wait(Lock, [this] { return PauseFlag || !PoolTasks.empty(); });
                        // loop exit.
                        if (PauseFlag && PoolTasks.empty())
                            break;
                        WorkTaskExecution = move(PoolTasks.front());
                        // queue => delete task.
                        PoolTasks.pop();

                        ++WorkingThreadsCount;
                        WorkTaskExecution();
                        --WorkingThreadsCount;
                    }
                });
            }
            catch (...) {
                throw Error::TPerror("failed create thread.", ThisThreadID(), "EXEC_TASK");
            }
        }
    }

    void PsagThreadTasks::ThreadsTaskFree() {
        {
            unique_lock<mutex> Lock(PoolMutex);
            PauseFlag = true;
        }
        try {
            WorkersCondition.notify_all();
            for (thread& Worker : ThreadWorkers) {
                // free all workers(threads).
                Worker.join();
            }
        }
        catch (...) {
            throw Error::TPerror("failed delete thread.", ThisThreadID(), "FREE_POOL");
        }
    }

    uint32_t PsagThreadTasks::GetWorkingThreadsCount() {
        return WorkingThreadsCount;
    }

    uint32_t PsagThreadTasks::GetTaskQueueCount() {
        uint32_t TasksCount = NULL;
        {
            unique_lock<mutex> Lock(PoolMutex);
            TasksCount = (uint32_t)PoolTasks.size();
        }
        return TasksCount;
    }

    void PsagThreadTasks::ResizeWorkers(uint32_t resize) {
        {
            unique_lock<mutex> Lock(PoolMutex);
            WorkersCondition.notify_all();
        }
        ThreadsTaskFree();

        ThreadWorkers.resize(resize);
        PauseFlag = false;
        ThreadsTaskExecution(resize);
    }
}