// framework_init_config.
// system_window config, system_imgui config.
#include "CoreFramework/psag_mainevent_support.h"

// func: before initialization.
namespace PsagFrameworkCore {
	void PSAGame2DFramework::FrameworkInitConfig(const std::string& gl_version) {
		// config.
		WindowInitConfig.WindowName     = "PSAGmae2D - TEST";
		WindowInitConfig.WindowFullFlag = PSAG_FALSE;

		WindowInitConfig.WindowSizeWidth  = 1680;
		WindowInitConfig.WindowSizeHeight = 945;

		RendererMSAA = 4;
		RendererWindowFixed = true;

		ImGuiInitConfig.ShaderVersionStr = gl_version;
		ImGuiInitConfig.WindowRounding   = 3.8f;
		ImGuiInitConfig.FrameRounding    = 5.8f;

		ImGuiInitConfig.FontsFilepath    = "PSAGameSystemFiles/JetBrainsMonoBold.ttf";
		ImGuiInitConfig.FontsGlobalColor = Vector4T<float>(0.0f, 1.0f, 1.0f, 0.92f);
		ImGuiInitConfig.FontsGlobalSize  = 26.5f;

		FrameworkGraphicsParams = INIT_PARAMETERS(); // NORMAL
	}
}