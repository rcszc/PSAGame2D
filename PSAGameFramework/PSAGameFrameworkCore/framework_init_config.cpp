// framework_init_config.
// system_window config, system_imgui config.
#include "CoreFramework/psag_mainevent_support.h"

// func: before initialization.
namespace PsagFrameworkCore {
	bool PSAGame2DFramework::FrameworkInitConfig(const std::string& gl_version) {
		// framework load_json config_file.
		auto FileMode = PSAG_FILESYS_JSON::InputJsonFileName;

		PsagLow::PsagSupFilesysJson JsonConfigLoaderParams("PSAGameFrameworkCore/framework_config.json", FileMode);
		PsagLow::PsagSupFilesysJson JsonConfigLoaderShaders("PSAGameFrameworkCore/framework_config_shaders.json", FileMode);

		if (!JsonConfigLoaderParams.GetLoaderStatusFlag() || !JsonConfigLoaderShaders.GetLoaderStatusFlag()) {
			// load,read config_file err.
			PSAG_LOGGER::PushLogger(LogError, "PSAG_INIT", "config loader: failed load (json)file.");
			return false;
		}
		
		rapidjson::Document* JsonObject[2] = { 
			JsonConfigLoaderParams.GetLoaderJsonObj(), JsonConfigLoaderShaders.GetLoaderJsonObj() 
		};
		if ((JsonObject[0]->HasParseError() || !JsonObject[0]->IsObject()) || 
			(JsonObject[1]->HasParseError() || !JsonObject[1]->IsObject())
		) {
			// processing json => object err.
			PSAG_LOGGER::PushLogger(LogError, "PSAG_INIT", "config loader: json_object error.");
			return false;
		}

		if (JsonObject[0]->HasMember("WindowName") && (*JsonObject[0])["WindowName"].IsString())
			WindowInitConfig.WindowName = (*JsonObject[0])["WindowName"].GetString();

		if (JsonObject[0]->HasMember("WindowFull") && (*JsonObject[0])["WindowFull"].IsBool())
			WindowInitConfig.WindowFullFlag = (*JsonObject[0])["WindowFull"].GetBool();

		if (JsonObject[0]->HasMember("WindowSize") && (*JsonObject[0])["WindowSize"].IsArray()) {
			// type: uint32_t, array: 0:x(width), 1:y(height).
			WindowInitConfig.WindowSizeWidth  = (uint32_t)(*JsonObject[0])["WindowSize"][0].GetInt();
			WindowInitConfig.WindowSizeHeight = (uint32_t)(*JsonObject[0])["WindowSize"][1].GetInt();
		}

		if (JsonObject[0]->HasMember("RenderMSAA") && (*JsonObject[0])["RenderMSAA"].IsInt())
			RendererMSAA = (*JsonObject[0])["RenderMSAA"].GetInt();

		if (JsonObject[0]->HasMember("RenderWindowFixed") && (*JsonObject[0])["RenderWindowFixed"].IsBool())
			RendererWindowFixed = (*JsonObject[0])["RenderWindowFixed"].GetBool();

		if (JsonObject[0]->HasMember("RenderBasicSize") && (*JsonObject[0])["RenderBasicSize"].IsArray()) {
			// type: uint32_t, array: 0:u(x), 1:v(y).
			RenderingVirTexBasicSize.vector_x = (uint32_t)(*JsonObject[0])["RenderBasicSize"][0].GetInt();
			RenderingVirTexBasicSize.vector_y = (uint32_t)(*JsonObject[0])["RenderBasicSize"][1].GetInt();
		}

		if (JsonObject[0]->HasMember("RenderTexGenNum") && (*JsonObject[0])["RenderTexGenNum"].IsArray()) {
			// type: size_t, array: 0: 1/8x, 1: 1/4x, 2: 1/2x, 3: 1/1x.
			VirTexturesMax.Tex1Xnum = (size_t)(*JsonObject[0])["RenderTexGenNum"][0].GetInt();
			VirTexturesMax.Tex2Xnum = (size_t)(*JsonObject[0])["RenderTexGenNum"][1].GetInt();
			VirTexturesMax.Tex4Xnum = (size_t)(*JsonObject[0])["RenderTexGenNum"][2].GetInt();
			VirTexturesMax.Tex8Xnum = (size_t)(*JsonObject[0])["RenderTexGenNum"][3].GetInt();
		}
		ImGuiInitConfig.ShaderVersionStr = gl_version;

		if (JsonObject[0]->HasMember("GuiWindowRounding") && (*JsonObject[0])["GuiWindowRounding"].IsFloat())
			ImGuiInitConfig.WindowRounding = (*JsonObject[0])["GuiWindowRounding"].GetFloat();

		if (JsonObject[0]->HasMember("GuiFrameRounding") && (*JsonObject[0])["GuiFrameRounding"].IsFloat())
			ImGuiInitConfig.FrameRounding = (*JsonObject[0])["GuiFrameRounding"].GetFloat();

		if (JsonObject[0]->HasMember("GuiFontsFilepath") && (*JsonObject[0])["GuiFontsFilepath"].IsString())
			ImGuiInitConfig.FontsFilepath = (*JsonObject[0])["GuiFontsFilepath"].GetString();

		if (JsonObject[0]->HasMember("GuiFontsSize") && (*JsonObject[0])["GuiFontsSize"].IsFloat())
			ImGuiInitConfig.FontsGlobalSize = (*JsonObject[0])["GuiFontsSize"].GetFloat();

		if (JsonObject[0]->HasMember("GuiFontsColor") && (*JsonObject[0])["GuiFontsColor"].IsArray()) {
			// type: float32, array: 0: color_r, 1: color_g, 2: color_b, 3: color_a.
			ImGuiInitConfig.FontsGlobalColor.vector_x = (*JsonObject[0])["GuiFontsColor"][0].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_y = (*JsonObject[0])["GuiFontsColor"][1].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_z = (*JsonObject[0])["GuiFontsColor"][2].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_w = (*JsonObject[0])["GuiFontsColor"][3].GetFloat();
		}

		// config system shaders.
		GraphicsShaderCode::GraphicsShadersDESC GraphShaders = {};

		namespace PPS = PsagLow::PsagSupLoader;
		// find hash_value.
		auto GetShaderScript = [&](const char* key_name) {
			if (JsonObject[1]->HasMember(key_name) && (*JsonObject[1])[key_name].IsString())
				return PPS::TextFileLoader((*JsonObject[1])[key_name].GetString());
			// failed find hash_value.
			PSAG_LOGGER::PushLogger(LogError, "PSAG_INIT", "shader_config loader: failed find_value: %s", key_name);
			return std::string();
		};

		GraphShaders.PublicShaders.ShaderVertTemplateActor = GetShaderScript("public_vertex_template_actor");
		GraphShaders.PublicShaders.ShaderVertTemplate      = GetShaderScript("public_vertex_template");
		GraphShaders.PublicShaders.ShaderFragTools         = GetShaderScript("public_fragment_tools");
		GraphShaders.PublicShaders.ShaderFragHeader        = GetShaderScript("public_fragment_header");

		GraphShaders.PrivateShaders.ShaderFragLight      = GetShaderScript("private_fragment_light");
		GraphShaders.PrivateShaders.ShaderFragMultFilter = GetShaderScript("private_fragment_mulfilter");
		GraphShaders.PrivateShaders.ShaderFragBloomH     = GetShaderScript("private_fragment_bloom_h");
		GraphShaders.PrivateShaders.ShaderFragBloomV     = GetShaderScript("private_fragment_bloom_v");
		GraphShaders.PrivateShaders.ShaderFragFinalPhase = GetShaderScript("private_fragment_final_phase");
		GraphShaders.PrivateShaders.ShaderFragBackground = GetShaderScript("private_fragment_background");
		GraphShaders.PrivateShaders.ShaderFragParticle   = GetShaderScript("private_fragment_particle");
		GraphShaders.PrivateShaders.ShaderFragFxSequence = GetShaderScript("private_fragment_fx_sequence");
		
		GraphicsShaderCode::GLOBALRES.Set(GraphShaders);

		// use system default.
		FrameworkGraphicsParams = INIT_PARAMETERS();
		// config success.
		return true;
	}
}