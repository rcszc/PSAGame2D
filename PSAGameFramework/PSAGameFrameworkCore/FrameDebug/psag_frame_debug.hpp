// psag_frame_debug. RCSZ 2024_08_15. 
// PSAG_FRAME_TIME_DEBUG - x64.

#ifndef __PSAG_FRAME_DEBUG_HPP
#define __PSAG_FRAME_DEBUG_HPP
#include <iostream>
#include <chrono>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

#include <new>
#include <type_traits>
#include <utility>

#if defined(_DEBUG) || defined(DEBUG)
#define ENABLE_DEBUG_MODE 1(bool)
#else
#define ENABLE_DEBUG_MODE 0(bool)
#endif

namespace PSAG_FTD_GLOBAL {
	extern std::atomic<size_t> MEMORY_OPER_ALLOC_COUNT;
	extern std::atomic<size_t> MEMORY_OPER_FREE_COUNT;

	using ClockTimePoint = std::chrono::steady_clock::time_point;
	
	// return system thread id. 
	// platform: windows,linux. [20240815]
	size_t GetCurrentThreadID();
	// return system process memory. 
	// platform: windows,linux. [20240815]
	size_t GetProcessMemoryUsage();

	class GLOBAL_SAMPLER_COUNT {
	protected:
		static std::mutex     SamplerTimerMutex;
		static ClockTimePoint SamplerTimer;
		static size_t         SamplerFrameCount;

		double SamplerFrameRunTimer();
		size_t SamplerFrameUpdate();

		static std::mutex PointsCountMutex;
		static size_t     PointsCountBytes;
		static size_t     PointsCountBytesFrame;

		void PointsCountADD();
		void PointsCountFrameUpdate(size_t* begin, size_t* size);
	};

	// version 0.2.0, x64, mem: 40bytes, 8bytes. 20240815.
	// update: 20250130 RCSZ. [MDSP02]
	struct MemDataSamplingPoint {
		// oper: [-][+] mem_bytes.
		int64_t  SmpMemoryOperBytes = NULL;
		uint64_t SmpMemoryAddress   = NULL;
		// timer: milliseconds.
		double SmpFrameTime = 0.0;

		size_t SmpCurrentThread = NULL;
		size_t SmpProcessMemory = NULL;
	};

	// version 0.2.5, x64, mem: 56bytes, 8bytes. 20240815.
	// update: 20240816 RCSZ. [MDSF02]
	struct MemDataSamplingFrame {
		// index_tabe: addr_offset + size.
		size_t SmpPointDatasetOffset = NULL;
		size_t SmpPointDatasetSize   = NULL;

		size_t SmpFrameUnqiue = NULL;
		double SmpFrameTime   = 0.0;
		size_t SmpFrameProcessMemUsage = NULL;

		size_t SmpFrameMemAlloc = NULL;
		size_t SmpFrameMemFree  = NULL;
	};
	
	// non-mutex thread safe double buffer.
	using MemDataPoints = std::vector<MemDataSamplingPoint>;
	struct AtomicDoubleBuffers {
	protected:
		std::atomic<bool> BufferIndex = false;
		MemDataPoints     Buffers[2]  = {};
	public:
		MemDataPoints* GetBufferW() { return &Buffers[(size_t)BufferIndex];  }
		MemDataPoints* GetBufferR() { return &Buffers[(size_t)!BufferIndex]; }

		void SwapBuffers() { BufferIndex = !BufferIndex; };
	};
}

namespace PsagDebugDencode {
	// dataset encoder(serialize), perf.
	template <typename T>
	std::vector<uint8_t> SerializeToBinary(const T& data) {
		static_assert(std::is_trivially_copyable<T>::value, "ERROR TYPE!");
		// binary_temp size = struct_size.
		std::vector<uint8_t> BinaryDataset(sizeof(T));
		// struct =copy=> binary.
		std::memcpy(BinaryDataset.data(), &data, sizeof(T));
		return BinaryDataset;
	}
}

namespace PsagDebugThread {
	// warning: global object.
	class ProcessThread :public PSAG_FTD_GLOBAL::GLOBAL_SAMPLER_COUNT {
	protected:
		static std::thread*     GLOBAL_THREAD;
		static std::atomic_bool GLOBAL_EXITFLAG;
		static std::mutex       GLOBAL_MUTEX;       // points.
		static std::mutex       GLOBAL_MUTEX_FRAME; // frames.

