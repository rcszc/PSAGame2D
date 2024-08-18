// psag_ft_debug_capture.
#include "psag_ft_debug.hpp"

using namespace std;

// nanoseconds(int64_t) => milliseconds(double).
inline double CLOCK_TIME_CVT_COUNT(const FTD_SYS_GLOBAL::clock_time_point& clock_point) {
	// time convert count(nanoseconds).
	int64_t TimeCount = chrono::duration_cast<chrono::nanoseconds>(
		chrono::steady_clock::now() - clock_point
	).count();
	// time nanoseconds => milliseconds.
	return (double)TimeCount / 1000000;
}

namespace FTD_SYS_GLOBAL {
	mutex            GLOBAL_SAMPLER_COUNT::SamplerTimerMutex = {};
	clock_time_point GLOBAL_SAMPLER_COUNT::SamplerTimer      = chrono::steady_clock::now();
	size_t           GLOBAL_SAMPLER_COUNT::SamplerFrameCount = NULL;

	double GLOBAL_SAMPLER_COUNT::SamplerFrameRunTimer() {
		unique_lock<mutex> LOCK(SamplerTimerMutex);
		// run timer.time_count.
		return CLOCK_TIME_CVT_COUNT(SamplerTimer);
	}

	size_t GLOBAL_SAMPLER_COUNT::SamplerFrameUpdate() {
		unique_lock<mutex> LOCK(SamplerTimerMutex);
		// global frame count.
		return ++SamplerFrameCount;
	}

	atomic<size_t> GLOBAL_SAMPLER_COUNT::PointsCountBytes      = NULL;
	atomic<size_t> GLOBAL_SAMPLER_COUNT::PointsCountBytesFrame = NULL;

	void GLOBAL_SAMPLER_COUNT::PointsCountADD() {
		PointsCountBytesFrame += sizeof(SrcDataSamplingPoint);
	}

	// data_offset & data_size, update_offset & clear size_count.
	void GLOBAL_SAMPLER_COUNT::PointsCountFrameUpdate(size_t* begin, size_t* size) {
		// points mapping => frames. 
		*begin = PointsCountBytes;
		*size  = PointsCountBytesFrame;

		PointsCountBytes += PointsCountBytesFrame;
		PointsCountBytesFrame = NULL;
	}
}

namespace FTDcapture {
	atomic<FTD_SYS_GLOBAL::id_count_tagging> CaptureContext::GLOBAL_COUNT = NULL;

	CaptureContext::~CaptureContext() {
		// 上下文内存操作大小. [-][+]
		ContextOperMemory = (int64_t)FTD_SYS_GLOBAL::GetProcessMemoryUsage() - ContextOperMemory;

		ContextDataset.SmpPointTagText   = GLOBAL_COUNT;
		ContextDataset.SmpPointTagThread = FTD_SYS_GLOBAL::GetCurrentThreadID();

		ContextDataset.SmpPointTimer        = CLOCK_TIME_CVT_COUNT(ContextTimer);
		ContextDataset.SmpPointMemOperBytes = ContextOperMemory;

		// point sampler data => dataset, count_add.
		DatasetPointADD(ContextDataset, ContextString);
		PointsCountADD();
	}

	void CaptureContext::CaptureSettingFilter(const FTD_LEVEL_TAG& level) {
		ContextDataset.SmpPointTagFilter = level;
	}

	void CaptureContext::CaptureSettingPointer(const void* pointer) {
		ContextDataset.SmpPointTagPointer = (FTD_SYS_GLOBAL::pointer_count)pointer;
	}

	void CaptureContext::CaptureSettingTagging(const string& text) {
		ContextString = text;
		if (!ContextString.empty()) ++GLOBAL_COUNT;
	}

	void CaptureContext::CaptureBegin() {
		// context begin: memory,timer.
		ContextOperMemory = (int64_t)FTD_SYS_GLOBAL::GetProcessMemoryUsage();
		ContextTimer = chrono::steady_clock::now();
	}
}