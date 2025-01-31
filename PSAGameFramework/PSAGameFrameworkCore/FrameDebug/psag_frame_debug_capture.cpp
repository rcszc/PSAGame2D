// psag_frame_debug_capture.
#include "psag_frame_debug.hpp"

using namespace std;

// nanoseconds(int64_t) => milliseconds(double).
inline double CLOCK_TIME_CVT_COUNT(const PSAG_FTD_GLOBAL::ClockTimePoint& clock_point) {
	// time convert count(nanoseconds).
	int64_t TimeCount = chrono::duration_cast<chrono::nanoseconds>(
		chrono::steady_clock::now() - clock_point
	).count();
	// time nanoseconds => milliseconds.
	return (double)TimeCount / 1000000.0;
}

namespace PSAG_FTD_GLOBAL {
	mutex          GLOBAL_SAMPLER_COUNT::SamplerTimerMutex = {};
	ClockTimePoint GLOBAL_SAMPLER_COUNT::SamplerTimer      = chrono::steady_clock::now();
	size_t         GLOBAL_SAMPLER_COUNT::SamplerFrameCount = NULL;

	double GLOBAL_SAMPLER_COUNT::SamplerFrameRunTimer() {
		lock_guard<mutex> Lock(SamplerTimerMutex);
		// run timer.time_count.
		return CLOCK_TIME_CVT_COUNT(SamplerTimer);
	}

	size_t GLOBAL_SAMPLER_COUNT::SamplerFrameUpdate() {
		lock_guard<mutex> Lock(SamplerTimerMutex);
		// global frame count.
		return ++SamplerFrameCount;
	}

	mutex  GLOBAL_SAMPLER_COUNT::PointsCountMutex      = {};
	size_t GLOBAL_SAMPLER_COUNT::PointsCountBytes      = NULL;
	size_t GLOBAL_SAMPLER_COUNT::PointsCountBytesFrame = NULL;

	void GLOBAL_SAMPLER_COUNT::PointsCountADD() {
		lock_guard<mutex> Lock(PointsCountMutex);
		PointsCountBytesFrame += sizeof(MemDataSamplingPoint);
	}

	// data_offset & data_size, update_offset & clear size_count.
	void GLOBAL_SAMPLER_COUNT::PointsCountFrameUpdate(size_t* begin, size_t* size) {
		lock_guard<mutex> Lock(PointsCountMutex);

		*begin = PointsCountBytes;
		*size  = PointsCountBytesFrame;

		PointsCountBytes += PointsCountBytesFrame;
		PointsCountBytesFrame = NULL;
	}
}

namespace PsagDebugCapture {

	void DebugCaptureMem::CaptureMemoryOperInfo(mem_address ptr_addr, int64_t oper_size) {
		PSAG_FTD_GLOBAL::MemDataSamplingPoint DataTemp = {};
		// copy & setting mem_sample params.
		DataTemp.SmpMemoryOperBytes = oper_size;
		DataTemp.SmpMemoryAddress   = ptr_addr;
		{
			lock_guard<mutex> Lock(SamplerTimerMutex);
			DataTemp.SmpFrameTime = CLOCK_TIME_CVT_COUNT(SamplerTimer);
		}
		DataTemp.SmpProcessMemory = PSAG_FTD_GLOBAL::GetProcessMemoryUsage();
		DataTemp.SmpCurrentThread = PSAG_FTD_GLOBAL::GetCurrentThreadID();
		// storage params.
		DatasetPointADD(DataTemp);
		PointsCountADD();
	}

	// define: system global object.
	DebugCaptureMem CAPinfoMemory = {};
}