		static std::queue<PSAG_FTD_GLOBAL::MemDataSamplingPoint> GLOBAL_POINTS_CACHE;
		static std::queue<PSAG_FTD_GLOBAL::MemDataSamplingFrame> GLOBAL_FRAMES_CACHE;

		static void DatasetProcessing(const std::string& file_path);

		void DatasetPointADD(const PSAG_FTD_GLOBAL::MemDataSamplingPoint& data);
		void DatasetFrameADD(const PSAG_FTD_GLOBAL::MemDataSamplingFrame& data);
	public:
		bool CreateProcessingThread(const std::string& folder, const std::string& name);
		bool DeleteProcessingThread();

		void MainUpdateFrame();
	};
	// warn: system global object. [20240816]
	extern ProcessThread FTDprocessThread;
}

namespace PsagDebugCapture {
	using mem_address = uint64_t;
	// debug module: memory. 20250131, RCSZ.
	class DebugCaptureMem :protected PsagDebugThread::ProcessThread {
	public: void CaptureMemoryOperInfo(mem_address ptr_addr, int64_t oper_size);
	};
	extern DebugCaptureMem CAPinfoMemory;
}

#if defined(_WIN64)
#include <windows.h>

#define __PSAG_MEM_FREE_SUCC     1
#define __PSAG_MEM_FREE_NULLPTR -1
#define __PSAG_MEM_FREE_STACK   -2

class __PSAG_MEMORY_OPERATOR_BASE 
	:public PsagDebugCapture::DebugCaptureMem 
{
public:
	template<typename T, typename... Args>
	static T* AllocMem(Args&&... args) {
		void* pointer = ::operator new(sizeof(T));
		if (pointer == nullptr) 
			throw std::bad_alloc();
		// thread-safe capture memory operations info. new(v:'+')
		PsagDebugCapture::CAPinfoMemory.
			CaptureMemoryOperInfo((PsagDebugCapture::mem_address)pointer, int64_t(sizeof(T)));
		PSAG_FTD_GLOBAL::MEMORY_OPER_ALLOC_COUNT += sizeof(T);

		T* object = static_cast<T*>(pointer);
		if constexpr (std::is_trivial<T>::value) return object;
		else new (object) T(std::forward<Args>(args)...);
		return object;
	}

	// warn: include windows api.
	template<typename T>
	static int DeleteMem(T* ptr) {
		if (ptr == nullptr) return __PSAG_MEM_FREE_NULLPTR;
		if (!HeapValidate(GetProcessHeap(), NULL, ptr)) return __PSAG_MEM_FREE_STACK;
		// get heap memory params. 
		size_t memory_size = _msize(ptr);
		void*  memory_ptr  = ptr;
		// delete memory ~obj. pod & object.
		if constexpr (!std::is_trivially_destructible<T>::value) { 
			ptr->~T(); ::operator delete(ptr); 
		} // value => delete memory.
		else ::operator delete(ptr);
		// thread-safe capture memory operations info. delete(v:'-')
		PsagDebugCapture::CAPinfoMemory.
			CaptureMemoryOperInfo((PsagDebugCapture::mem_address)memory_ptr, -(int64_t)memory_size);
		PSAG_FTD_GLOBAL::MEMORY_OPER_FREE_COUNT += memory_size;
		return __PSAG_MEM_FREE_SUCC;
	}
};
#if defined(_DEBUG) && defined(PSAG_MEMORY_DEBUG)
// alloc & delete object memory def.
#define PSAG_MOBJ_NEW(TYPE, ...) __PSAG_MEMORY_OPERATOR_BASE::AllocMem <TYPE>(__VA_ARGS__)
#define PSAG_MOBJ_DEL(TYPE, PTR) __PSAG_MEMORY_OPERATOR_BASE::DeleteMem<TYPE>(PTR)
#else // RELEASE MODE.
#define PSAG_MOBJ_NEW(TYPE, ...) new TYPE(__VA_ARGS__)
#define PSAG_MOBJ_DEL(TYPE, PTR) delete PTR
#endif
#endif
#endif