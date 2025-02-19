// framework_logger.
#include <fstream>
#include <thread>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <functional>
#include <filesystem>

#include "framework_logger.hpp"

using namespace std;
namespace PRLC = PSAG_LOGGER::ReadLogCache;

vector<PRLC::LogCache> LogMessageCache = {};
size_t LogWarringLines = 0, LogErrorLines = 0;

mutex LogMutex = {};
queue<PRLC::LogCache> LogWriteQueue = {};

#include <iomanip>
// time [xxxx.xx.xx.xx:xx:xx:xx ms].
string __get_timestamp(const chrono::system_clock::time_point& time_point) {
	auto Time   = std::chrono::system_clock::to_time_t(time_point);
	auto TimeMs = std::chrono::duration_cast<chrono::milliseconds>(time_point.time_since_epoch()) % 1000;
	// format time stamp.
	stringstream SStream;
	struct tm TimeStruct = {};
	localtime_s(&TimeStruct, &Time);
	SStream << put_time(&TimeStruct, "%Y.%m.%d %H:%M:%S") << ".";
	SStream << setfill('0') << setw(3) << TimeMs.count() << " ms";
	// windows / linux time stamp.
	return SStream.str();
}

string FMT_TIME_STAMP(const chrono::system_clock::time_point& time_point) {
	return __get_timestamp(time_point);
}

// global const hashlable.
const unordered_map<int32_t, string> HashLogLevel = {
	{LogError,   "\033[31m"},
	{LogWarning, "\033[33m"},
	{LogInfo,    "\033[90m"},
	{LogTrace,   "\033[96m"},
	{LogPerfmac, "\033[35m"}
};

string FMT_NUMBER_FILLZERO(uint32_t number, int32_t digits) {
	// format: %xd.
	string ResultTemp = to_string(number);
	// fill_number zero.
	while (ResultTemp.length() < (size_t)digits) {
		ResultTemp = "0" + ResultTemp;
	}
	return ResultTemp;
}

namespace PSAG_LOGGER {
#include <io.h>
#include <fcntl.h>
	atomic<bool> LOG_PRINT_SWITCH = true;
	// print color log: ANSI color.begin << message << ANSI color.end
	function<void(const string&, const string& msg)> 
		LOG_PRINT_FUNC = [](const string&, const string& msg) { cout << msg << endl; };
	mutex LOG_PRINT_FUNC_MUTEX = {};

	void PushLogProcess(
		const string& filemessage, 
		const LOGLABEL& level, const string& module_name, 
		const string& logstring
	) {
		unique_lock<mutex> LogThreadLock(LogMutex);

		string FileMessage = {};
		if (level & LogWarning) { LogWarringLines++; FileMessage = "<" + filemessage + "> "; };
		if (level & LogError)   { LogErrorLines++;   FileMessage = "<" + filemessage + "> "; };

		const char* LogLeveTag = "[NONE]";
		switch (level) {
		case(LogError):   { LogLeveTag = "[ERROR]";   break; }
		case(LogWarning): { LogLeveTag = "[WARNING]"; break; }
		case(LogInfo):    { LogLeveTag = "[INFO]";    break; }
		case(LogTrace):   { LogLeveTag = "[TRACE]";   break; }
		case(LogPerfmac): { LogLeveTag = "[PERF]";    break; }
		}
		string FmtModuleName = "[" + module_name + "]: ";
		string FmtModuleLog  = "[" + __get_timestamp(chrono::system_clock::now()) + "]"
			+ ":" + LogLeveTag + ":" + FmtModuleName + FileMessage + logstring;

		// => read logger chache & logger process(print).
		LogMessageCache.push_back(PRLC::LogCache(FmtModuleLog, module_name, level));
		LogWriteQueue.push(PRLC::LogCache(FmtModuleLog, module_name, level));
	}

#define LOGGER_BUFFER_LEN 2048
	void PushLoggerLLFPN(
		const LOGLABEL& level, const char* module_label,
		const char* module_message, ...
	) {
#if PSAG_DEBUG_MODE
		char LogCharTemp[LOGGER_BUFFER_LEN] = {};

		va_list ParamArgs;
		va_start(ParamArgs, module_message);
		vsnprintf(LogCharTemp, LOGGER_BUFFER_LEN, module_message, ParamArgs);
		va_end(ParamArgs);
		// format log process. non filelines msg.
		PushLogProcess({}, level, (string)module_label, LogCharTemp);
#endif
	}
	void PushLoggerImpl(
		const string& file_lines_message,
		const LOGLABEL& level, const char* module_label, 
		const char* module_message, ...
	) {
#if PSAG_DEBUG_MODE
		char LogCharTemp[LOGGER_BUFFER_LEN] = {};

		va_list ParamArgs;
		va_start(ParamArgs, module_message);
		vsnprintf(LogCharTemp, LOGGER_BUFFER_LEN, module_message, ParamArgs);
		va_end(ParamArgs);
		// format log process.
		PushLogProcess(file_lines_message, 
			level, (string)module_label, LogCharTemp);
#endif
	}

	void PrintColorLog(const string& color, const string& msg) {
		cout << color << msg << " \033[0m" << endl;
	}
	void PrintDefaultLog(const string& color, const string& msg) {
		(void)color; cout << msg << endl;
	}
	// status: print log ?, print color log ? 
	void SET_PRINTLOG_STATE(bool status_flag) { LOG_PRINT_SWITCH = status_flag; }
	void SET_PRINTLOG_COLOR(bool colors_flag) { 
		lock_guard<mutex> Lock(LOG_PRINT_FUNC_MUTEX);
		LOG_PRINT_FUNC = colors_flag == true ? PrintColorLog : PrintDefaultLog;
	}

