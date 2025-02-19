// PSAGameFramework. 2024_03_31.
// @PomeloStar Studio 2024-2026 GameFramework(Engine).
// Developers: 
/*
* Engine Version: 0.1.2 Alpha (10.20241226)
* MSVC C11, C++17 x64 Release
* OpenGL GLSL: 4.6
* OpenGL GLFW: 3.0
* ImGui: ImGui For OpenGL3
* ImageLoader: STB_IMAGE, STB_IMAGE_WRITE
* JsonLoader: RapidJSON
* SoundLoader: OpenAL, STB_VORBIS
* FileCompress: Zlib
* PhysicsSystem: Box2D
*/
// Engine Update: 2025_01_27.
// Copyright(c) RCSZ.
#define PSAG2D_ENGINE_VER 12.20250127
/*
* 附加说明:
* [2024.03.31] 部分底层模块来自: ImProFX: https://github.com/rcszc/ImProFX
*/
#include "PSAGameFrameworkCore/CoreFramework/psag_core_drivers.h"
// 无丝竹之乱耳~
#pragma warning(disable: 4819)

#pragma comment(lib, "opengl32.lib ")
#pragma comment(lib, "glfw3.lib    ")
#pragma comment(lib, "glew32.lib   ")
#pragma comment(lib, "glu32.lib    ")
#pragma comment(lib, "OpenAL32.lib ")
#pragma comment(lib, "zlibwapi.lib ")
#pragma comment(lib, "box2d.lib    ")

// windows x64 crt debug.
#if defined(PSAG_DEBUG_EXT_MODE)
#define _CRTDBG_MAP_ALLOC  
#include <cstdlib>
#include <crtdbg.h>
#endif
int main() {
#ifdef PSAG_DEBUG_EXT_MODE
	_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
#endif
	int FrameworkResult = -1;
	// set log print & start(debug) logger thread.
	PSAG_LOGGER::SET_PRINTLOG_COLOR(STAT_ON);
	PSAG_LOGGER_PROCESS::StartLogProcessing("PSAGameSystemLogs/");
#ifdef PSAG_DEBUG_EXT_MODE
	PsagDebugThread::FTDprocessThread.CreateProcessingThread("PSAGameSystemDebug/", "MemoryMsg");
#endif
	{
		// create framework object.
		PsagFrameworkCore::PSAGame2DFramework* MainPSAGame2D = new PsagFrameworkCore::PSAGame2DFramework();
		PsagFrameworkStart::CorePsagMainStart FrameworkStarter = {};
		// add start_item: object.
		FrameworkStarter.UniqueFrameworkObj(MainPSAGame2D);
		// start framework(engine)[exe], 200MS.
		FrameworkStarter.StartPsaGame(PSAG2D_ENGINE_VER, 200);
		// framework free exit.
		FrameworkResult = FrameworkStarter.FreeFramework();
	}
#ifdef PSAG_DEBUG_EXT_MODE
	PsagDebugThread::FTDprocessThread.DeleteProcessingThread();
#endif
	PSAG_LOGGER_PROCESS::FreeLogProcessing();
#ifdef PSAG_DEBUG_EXT_MODE
	_CrtDumpMemoryLeaks();
#endif
	return FrameworkResult;
}