// framework_init_config.
// system_window config, system_imgui config.
#include "CoreFramework/psag_mainevent_support.h"

// func: before initialization.
namespace PsagFrameworkCore {
	bool PSAGame2DFramework::FrameworkInitConfig(const std::string& gl_version) {
		// read file => decode json => config.
		
		PsagLow::PsagSupLoader::PsagFilesysDecoderLoader<PsagLow::PsagSupDCH::JsonDecodeChannel> 
			ConfigFiles[2] = 
		{
			{ "PSAGameFrameworkCore/framework_config.json" },
			{ "PSAGameFrameworkCore/framework_config_shaders.json" }
		};

		rapidjson::Document* DecodeOutput[2] 
			= { ConfigFiles[0].OperateDecoderOBJ()->GetDocumentOutput(), ConfigFiles[1].OperateDecoderOBJ()->GetDocumentOutput() };
		// check json data[2].
		if ((DecodeOutput[0]->HasParseError() || !DecodeOutput[0]->IsObject()) || 
			(DecodeOutput[1]->HasParseError() || !DecodeOutput[1]->IsObject())
		) {
			// processing json => object err.
			PSAG_LOGGER::PushLogger(LogError, "PSAG_INIT", "config loader: json_object error.");
			return false;
		}

		if (DecodeOutput[0]->HasMember("WindowName") && (*DecodeOutput[0])["WindowName"].IsString())
			WindowInitConfig.WindowName = (*DecodeOutput[0])["WindowName"].GetString();

		if (DecodeOutput[0]->HasMember("WindowFull") && (*DecodeOutput[0])["WindowFull"].IsBool())
			WindowInitConfig.WindowFullFlag = (*DecodeOutput[0])["WindowFull"].GetBool();

		if (DecodeOutput[0]->HasMember("WindowSize") && (*DecodeOutput[0])["WindowSize"].IsArray()) {
			// type: uint32_t, array: 0:x(width), 1:y(height).
			WindowInitConfig.WindowSizeWidth  = (uint32_t)(*DecodeOutput[0])["WindowSize"][0].GetInt();
			WindowInitConfig.WindowSizeHeight = (uint32_t)(*DecodeOutput[0])["WindowSize"][1].GetInt();
		}

		if (DecodeOutput[0]->HasMember("RenderMSAA") && (*DecodeOutput[0])["RenderMSAA"].IsInt())
			RendererMSAA = (*DecodeOutput[0])["RenderMSAA"].GetInt();

		if (DecodeOutput[0]->HasMember("RenderWindowFixed") && (*DecodeOutput[0])["RenderWindowFixed"].IsBool())
			RendererWindowFixed = (*DecodeOutput[0])["RenderWindowFixed"].GetBool();

		if (DecodeOutput[0]->HasMember("RenderBasicSize") && (*DecodeOutput[0])["RenderBasicSize"].IsArray()) {
			// type: uint32_t, array: 0:u(x), 1:v(y).
			RenderingVirTexBasicSize.vector_x = (uint32_t)(*DecodeOutput[0])["RenderBasicSize"][0].GetInt();
			RenderingVirTexBasicSize.vector_y = (uint32_t)(*DecodeOutput[0])["RenderBasicSize"][1].GetInt();
		}

		if (DecodeOutput[0]->HasMember("RenderGenTexure") && (*DecodeOutput[0])["RenderGenTexure"].IsArray()) {
			// type: size_t, array: 0: 1/8x, 1: 1/4x, 2: 1/2x, 3: 1/1x.
			VirTexturesMax.Tex1Xnum = (size_t)(*DecodeOutput[0])["RenderGenTexure"][0].GetInt();
			VirTexturesMax.Tex2Xnum = (size_t)(*DecodeOutput[0])["RenderGenTexure"][1].GetInt();
			VirTexturesMax.Tex4Xnum = (size_t)(*DecodeOutput[0])["RenderGenTexure"][2].GetInt();
			VirTexturesMax.Tex8Xnum = (size_t)(*DecodeOutput[0])["RenderGenTexure"][3].GetInt();
		}
		ImGuiInitConfig.ShaderVersionStr = gl_version;

		if (DecodeOutput[0]->HasMember("GuiWindowRounding") && (*DecodeOutput[0])["GuiWindowRounding"].IsFloat())
			ImGuiInitConfig.WindowRounding = (*DecodeOutput[0])["GuiWindowRounding"].GetFloat();

		if (DecodeOutput[0]->HasMember("GuiFrameRounding") && (*DecodeOutput[0])["GuiFrameRounding"].IsFloat())
			ImGuiInitConfig.FrameRounding = (*DecodeOutput[0])["GuiFrameRounding"].GetFloat();

		if (DecodeOutput[0]->HasMember("GuiFontsFilepath") && (*DecodeOutput[0])["GuiFontsFilepath"].IsString())
			ImGuiInitConfig.FontsFilepath = (*DecodeOutput[0])["GuiFontsFilepath"].GetString();

		if (DecodeOutput[0]->HasMember("GuiFontsSize") && (*DecodeOutput[0])["GuiFontsSize"].IsFloat())
			ImGuiInitConfig.FontsGlobalSize = (*DecodeOutput[0])["GuiFontsSize"].GetFloat();

		if (DecodeOutput[0]->HasMember("GuiFontsColor") && (*DecodeOutput[0])["GuiFontsColor"].IsArray()) {
			// type: float32, array: 0: color_r, 1: color_g, 2: color_b, 3: color_a.
			ImGuiInitConfig.FontsGlobalColor.vector_x = (*DecodeOutput[0])["GuiFontsColor"][0].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_y = (*DecodeOutput[0])["GuiFontsColor"][1].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_z = (*DecodeOutput[0])["GuiFontsColor"][2].GetFloat();
			ImGuiInitConfig.FontsGlobalColor.vector_w = (*DecodeOutput[0])["GuiFontsColor"][3].GetFloat();
		}

		// config system shaders.
		GraphicsShaderCode::GraphicsShadersDESC GraphShaders = {};

		namespace LPS = PsagLow::PsagSupLoader;
		// find hash_value.
		auto GetShaderScript = [&](const char* key_name) {
			if (DecodeOutput[1]->HasMember(key_name) && (*DecodeOutput[1])[key_name].IsString())
				return LPS::EasyFileReadString((*DecodeOutput[1])[key_name].GetString());
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