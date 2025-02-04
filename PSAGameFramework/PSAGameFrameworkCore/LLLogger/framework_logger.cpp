// framework_logger.
#include <fstream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <filesystem>

#include "framework_logger.hpp"

using namespace std;
namespace PRLC = PSAG_LOGGER::ReadLogCache;

vector<PRLC::LogCache> LogMessageCache = {};
size_t LogWarringLines = NULL, LogErrorLines = NULL;

mutex LogMutex = {};
queue<PRLC::LogCache> LogWriteQueue = {};

#include <iomanip>
// time [xxxx.xx.xx.xx:xx:xx:xx ms].
string __get_timestamp(const chrono::system_clock::time_point& time_point) {
	auto Time   = std::chrono::system_clock::to_time_t(time_point);
	auto TimeMs = std::chrono::duration_cast<chrono::milliseconds>(time_point.time_since_epoch()) % 1000;
	// format time stamp.
	stringstream SStream;
	SStream << put_time(localtime(&Time), "%Y.%m.%d %H:%M:%S") << ".";
	SStream << setfill('0') << setw(3) << TimeMs.count() << " ms";
	// windows / linux time stamp.
	return SStream.str();
}

string FMT_TIME_STAMP(const chrono::system_clock::time_point& time_point) {
	return __get_timestamp(time_point);
}

// global const hashlable.
const std::unordered_map<int32_t, std::string> HashLogLevel = {
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
	while (ResultTemp.length() < digits) {
		ResultTemp = "0" + ResultTemp;
	}
	return ResultTemp;
}

namespace PSAG_LOGGER {
#include <io.h>
#include <fcntl.h>
	atomic<bool> LOG_PRINT_SWITCH = true;
	// print color log: ANSI color.begin << message << ANSI color.end
	function<void(const std::string&, const std::string& msg)> LOG_PRINT_FUNC = 
		[](const std::string&, const std::string& msg) { cout << msg << endl; };

	void PushLogProcess(const LOGLABEL& label, const std::string& module_name, const std::string& logstr_text) {
		unique_lock<mutex> LogThreadLock(LogMutex);

		if (label & LogWarning) { LogWarringLines++; };
		if (label & LogError)   { LogErrorLines++; };

		const char* LogLEVEL = "[NULL]";
		switch (label) {
		case(LogError):   { LogLEVEL = "[ERROR]";   break; }
		case(LogWarning): { LogLEVEL = "[WARNING]"; break; }
		case(LogInfo):    { LogLEVEL = "[INFO]";    break; }
		case(LogTrace):   { LogLEVEL = "[TRACE]";   break; }
		case(LogPerfmac): { LogLEVEL = "[PERF]";    break; }
		}
		string FmtModuleName = "[" + module_name + "]: ";
		string FmtModuleLog  = "[" + __get_timestamp(chrono::system_clock::now()) + "]"
			+ ":" + LogLEVEL + ":" + FmtModuleName + logstr_text;

		// => read logger chache & logger process(print).
		LogMessageCache.push_back(PRLC::LogCache(FmtModuleLog, module_name, label));
		LogWriteQueue.push(PRLC::LogCache(FmtModuleLog, module_name, label));
	}

#define LOGGER_BUFFER_LEN 2048
	void PushLogger(const LOGLABEL& label, const char* module_label, const char* log_text, ...) {
#if PSAG_DEBUG_MODE
		char LoggerStrTemp[LOGGER_BUFFER_LEN] = {};

		va_list ParamArgs;
		va_start(ParamArgs, log_text);
		vsnprintf(LoggerStrTemp, LOGGER_BUFFER_LEN, log_text, ParamArgs);
		va_end(ParamArgs);

		PushLogProcess(label, (string)module_label, LoggerStrTemp);
#endif
	}

	void PrintColorLog(const string& color, const string& msg) {
		cout << color << msg << " \033[0m" << endl;
	}
	void PrintDefaultLog(const string& color, const string& msg) {
		cout << msg << endl;
	}
	// status: print log ?, print color log ? 
	void SET_PRINTLOG_STATE(bool status_flag) { LOG_PRINT_SWITCH = status_flag; }
	void SET_PRINTLOG_COLOR(bool colors_flag) { 
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
	bool    LogProcessFlag   = true;

	void process_printwrite_file_eventloop(const char* folder) {
		// create name: folder + name(time) + extensions.
		string FileNameTemp = 
			folder + 
			to_string(chrono::duration_cast<chrono::nanoseconds>(
				chrono::steady_clock::now().time_since_epoch()).count()
			) + LOGFILE_EXTENSION;

		fstream WriteLogFile(FileNameTemp, ios::out | ios::app);
		PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "create log_file: %s", FileNameTemp.c_str());
		
		while (LogProcessFlag) {
			unique_lock<mutex> LogThreadLock(LogMutex);
			// read log cache queue.
			while (!LogWriteQueue.empty()) {
				const PRLC::LogCache& LogMsgTemp = LogWriteQueue.front();
				
				WriteLogFile << LogMsgTemp.LogString << endl;
				// print color_log entry.
#if PSAG_DEBUG_MODE
				auto ColorsFind = HashLogLevel.find(LogMsgTemp.LogLabel);
				if (ColorsFind != HashLogLevel.end() && PSAG_LOGGER::LOG_PRINT_SWITCH)
					PSAG_LOGGER::LOG_PRINT_FUNC(ColorsFind->second, LogMsgTemp.LogString);
#endif
				// delete log_message item.
				LogWriteQueue.pop();
			}
			LogThreadLock.unlock();
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
			LogProcessThread = new thread(process_printwrite_file_eventloop, folder);
			PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "start thread success.");
		}
		catch (const exception& err) {
			PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "start thread error: %s", err.what());
			return false;
		}
		return true;
	}

	bool FreeLogProcessing() {
		LogProcessFlag = false;
		try {
			// set_flag  => join_thread.
			LogProcessThread->join();
			PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "free thread success.");
		}
		catch (const exception& err) {
			PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "free thread error: %s", err.what());
			return false;
		}
		return true;
	}
}

std::atomic<size_t> PSAG_SYS_GENERATE_KEY::UniqueCodeCounter = {};
// time counter & resource mutex.
std::mutex PSAG_SYS_GENERATE_KEY::TimeCounterMutex = {};
size_t     PSAG_SYS_GENERATE_KEY::TimeCounter      = {};