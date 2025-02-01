// framework_init_config.
// system_window config, system_imgui config.
#include "CoreFramework/psag_core_drivers.h"

// func: before initialization.
namespace PsagFrameworkCore {
	StaticStrLABEL PSAG_CONFIG = "PSAG_INIT_CONFIG";

	bool PSAGame2DFramework::FrameworkInitConfig(const std::string& gl_version) {
		// read file => decode json => config.
		
		PsagLow::PsagSupLoader::PsagFilesysDecoderLoader<PsagLow::PsagSupDCH::JsonDecodeChannel> 
			ConfigFiles[2] = 
		{
			{ "PSAGameFrameworkCore/framework_config.json" },
			{ "PSAGameFrameworkCore/framework_config_shaders.json" }
		};

		rapidjson::Document* ConfigDoc[2] 
			= { ConfigFiles[0].OperateDecoderOBJ()->GetDocumentOutput(), ConfigFiles[1].OperateDecoderOBJ()->GetDocumentOutput() };
		// check json data[2].
		if ((ConfigDoc[0]->HasParseError() || !ConfigDoc[0]->IsObject()) || 
			(ConfigDoc[1]->HasParseError() || !ConfigDoc[1]->IsObject())
		) {
			// processing json => object err.
			PSAG_LOGGER::PushLogger(LogError, PSAG_CONFIG, "config loader: json_object error.");
			return false;
		}

		bool CHECK_FLAG_WINDOW = DriversParams.DriversParamsLoader(ConfigDoc[0]);

		// config system shaders.
		GraphicsShaderCode::GraphicsShadersDESC GraphShaders = {};

		namespace LPS = PsagLow::PsagSupLoader;
		// find hash_value.
		auto GetShaderScript = [&](const char* key_name) {
			if (ConfigDoc[1]->HasMember(key_name) && (*ConfigDoc[1])[key_name].IsString())
				return LPS::EasyFileReadString((*ConfigDoc[1])[key_name].GetString());
			// failed find hash_value.
			PSAG_LOGGER::PushLogger(LogError, PSAG_CONFIG, "shader_config loader: failed find_value: %s", key_name);
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
		return CHECK_FLAG_WINDOW;
	}
}