	Vector3T<size_t> LogLinesStatistics() {
		Vector3T<size_t> ReturnValue = {};
		{
			lock_guard<mutex> LogThreadLock(LogMutex);
			// log lines counting.
			ReturnValue.vector_x = LogMessageCache.size(); // total   lines.
			ReturnValue.vector_y = LogWarringLines;        // warning lines.
			ReturnValue.vector_z = LogErrorLines;          // error   lines.
		}
		return ReturnValue;
	}

	namespace ReadLogCache {
		vector<LogCache> ExtractLogSegment(const uint32_t& lines) {
			vector<LogCache> ReturnLogCache = {};
			{
				size_t RetLinesCount =
					((int64_t)LogMessageCache.size() - lines - 1) >= 0 ?
					((int64_t)LogMessageCache.size() - lines - 1) : 0;

				lock_guard<mutex> LogThreadLock(LogMutex);
				for (size_t i = RetLinesCount; i < LogMessageCache.size(); ++i)
					ReturnLogCache.push_back(LogMessageCache[i]);
			}
			return ReturnLogCache;
		}
	}

	size_t GetTimeCountNow() {
		return (size_t)chrono::duration_cast
			<chrono::microseconds>(
				chrono::steady_clock::now().time_since_epoch()
			).count();
	}
}

#define LOGFILE_EXTENSION ".log"
namespace PSAG_LOGGER_PROCESS {

	thread* LogProcessThread = {};
	atomic<bool> LogProcessFlag = false;

	void ProcessLogBackground(const char* folder) {
		// create name: folder + name(time) + extensions.
		string FileNameTemp = 
			folder + 
			to_string(chrono::duration_cast<chrono::nanoseconds>(
				chrono::steady_clock::now().time_since_epoch()).count()
			) + LOGFILE_EXTENSION;

		fstream WriteLogFile(FileNameTemp, ios::out | ios::app);
		PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "create log_file: %s", FileNameTemp.c_str());
		
		vector<PRLC::LogCache> LogPrivateCache = {};
		// check file open status => set flag.
		if (WriteLogFile.is_open()) LogProcessFlag = true;
		while (LogProcessFlag) {
			// sleep 200 ms next query queue.
			this_thread::sleep_for(chrono::milliseconds(200));
			{
				lock_guard<mutex> Lock(LogMutex);
				if (LogWriteQueue.empty()) continue;
				// clear private cache => write.
				LogPrivateCache.clear();
				// global cache => write(private) cache.
				while (!LogWriteQueue.empty()) {
					LogPrivateCache.push_back(LogWriteQueue.front());
					LogWriteQueue.pop();
				}
			}
			// read log cache => write file.
			for (const auto& WriteLog : LogPrivateCache) {
				WriteLogFile << WriteLog.LogString << endl;
				// print log messgae entry.
#if PSAG_DEBUG_MODE
				auto ColorsFind = HashLogLevel.find(WriteLog.LogLabel);
				if (ColorsFind == HashLogLevel.end() || !PSAG_LOGGER::LOG_PRINT_SWITCH)
					continue;
				lock_guard<mutex> Lock(PSAG_LOGGER::LOG_PRINT_FUNC_MUTEX);
				PSAG_LOGGER::LOG_PRINT_FUNC(ColorsFind->second, WriteLog.LogString);
#endif
			}
		}
		WriteLogFile.close();
	}

	bool StartLogProcessing(const char* folder) {
		// check logger system folder.
		if (!filesystem::exists(folder) || !filesystem::is_directory(folder)) {
			PSAG_LOGGER::PushLogger(LogWarning, PSAG_LOGGER_LABEL, "invalid folder: %s", folder);
			// create system folder.
			if (filesystem::create_directories(folder)) {
				PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "create log_folder failed create.");
				return false;
			}
			PSAG_LOGGER::PushLogger(LogTrace, PSAG_LOGGER_LABEL, "create folder success.");
		}
		// init logger_thread.
		try {
			LogProcessThread = new thread(ProcessLogBackground, folder);
			PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "start thread success.");
		}
		catch (const exception& err) {
			PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "start thread error: %s", err.what());
			return false;
		}
		return true;
	}

	bool FreeLogProcessing() {
		// 处理日志缓存残留. 2025_02_06 RCSZ.
		this_thread::sleep_for(chrono::milliseconds(500));
		try {
			while (LogProcessThread->joinable()) {
				// set stop_flag => join_thread.
				LogProcessFlag = false;
				LogProcessThread->join();
			}
			PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "free thread success.");
			delete LogProcessThread;
		}
		catch (const exception& err) {
			PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "free thread error: %s", err.what());
			return false;
		}
		// global_cache clear free memory.
		LogMessageCache.clear();
		LogMessageCache.shrink_to_fit();
		return true;
	}
}

std::atomic<size_t> PSAG_SYS_GENERATE_KEY::UniqueCodeCounter = {};
// time counter & resource mutex.
std::mutex PSAG_SYS_GENERATE_KEY::TimeCounterMutex = {};
size_t     PSAG_SYS_GENERATE_KEY::TimeCounter      = {};