// psag_ft_debug_thread.
#include <iostream>
#include <fstream>
#include <filesystem>
#include <condition_variable>

#include "psag_ft_debug.hpp"

using namespace std;

namespace FTD_SYS_GLOBAL {
    // windows-specific implementation.
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

    id_count_thread GetCurrentThreadID() {
        return (id_count_thread)GetCurrentThreadId();
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

    thread_id_count GetCurrentThreadID() {
        return (thread_id_count)syscall(SYS_gettid);
    }

    size_t GetProcessMemoryUsage() {
        struct sysinfo Info;
        if (sysinfo(&Info) != 0) return NULL;

        long PageSize  = sysconf(_SC_PAGESIZE);
        long PageCount = Info.freeram / PageSize;
        return PageCount * PageSize;
    }
#else
#error "Unsupported Platform!"
#endif
}

namespace FTDthread {
    inline bool DirectoryExists(const string& PATH) {
        // path: current folder.
        if (PATH.empty()) return true;

        filesystem::path _PATH(PATH);
        return filesystem::exists(_PATH) && filesystem::is_directory(_PATH);
    }

    thread*     ProcessThread::GLOBAL_ProcessingThread = nullptr;
    atomic_bool ProcessThread::GLOBAL_ProcessingExit   = false;

    mutex ProcessThread::GLOBAL_PointsResMutex = {};
    mutex ProcessThread::GLOBAL_FramesResMutex = {};

    queue<string>                               ProcessThread::GLOBAL_TEXTS_CACHE  = {};
    queue<FTD_SYS_GLOBAL::SrcDataSamplingPoint> ProcessThread::GLOBAL_POINTS_CACHE = {};
    queue<FTD_SYS_GLOBAL::SrcDataSamplingFrame> ProcessThread::GLOBAL_FRAMES_CACHE = {};

#define CONVERT_BIN reinterpret_cast<const char*>
    void ProcessThread::DatasetProcessing(const string& file_path) {
        // files_type: text => .ftbt, points => .ftbp, frames => .ftbf
        fstream DatasetFileText  (file_path + ".ftbt", ios::out | ios::trunc);
        fstream DatasetFilePoints(file_path + ".ftbp", ios::out | ios::trunc | ios::binary);
        fstream DatasetFileFrames(file_path + ".ftbf", ios::out | ios::trunc | ios::binary);

        if (!DatasetFileText.is_open() || !DatasetFilePoints.is_open() || !DatasetFileFrames.is_open()) {
            GLOBAL_ProcessingExit = true;
            cout << "FT_DEBUG: invalid files path, exit." << endl;
        }

        // processing write_loop binary,text.
        while (!GLOBAL_ProcessingExit) {
            GLOBAL_PointsResMutex.lock();
            {
                while (!GLOBAL_POINTS_CACHE.empty()) {
                    // dataset queue => file(bin).
                    auto BinaryData = SerializeToBinary<FTD_SYS_GLOBAL::SrcDataSamplingPoint>(GLOBAL_POINTS_CACHE.front());
                    // write file(.ftbp), type: binary.
                    DatasetFilePoints.write(CONVERT_BIN(BinaryData.data()), BinaryData.size());
                    GLOBAL_POINTS_CACHE.pop();
                }
                while (!GLOBAL_TEXTS_CACHE.empty()) {
                    // dataset queue => file(text).
                    auto StringData = GLOBAL_TEXTS_CACHE.front();
                    // write file(.ftbt), type: text.
                    DatasetFileText << StringData << endl;
                    GLOBAL_TEXTS_CACHE.pop();
                }
            }
            GLOBAL_PointsResMutex.unlock();

            GLOBAL_FramesResMutex.lock();
            {
                while (!GLOBAL_FRAMES_CACHE.empty()) {
                    // dataset queue => file(bin).
                    auto BinaryData = SerializeToBinary<FTD_SYS_GLOBAL::SrcDataSamplingFrame>(GLOBAL_FRAMES_CACHE.front());
                    // write file(.ftbf), type: binary.
                    DatasetFileFrames.write(CONVERT_BIN(BinaryData.data()), BinaryData.size());
                    GLOBAL_FRAMES_CACHE.pop();
                }
            }
            GLOBAL_FramesResMutex.unlock();
        }
        // close [.ftbt][.ftbp][.ftbf] dataset_files.
        DatasetFileText.close();
        DatasetFilePoints.close();
        DatasetFileFrames.close();
    }

    bool ProcessThread::CreateProcessingThread(const string& folder, const string& name) {
        if (!DirectoryExists(folder)) {
            cout << "FT_DEBUG: invalid folder: " << folder << endl;
            return false;
        }
        // file name == null, fmt: in_default name.
        string FileNameStr = name;
        if (name.empty()) 
            FileNameStr = "ftdebug";
        try {
            // craete processing thread(global).
            GLOBAL_ProcessingThread = new thread(DatasetProcessing, folder + FileNameStr);
        }
        catch (const exception& ErrInfo) {
            cout << "FT_DEBUG: failed create thread: " << ErrInfo.what() << endl;
            return false;
        }
        return true;
    }

    bool ProcessThread::DeleteProcessingThread() {
        try {
            GLOBAL_ProcessingExit = true;
            // free thread. join => delete_ptr.
            GLOBAL_ProcessingThread->join(); delete GLOBAL_ProcessingThread;
        }
        catch (const exception& ErrInfo) {
            cout << "FT_DEBUG: failed delete thread: " << ErrInfo.what() << endl;
            return false;
        }
        return true;
    }

    void ProcessThread::MainUpdateFrame() {
        FTD_SYS_GLOBAL::SrcDataSamplingFrame DataTemp = {};
        DataTemp.SmpFrameProcessMemoryUsage = FTD_SYS_GLOBAL::GetProcessMemoryUsage();

        DataTemp.SmpFrameClock  = SamplerFrameRunTimer();
        DataTemp.SmpFrameUnqiue = SamplerFrameUpdate();

        // update frame => get points pos_info.
        PointsCountFrameUpdate(&DataTemp.SmpPointDatasetOffset, &DataTemp.SmpPointDatasetSize);
        DatasetFrameADD(DataTemp);
    }

    void ProcessThread::DatasetPointADD(const FTD_SYS_GLOBAL::SrcDataSamplingPoint& data, const string& text) {
        unique_lock<mutex> LOCK(GLOBAL_PointsResMutex);
        // points.sampler data => dadaset queue.
        GLOBAL_POINTS_CACHE.push(data);

        if (text.empty()) return;
        GLOBAL_TEXTS_CACHE.push(text);
    }

    void ProcessThread::DatasetFrameADD(const FTD_SYS_GLOBAL::SrcDataSamplingFrame& data) {
        unique_lock<mutex> LOCK(GLOBAL_FramesResMutex);
        // frame.sampler data => dadaset queue.
        GLOBAL_FRAMES_CACHE.push(data);
    }

    // def: system global_object.
    ProcessThread FTDprocessingThread = {};
}