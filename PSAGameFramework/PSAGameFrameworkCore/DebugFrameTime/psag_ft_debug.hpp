// psag_ft_debug. RCSZ 2024_08_15. 
// PSAG_FRAME_TIME_DEBUG - x64.

#ifndef __PSAG_FT_DEBUG_H
#define __PSAG_FT_DEBUG_H
#include <iostream>
#include <chrono>
#include <queue>
#include <cstring>

#include <thread>
#include <mutex>
#include <atomic>

#if (defined(_DEBUG) || defined(DEBUG)) && ENABLE_FTD
#define ENABLE_DEBUG_MODE 1
#else
#define ENABLE_DEBUG_MODE 0
#endif

// ft_debug: 采样点等级, MIN Lv.1 => Lv.5 MAJ.  
enum FTD_LEVEL_TAG {
	FTD_TagLv1 = 1 << 1,
	FTD_TagLv2 = 1 << 2,
	FTD_TagLv3 = 1 << 3,
	FTD_TagLv4 = 1 << 4,
	FTD_TagLv5 = 1 << 5
};

namespace FTD_SYS_GLOBAL {
	using clock_time_point = std::chrono::steady_clock::time_point;
	using pointer_count    = unsigned long long;
	using id_count_thread  = unsigned long long;
	using id_count_tagging = unsigned long;

	// return id_count. platform: windows,linux. [20240815]
	id_count_thread GetCurrentThreadID();
	// return bytes. platform: windows,linux. [20240815]
	size_t GetProcessMemoryUsage();

	class GLOBAL_SAMPLER_COUNT {
	protected:
		static std::mutex       SamplerTimerMutex;
		static clock_time_point SamplerTimer;
		static size_t           SamplerFrameCount;

		double SamplerFrameRunTimer();
		size_t SamplerFrameUpdate();

		static std::atomic<size_t> PointsCountBytes;
		static std::atomic<size_t> PointsCountBytesFrame;

		void PointsCountADD();
		void PointsCountFrameUpdate(size_t* begin, size_t* size);
	};

	// version 0.1, x64, mem: 48bytes, 8bytes. 20240815, update: 20240816.
	struct SrcDataSamplingPoint {
		// timer: milliseconds.
		double SmpPointTimer = 0.0;

		FTD_LEVEL_TAG    SmpPointTagFilter  = FTD_TagLv1;
		pointer_count    SmpPointTagPointer = NULL;
		id_count_tagging SmpPointTagText    = NULL;
		id_count_thread  SmpPointTagThread  = NULL;

		// oper: [-][+] mem_bytes.
		int64_t SmpPointMemOperBytes = NULL;
	};

	// version 0.1, x64, mem: 40bytes, 8bytes. 20240815, update: 20240816.
	struct SrcDataSamplingFrame {
		// index_tabe: addr_offset + size.
		size_t SmpPointDatasetOffset = NULL;
		size_t SmpPointDatasetSize   = NULL;

		size_t SmpFrameUnqiue = NULL;
		double SmpFrameClock  = 0.0;
		size_t SmpFrameProcessMemoryUsage = NULL;
	};
}

namespace FTDencode {
	// dataset encoder, performance.
	class DatasetEncode {
	protected:
		template <typename T>
		static std::vector<uint8_t> SerializeToBinary(const T& data) {
			static_assert(std::is_trivially_copyable<T>::value, "ERROR TYPE!");
			// binary_temp size = struct_size.
			std::vector<uint8_t> BinaryDataset(sizeof(T));
			// struct =copy=> binary.
			std::memcpy(BinaryDataset.data(), &data, sizeof(T));
			return BinaryDataset;
		}
	};
}

namespace FTDthread {
	// warning: global object.
	class ProcessThread :public FTD_SYS_GLOBAL::GLOBAL_SAMPLER_COUNT,
		public FTDencode::DatasetEncode
	{
	protected:
		static std::thread*     GLOBAL_ProcessingThread;
		static std::atomic_bool GLOBAL_ProcessingExit;

		static std::mutex GLOBAL_PointsResMutex; // points,texts.
		static std::mutex GLOBAL_FramesResMutex; // frame.

		static std::queue<std::string> GLOBAL_TEXTS_CACHE;
		static std::queue<FTD_SYS_GLOBAL::SrcDataSamplingPoint> GLOBAL_POINTS_CACHE;
		static std::queue<FTD_SYS_GLOBAL::SrcDataSamplingFrame> GLOBAL_FRAMES_CACHE;

		static void DatasetProcessing(const std::string& file_path);

		void DatasetPointADD(const FTD_SYS_GLOBAL::SrcDataSamplingPoint& data, const std::string& text);
		void DatasetFrameADD(const FTD_SYS_GLOBAL::SrcDataSamplingFrame& data);
	public:
		bool CreateProcessingThread(const std::string& folder, const std::string& name);
		bool DeleteProcessingThread();

		void MainUpdateFrame();
	};

	// OBJ: system global_object. [20240816]
	extern ProcessThread FTDprocessingThread;
}

namespace FTDcapture {
	// con => capture => des => upload.
	class CaptureContext :protected FTDthread::ProcessThread {
	protected:
		// atomic value > 0, empty => 0.
		static std::atomic<FTD_SYS_GLOBAL::id_count_tagging> GLOBAL_COUNT;

		FTD_SYS_GLOBAL::clock_time_point     ContextTimer   = {};
		FTD_SYS_GLOBAL::SrcDataSamplingPoint ContextDataset = {};

		std::string ContextString = {};
		size_t ContextOperMemory = NULL;
	public:
		~CaptureContext();

		void CaptureSettingFilter (const FTD_LEVEL_TAG& level);
		void CaptureSettingPointer(const void* pointer);
		void CaptureSettingTagging(const std::string& text);

		void CaptureBegin();
	};
}

#endif