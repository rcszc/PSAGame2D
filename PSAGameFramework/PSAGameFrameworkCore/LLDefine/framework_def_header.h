// framework_def_header. RCSZ.

#ifndef _FRAMEWORK_DEFINE_HPP
#define _FRAMEWORK_DEFINE_HPP
#include <string>
#include <vector>
#include <any>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <random>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

#include <fstream>
#include <filesystem>

#include <functional>
#include <typeinfo> // RTTI.

//#define PSAG_MEMORY_DEBUG
#include "../FrameDebug/psag_frame_debug.hpp"
#define POMELO_STAR_GAME2D_DEF true

#if defined(_DEBUG) || defined(DEBUG)
#define PSAG_COMPILE_MODE 1 // mode: debug(code: 1)
#else
#define PSAG_COMPILE_MODE 0 // mode: release(code: 0)
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
#define PSAG_COMPILE_X64 1 // 64bit.
#else
#define PSAG_COMPILE_X64 0
#endif

// only one can be opened.
#define PSAG_COLSYSTEM_RGBA12
// #define PSAG_COLSYSTEM_RGBA16

// warring old_function.
#if defined(_MSC_VER) // MSVC.
#define OLD_FUNCTION __declspec(deprecated)
#else                 // GCC.
#define OLD_FUNCTION __attribute__((deprecated))
#endif

#define STAGE_EXIT_INIT -2
#define STAGE_EXIT_FREE -3
#define STAGE_EXIT_LOOP -4

#define MAIN_RENDER_LOOP_COUNT_STEP 0.025f

#define IMTOOL_CVT_IMPTR (void*)(intptr_t)
#define LAMBDA_QUOTE     [&]()
#define LAMBDA_PARAM     [=]()

#define FS_VERTEX_ELEMENT 3
#define FS_VERTEX_NORMAL  3
#define FS_VERTEX_COLOR   4
#define FS_VERTEX_UVCOORD 2

#define FS_VERTEX_LENGTH  FS_VERTEX_ELEMENT + FS_VERTEX_NORMAL + FS_VERTEX_COLOR + FS_VERTEX_UVCOORD
#define FS_VERTEX_BYTES  (FS_VERTEX_ELEMENT + FS_VERTEX_NORMAL + FS_VERTEX_COLOR + FS_VERTEX_UVCOORD) * sizeof(float)

#define FS_SIZE_VERTEX   (size_t)52
#define FS_SIZE_TRIANGLE (size_t)156

#define PSAG_TRUE  (bool)1
#define PSAG_FALSE (bool)0

#if defined(_DEBUG) || defined(DEBUG)
#define PSAG_DEBUG_MODE 1
#else
#define PSAG_DEBUG_MODE 0
#endif

#define StaticStrLABEL constexpr const char*
// core framework logger label.
enum LOGLABEL {
	LogError   = 1 << 1, // 标签 <错误>
	LogWarning = 1 << 2, // 标签 <警告>
	LogInfo    = 1 << 3, // 标签 <信息>
	LogTrace   = 1 << 4, // 标签 <跟踪>
	LogPerfmac = 1 << 5  // 标签 <性能> (performance)
};

// fp64 (double): xxx.xxxx_xx_xx
// fp32 (float):  xxx.xxxx_xx
using FrameworkSysVersion = double;

// raw bytes_stream data ptr.
struct RawDataStreamIdx {
	uint8_t* DataPointer;
	size_t   DataBytes;
	RawDataStreamIdx() : DataPointer(nullptr), DataBytes(NULL) {}
	RawDataStreamIdx(uint8_t* ptr, size_t size) : DataPointer(ptr), DataBytes(size) {}
};

// raw bytes_stream data.
class RawDataStream :public std::vector<uint8_t> {
public:
	RawDataStream() = default;
	RawDataStream(uint8_t* v_data, size_t v_bytes) : std::vector<uint8_t>(v_data, v_data + v_bytes) {}
};
// raw audio(bytes_stream) data.
class RawAudioStream :public std::vector<uint8_t> {
public:
	RawAudioStream() = default;
	RawAudioStream(const RawDataStream& data) : std::vector<uint8_t>(data.begin(), data.end()) {}
};
using ResUnique = unsigned long long;
// virtual texture unique_key. (not llres key)
using VirTextureUnqiue = unsigned long long;

#endif