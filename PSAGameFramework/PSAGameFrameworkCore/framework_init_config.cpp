// framework_init_config.
// system_window config, system_imgui config.
#include "CoreFramework/psag_mainevent_support.h"

// func: before initialization.
namespace PsagFrameworkCore {
	bool PSAGame2DFramework::FrameworkInitConfig(const std::string& gl_version) {
		// framework load_json config_file.
		PsagLow::PsagSupFilesysJson JsonConfigLoader("PSAGameFrameworkCore/framework_config.json", PSAG_FILESYS_JSON::InputJsonFileName);

		if (!JsonConfigLoader.GetLoaderStatusFlag()) {
			// load,read config_file err.
			PSAG_LOGGER::PushLogger(LogError, "PSAG_INIT", "config loader: failed load (json)file.");
			return false;
		}

		auto JsonObject = JsonConfigLoader.GetLoaderJsonObj();
		if (JsonObject->HasParseError() || !JsonObject->IsObject()) {
			// processing json => object err.
			PSAG_LOGGER::PushLogger(LogError, "PSAG_INIT", "config loader: json_object error.");
			return false;
		}

		if (JsonObject->HasMember("WindowName") && (*JsonObject)["WindowName"].IsString())
			WindowInitConfig.WindowName = (*JsonObject)["WindowName"].GetString();

		if (JsonObject->HasMember("WindowFull") && (*JsonObject)["WindowFull"].IsBool())
			WindowInitConfig.WindowFullFlag = (*JsonObject)["WindowFull"].GetBool();

		if (JsonObject->HasMember("WindowSize") && (*JsonObject)["WindowSize"].IsArray()) {
			// type: uint32_t, array: 0:x(width), 1:y(height).
			WindowInitConfig.WindowSizeWidth  = (uint32_t)(*JsonObject)["WindowSize"][0].GetInt();
			WindowInitConfig.WindowSizeHeight = (uint32_t)(*JsonObject)["WindowSize"][1].GetInt();
		}

		if (JsonObject->HasMember("RenderMSAA") && (*JsonObject)["RenderMSAA"].IsInt())
			RendererMSAA = (*JsonObject)["RenderMSAA"].GetInt();

		if (JsonObject->HasMember("RenderWindowFixed") && (*JsonObject)["RenderWindowFixed"].IsBool())
			RendererWindowFixed = (*JsonObject)["RenderWindowFixed"].GetBool();

		if (JsonObject->HasMember("RenderBasicSize") && (*JsonObject)["RenderBasicSize"].IsArray()) {
			// type: uint32_t, array: 0:u(x), 1:v(y).
			RenderingVirTexBasicSize.vector_x = (uint32_t)(*JsonObject)["RenderBasicSize"][0].GetInt();
			RenderingVirTexBasicSize.vector_y = (uint32_t)(*JsonObject)["RenderBasicSize"][1].GetInt();
		}

		if (JsonObject->HasMember("RenderTexGenNum") && (*JsonObject)["RenderTexGenNum"].IsArray()) {
			// type: size_t, array: 0: 1/8x, 1: 1/4x, 2: 1/2x, 3: 1/1x.
			VirTexturesMax.Tex1Xnum = (size_t)(*JsonObject)["RenderTexGenNum"][0].GetInt();
			VirTexturesMax.Tex2Xnum = (size_t)(*JsonObject)["RenderTexGenNum"][1].GetInt();
			VirTexturesMax.Tex4Xnum = (size_t)(*JsonObject)["RenderTexGenNum"][2].GetInt();
			VirTexturesMax.Tex8Xnum = (size_t)(*JsonObject)["RenderTexGenNum"][3].GetInt();
		}
		ImGuiInitConfig.ShaderVersionStr = gl_version;

		if (JsonObject->HasMember("GuiWindowRounding") && (*JsonObject)["GuiWindowRounding"].IsFloat())
			ImGuiInitConfig.WindowRounding = (*JsonObject)["GuiWindowRounding"].GetFloat();

		if (JsonObject->HasMember("GuiFrameRounding") && (*JsonObject)["GuiFrameRounding"].IsFloat())
			ImGuiInitConfig.FrameRounding = (*JsonObject)["GuiFrameRounding"].GetFloat();

		if (JsonObject->HasMember("GuiFontsFilepath") && (*JsonObject)["GuiFontsFilepath"].IsString())
			ImGuiInitConfig.FontsFilepath = (*JsonObject)["GuiFontsFilepath"].GetString();

		if (JsonObject->HasMember("GuiFontsSize") && (*JsonObject)["GuiFontsSize"].IsFloat())
			ImGuiInitConfig.FontsGlobalSize = (*JsonObject)["GuiFontsSize"].GetFloat();

		if (JsonObject->HasMember("GuiFontsColor") && (*JsonObject)["GuiFontsColor"].IsArray()) {
			// type: float32, array: 0: color_r, 1: color_g, 2: color_b, 3: color_a.
			ImGuiInitConfig.FontsGlobalColor.vector_x = (*JsonObject)["GuiFontsColor"][0].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_y = (*JsonObject)["GuiFontsColor"][1].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_z = (*JsonObject)["GuiFontsColor"][2].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_w = (*JsonObject)["GuiFontsColor"][3].GetFloat();
		}

		// use system default.
		FrameworkGraphicsParams = INIT_PARAMETERS();

		// config system shaders.
		GraphicsShaderCode::GraphicsShadersDESC GraphShaders = {};

		GraphShaders.PublicShaders.ShaderVertTemplate = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/public/psag_graph_template.vert");
		GraphShaders.PublicShaders.ShaderFragTools    = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/public/psag_graph_tools.frag");
		GraphShaders.PublicShaders.ShaderFragHeader   = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/public/psag_graph_header.frag");

		GraphShaders.PrivateShaders.ShaderFragLight       = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_final_vollight.frag");
		GraphShaders.PrivateShaders.shaderFragMultFilter  = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_final_multfilter.frag");
		GraphShaders.PrivateShaders.ShaderFragBloomH      = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_final_blurh.frag");
		GraphShaders.PrivateShaders.ShaderFragBloomV      = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_final_blurv.frag");
		GraphShaders.PrivateShaders.ShaderFragFinalPhase  = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_final.frag");
		GraphShaders.PrivateShaders.ShaderFragBackground  = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_background.frag");
		GraphShaders.PrivateShaders.ShaderFragParticle    = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_particle.frag");
		GraphShaders.PrivateShaders.ShaderFragFxSequence  = PsagLow::PsagSupLoader::TextFileLoader("PSAGameFrameworkCore/CoreShaderScript/private/psag_graph_fx_sequence.frag");
		
		GraphicsShaderCode::GLOBALRES.Set(GraphShaders);
		// config success.
		return true;
	}
}