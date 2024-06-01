// PSAGameFramework. 2024_03_31. [闭源]
// @PomeloStar Studio 2024-2025 GameFramework(Engine).
// Developers: 
/*
* Engine Version: 0.1.2 Alpha (10.20240514)
* MSVC C11, C++17 x64 Release
* OpenGL GLSL: 4.6
* OpenGL GLFW: 3.0
* ImGui: ImGui For OpenGL3
* ImageLoader: STB_IMAGE, STB_IMAGE_WRITE
* JsonLoader: RapidJSON
* SoundLoader: Audiere
* FileCompress: Zlib
* PhysicsSystem: Box2D
*/
// Engine Update: 2024_05_25.
#define PSAG2D_ENGINE_VER 12.20240514
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
#pragma comment(lib, "audiere.lib  ")
#pragma comment(lib, "zlibwapi.lib ")
#pragma comment(lib, "box2d.lib    ")

int main() {
	int FrameworkResult = -1;
	PSAG_LOGGER_FILESYS::StartLogFileProcess("PSAGameSystemLogs/");
	{
		PsagFrameworkCore::PSAGame2DFramework* MainPSAGame2D = new PsagFrameworkCore::PSAGame2DFramework();
		PsagFrameworkStart::CorePsagMainStart FrameworkStarter = {};
		// add start_item: object.
		FrameworkStarter.UniqueFrameworkObj(MainPSAGame2D);
		// start framework(engine)[exe].
		FrameworkStarter.StartPsaGame(PSAG2D_ENGINE_VER, 125);
		// framework free exit.
		FrameworkResult = FrameworkStarter.FreeFramework();
	}
	PSAG_LOGGER_FILESYS::FreeLogFileProcess();
	return FrameworkResult;
}