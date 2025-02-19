// psag_frame_debug_capture.
#include <iostream>
#include <fstream>
#include <filesystem>

#include "psag_frame_debug.hpp"

using namespace std;
using namespace PsagDebugDencode;

namespace PSAG_FTD_GLOBAL {
    atomic<size_t> MEMORY_OPER_ALLOC_COUNT = 0;
    atomic<size_t> MEMORY_OPER_FREE_COUNT  = 0;
    // windows-specific implementation.
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

    size_t GetCurrentThreadID() {
        return (size_t)GetCurrentThreadId();
    }
    size_t GetProcessMemoryUsage() {
        HANDLE HDprocess = GetCurrentProcess();
        if (HDprocess == NULL) return NULL;

        PROCESS_MEMORY_COUNTERS PMC;
        size_t MemoryUsage = 0;
        if (GetProcessMemoryInfo(HDprocess, &PMC, sizeof(PMC)))
            MemoryUsage = PMC.WorkingSetSize;
        return MemoryUsage;
    }

    // linux-specific implementation.
#elif defined(__linux__)
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>

    size_t GetCurrentThreadID() {
        return (size_t)syscall(SYS_gettid);
    }
    size_t GetProcessMemoryUsage() {
        struct sysinfo Info;
        if (sysinfo(&Info) != 0) return NULL;

        long PageSize = sysconf(_SC_PAGESIZE);
        long PageCount = Info.freeram / PageSize;
        return PageCount * PageSize;
    }
#else
#error "Unsupported Platform!"
#endif
}

namespace PsagDebugThread {
    using namespace PSAG_FTD_GLOBAL;

    inline bool DirectoryExists(const string& PATH) {
        // path: current folder.
        if (PATH.empty()) return true;

        filesystem::path _PATH(PATH);
        return filesystem::exists(_PATH) && filesystem::is_directory(_PATH);
    }

    thread*     ProcessThread::GLOBAL_THREAD      = nullptr;
    atomic_bool ProcessThread::GLOBAL_EXITFLAG    = false;
    mutex       ProcessThread::GLOBAL_MUTEX       = {};
    mutex       ProcessThread::GLOBAL_MUTEX_FRAME = {};

    queue<MemDataSamplingPoint> ProcessThread::GLOBAL_POINTS_CACHE = {};
    queue<MemDataSamplingFrame> ProcessThread::GLOBAL_FRAMES_CACHE = {};

#define CONVERT_BIN reinterpret_cast<const char*>
    void ProcessThread::DatasetProcessing(const string& file_path) {
#if defined(_DEBUG) && defined(PSAG_MEMORY_DEBUG)
        // files_type: points => (.ftbp), frame => (.ftbf)
        fstream DatasetFilePoints(file_path + ".ftbp", ios::out | ios::trunc | ios::binary);
        fstream DatasetFileFrame (file_path + ".ftbf", ios::out | ios::trunc | ios::binary);

        if (!DatasetFilePoints.is_open() || !DatasetFileFrame.is_open()) {
            GLOBAL_EXITFLAG = true;
            cout << "debug: invalid file paths, exit." << endl;
        }
        // processing write_loop binary,text.
        while (!GLOBAL_EXITFLAG) {
            {
                lock_guard<mutex> LOCK(GLOBAL_MUTEX);
                while (!GLOBAL_POINTS_CACHE.empty()) {
                    // dataset queue front => file(bin).
                    auto BinaryData = SerializeToBinary<MemDataSamplingPoint>(
                        GLOBAL_POINTS_CACHE.front());
                    // write file(.ftbp), type: binary. pop.
                    DatasetFilePoints.write(CONVERT_BIN(BinaryData.data()), BinaryData.size());
                    GLOBAL_POINTS_CACHE.pop();
                }
            }
            {
                lock_guard<mutex> LOCK(GLOBAL_MUTEX_FRAME);
                while (!GLOBAL_FRAMES_CACHE.empty()) {
                    // dataset queue => file(bin).
                    auto BinaryData = SerializeToBinary<MemDataSamplingFrame>(
                        GLOBAL_FRAMES_CACHE.front());
                    // write file(.ftbf), type: binary.
                    DatasetFileFrame.write(CONVERT_BIN(BinaryData.data()), BinaryData.size());
                    GLOBAL_FRAMES_CACHE.pop();
                }
            }
        }
        // close files.
        DatasetFilePoints.close();
        DatasetFileFrame.close();
#endif
    }

    bool ProcessThread::CreateProcessingThread(const std::string& folder, const std::string& name) {
        if (!DirectoryExists(folder)) {
            cout << "debug: invalid folder: " << folder << endl;
            return false;
        }
        // file name == null, fmt: in_default name.
        string FileNameStr = name;
        if (name.empty()) 
            FileNameStr = "ftdebug";
        try {
            // craete processing thread(global).
            GLOBAL_THREAD = new thread(DatasetProcessing, folder + FileNameStr);
        }
        catch (const exception& ErrInfo) {
            cout << "debug: failed create thread: " << ErrInfo.what() << endl;
            return false;
        }
        return true;
    }

    bool ProcessThread::DeleteProcessingThread() {
#if defined(_DEBUG) && defined(PSAG_MEMORY_DEBUG)
        try {
            GLOBAL_EXITFLAG = true;
            // free thread. join => delete_ptr.
            GLOBAL_THREAD->join(); delete GLOBAL_THREAD;
        }
        catch (const exception& ErrInfo) {
            cout << "debug: failed delete thread: " << ErrInfo.what() << endl;
            return false;
        }
#endif
        return true;
    }

    void ProcessThread::MainUpdateFrame() {
#if defined(_DEBUG) && defined(PSAG_MEMORY_DEBUG)
        PSAG_FTD_GLOBAL::MemDataSamplingFrame DataTemp = {};
        DataTemp.SmpFrameProcessMemUsage = PSAG_FTD_GLOBAL::GetProcessMemoryUsage();

        DataTemp.SmpFrameTime   = SamplerFrameRunTimer();
        DataTemp.SmpFrameUnqiue = SamplerFrameUpdate();

        DataTemp.SmpFrameMemAlloc = PSAG_FTD_GLOBAL::MEMORY_OPER_ALLOC_COUNT;
        DataTemp.SmpFrameMemFree  = PSAG_FTD_GLOBAL::MEMORY_OPER_FREE_COUNT;

        // update frame => get points pos_info.
        PointsCountFrameUpdate(&DataTemp.SmpPointDatasetOffset, &DataTemp.SmpPointDatasetSize);
        DatasetFrameADD(DataTemp);
#endif
    }

    void ProcessThread::DatasetPointADD(const PSAG_FTD_GLOBAL::MemDataSamplingPoint& data) {
        lock_guard<mutex> Lock(GLOBAL_MUTEX);
        // point.sampler data => dadaset queue.
        GLOBAL_POINTS_CACHE.push(data);
    }

    void ProcessThread::DatasetFrameADD(const PSAG_FTD_GLOBAL::MemDataSamplingFrame& data) {
        lock_guard<mutex> Lock(GLOBAL_MUTEX_FRAME);
        // frame.sampler data => dadaset queue.
        GLOBAL_FRAMES_CACHE.push(data);
    }
    // define: system global object.
    ProcessThread FTDprocessThread = {};
}