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
		RenderingVirTextureSizeBase = Vector2T<uint32_t>(2048, 2048);

		ImGuiInitConfig.ShaderVersionStr = gl_version;
		ImGuiInitConfig.WindowRounding   = 3.8f;
		ImGuiInitConfig.FrameRounding    = 5.8f;

		ImGuiInitConfig.FontsFilepath    = "PSAGameSystemFiles/JetBrainsMonoBold.ttf";
		ImGuiInitConfig.FontsGlobalColor = Vector4T<float>(0.0f, 1.0f, 1.0f, 0.92f);
		ImGuiInitConfig.FontsGlobalSize  = 46.0f;

		// use system default.
		FrameworkGraphicsParams = INIT_PARAMETERS();

		// config system shaders.
		GraphicsShaderCode::GraphicsShadersDESC GraphShaders = {};

		GraphShaders.PublicShaders.ShaderVertTemplate = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/public/psag_graph_template.vert");
		GraphShaders.PublicShaders.ShaderFragTools    = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/public/psag_graph_tools.frag");
		GraphShaders.PublicShaders.ShaderFragHeader   = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/public/psag_graph_header.frag");

		GraphShaders.PrivateShaders.shaderFragColorFilter = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_colorfilter.frag");
		GraphShaders.PrivateShaders.ShaderFragBloomH      = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_bloom_h.frag");
		GraphShaders.PrivateShaders.ShaderFragBloomV      = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_bloom_v.frag");
		GraphShaders.PrivateShaders.ShaderFragFinalPhase  = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_final.frag");
		GraphShaders.PrivateShaders.ShaderFragBackground  = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_background.frag");
		GraphShaders.PrivateShaders.ShaderFragLight       = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_vollight.frag");
		GraphShaders.PrivateShaders.ShaderFragParticle    = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_particle.frag");
		GraphShaders.PrivateShaders.ShaderFragFxSequence  = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_fx_sequence.frag");
		
		// config system textures_max, 1x(base:1/8), 2x(base:1/4), 4x(base:1/2), 1x(base:1/1).
		VirTexturesMax.Tex1Xnum = 16;
		VirTexturesMax.Tex2Xnum = 32;
		VirTexturesMax.Tex4Xnum = 32;
		VirTexturesMax.Tex8Xnum = 48;

		GraphicsShaderCode::GLOBALRES.Set(GraphShaders);
	}
}