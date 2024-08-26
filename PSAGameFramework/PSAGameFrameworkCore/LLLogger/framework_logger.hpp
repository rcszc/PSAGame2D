// framework_logger. 2023_10_11. (ÒÆÖ²ÐÞ¸Ä)RCSZ

#ifndef _FRAMEWORK_LOG_HPP
#define _FRAMEWORK_LOG_HPP
#include <iostream>
#include <sstream>
#include <chrono>
#include <cstdarg>

#include "../LLDefine/framework_def_header.h"
#include "../LLDefine/framework_def_math.hpp"

StaticStrLABEL PSAG_LOGGER_LABEL = "PSAG_LOGGER";

// format number => string, %d(fill_zero).
std::string FMT_NUMBER_FILLZERO(uint32_t number, int32_t digits);

namespace PSAG_LOGGER {
	// @param "label" (level label), "module_name" (module), "logstr_text" (log information)
	void PushLogProcess(const LOGLABEL& label, const std::string&  module_name, const std::string& logstr_text);
	// @param label, module_label, text, params. [20231205]
	void PushLogger(const LOGLABEL& label, const char* module_label, const char* log_text, ...);

	// false: not printing on the console.
	void SET_PRINTLOG_STATE(bool status_flag);
	
	// @return Vector3T<size_t> (x : lines, y : warring, z : error)
	Vector3T<size_t> LogLinesStatistics();

	namespace ReadLogCache {
		struct LogCache {
			// lv.0: log_label, lv.1: log_model_name, lv.2: log_string.
			std::string LogString;
			std::string LogModuleName;
			LOGLABEL    LogLabel;
			LogCache(const std::string& str, const std::string& name, LOGLABEL lab) :
				LogString(str), LogModuleName(name), LogLabel(lab)
			{}
		};
		// @param  uint32_t, back - lines.
		// @return string
		std::vector<LogCache> ExtractLogSegment(const uint32_t& lines);
	}
	// get src time[nanoseconds].
	size_t GetTimeCountNow();
}

namespace PSAG_LOGGER_PROCESS {
	// async thread process. write folder & print.
	bool StartLogProcessing(const char* folder);
	bool FreeLogProcessing();
}

// framework global generate_key.
class PSAG_SYS_GENERATE_KEY {
private:
	static std::atomic<size_t> UniqueCodeCountTemp;

	static size_t     TimeCountBuffer;
	static std::mutex TimeCountBufferMutex;
public:
	// generate unique number(time:nanoseconds).
	// time_key => count_key. RCSZ [20240825]
	size_t PsagGenUniqueKey() {
		// count: const_add_value: 2.
		return UniqueCodeCountTemp += 2;
	}

	// generate unique number(time:nanoseconds). [OLD]
	size_t PsagGenTimeKey() {
		std::lock_guard<std::mutex> Lock(TimeCountBufferMutex);

		size_t GenNumberTemp = (size_t)std::chrono::duration_cast
			<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

		if (GenNumberTemp == TimeCountBuffer) {
			// prevent duplication.
			std::chrono::nanoseconds ThreadSleep(2);
			std::this_thread::sleep_for(ThreadSleep);
		}
		TimeCountBuffer = GenNumberTemp;
		return GenNumberTemp;
	}
};

#endif