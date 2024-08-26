// PSAGameFramework. 2024_03_31.
// @PomeloStar Studio 2024-2025 GameFramework(Engine).
// Developers: 
/*
* Engine Version: 0.1.2 Alpha (10.20240826)
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
// Engine Update: 2024_08_26.
#define PSAG2D_ENGINE_VER 12.20240817
/*
* 附加说明:
* [2024.03.31] 部分底层模块来自: ImProFX: https://github.com/rcszc/ImProFX
*/

#include <cstdlib>
#include "PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"
// 无丝竹之乱耳~
#pragma warning(disable: 4819)

#pragma comment(lib, "opengl32.lib ")
#pragma comment(lib, "glfw3.lib    ")
#pragma comment(lib, "glew32.lib   ")
#pragma comment(lib, "glu32.lib    ")
#pragma comment(lib, "OpenAL32.lib ")
#pragma comment(lib, "zlibwapi.lib ")
#pragma comment(lib, "box2d.lib    ")

int main() {
	int FrameworkResult = -1;
	// start(debug) logger thread & debug thread.
	// FTD update: "PSAGameFrameworkCore\CoreFramework\psag_mainevent_support.cpp"
	// RCSZ. [20240817]
	PSAG_LOGGER_PROCESS::StartLogProcessing("PSAGameSystemLogs/");
#if ENABLE_DEBUG_MODE
	FTDthread::FTDprocessingThread.CreateProcessingThread("PSAGameSystemDebug/", "PSAG2D_GAME");
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
#if ENABLE_DEBUG_MODE
	FTDthread::FTDprocessingThread.DeleteProcessingThread();
#endif
	PSAG_LOGGER_PROCESS::FreeLogProcessing();

	return FrameworkResult;
}