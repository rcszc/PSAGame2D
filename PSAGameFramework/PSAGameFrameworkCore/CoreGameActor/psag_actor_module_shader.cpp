// psag_actor_module_shader.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

using VTUniformName = GraphicsEngineDataset::VirTextureUniformName;
inline VTUniformName VirtualTexUniform(bool* status, const string& name) {
	*status = !name.empty();
	// check uniform name valid ?
	if (!*status) return VTUniformName();
	// gen system uniform name.
	VTUniformName ResultTemp = {};
	ResultTemp.TexParamSampler  = "VirTexture" + name;
	ResultTemp.TexParamLayer    = "VirTexture" + name + "Layer";
	ResultTemp.TexParamCropping = "VirTexture" + name + "Cropping";
	ResultTemp.TexParamSize     = "VirTexture" + name + "Size";
	return ResultTemp;
}

#define FORMAT_BUFFER_SIZE 1024
string FormatVirTextureScript(const char* script, ...) {
	char LogCharTemp[FORMAT_BUFFER_SIZE] = {};

	va_list ParamArgs;
	va_start(ParamArgs, script);
	vsnprintf(LogCharTemp, FORMAT_BUFFER_SIZE, script, ParamArgs);
	va_end(ParamArgs);

	return string(LogCharTemp);
}

namespace GameActorCore {
	unordered_set<string> GameActorShader::TUUN_RegisterInfo = {};

	bool GameActorShader::ShaderImageTextureLoad(VirTextureUnique* ref_texture, const ImageRawData& image) {
		if (*ref_texture > NULL) return false;
		// check image data_pixels.
		if (image.ImagePixels.empty()) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader image null_pixel.");
			return false;
		}
		PSAG_SYS_GENERATE_KEY GenResourceID;
		*ref_texture = GenResourceID.PsagGenUniqueKey();
		// alloc virtual sampler texture.
		bool AllocSuccessFlag = VirTextureItemAlloc(*ref_texture, image);
		if (!AllocSuccessFlag) { // false: virtual texture engine err.
			*ref_texture = PSAG_VIR_TEXTURE_INVALID;
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed load_image.");
		}
		return AllocSuccessFlag;
	}

	GameActorShader::GameActorShader(const std::string& shader_frag, const Vector2T<uint32_t>& size) {
		// system actor system_default vert_shader.
		ShaderScript.vector_x = GraphicsShaderCode::GLOBALRES.Get().PublicShaders.ShaderVertTemplateActor;
		ShaderScript.vector_y = shader_frag;

		// render output resolution(size).
		__RENDER_RESOLUTION = size;
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader create: %u x %u", 
			__RENDER_RESOLUTION.vector_x, __RENDER_RESOLUTION.vector_y);

		// ATOMIC ENTITIES COUNTER.
		++ActorSystemAtomic::GLOBAL_PARAMS_SHADERS;
	}

	GameActorShader::~GameActorShader() {
		// delete virtual static vertex_dataset.
		VerStcDataItemFree(__ACTOR_VERTEX_ITEM);

		// texture engine free vir_texture.
		for (const auto& Texture : __VIR_TEXTURES_GROUP) {
			auto it = TUUN_RegisterInfo.find(Texture.VirTextureName);
			if (it != TUUN_RegisterInfo.end())
				TUUN_RegisterInfo.erase(it);
			// refe mode vir_texture no delete.
			if (Texture.VirTextureMode == GameComponents::VIR_TEXTURE_REFE)
				continue;
			VirTextureItemFree(Texture.VirTextureIndex);
		}
		__VIR_TEXTURES_GROUP.clear();

		// delete opengl shader.
		GraphicShaders->ResourceDelete(__ACTOR_SHADER_ITEM);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader delete.");

		// ATOMIC ENTITIES COUNTER.
		--ActorSystemAtomic::GLOBAL_PARAMS_SHADERS;
	}

	bool GameActorShader::CreateShaderResource(bool default_is_circle) {
		PSAG_SYS_GENERATE_KEY GenResourceID;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;

		ShaderProcess.ShaderLoaderPushVS(ShaderScript.vector_x, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GameActorScript::psag_shader_public_frag_header, StringScript);
		// load virtual textures mapping glsl.
		for (const auto& Texture : __VIR_TEXTURES_GROUP) {
			const char* VirTexName = Texture.VirTextureName.c_str();
			// const params: 8-items, 20250219 RCSZ. 
			string TexScript = FormatVirTextureScript(
				GameActorScript::psag_shader_public_frag_texture,
				VirTexName, VirTexName, VirTexName, VirTexName,
				VirTexName, VirTexName, VirTexName, VirTexName
			);
			ShaderProcess.ShaderLoaderPushFS(TexScript, StringScript);
		}
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader load textures: %u", 
			__VIR_TEXTURES_GROUP.size());
		// frag shader glsl:"main" function.
		ShaderProcess.ShaderLoaderPushFS(ShaderScript.vector_y, StringScript);

		if (!ShaderProcess.CreateCompileShader()) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed create.");
			return false;
		}
		// compile shader => storage shader.
		__ACTOR_SHADER_ITEM = GenResourceID.PsagGenUniqueKey();
		GraphicShaders->ResourceStorage(__ACTOR_SHADER_ITEM, &ShaderProcess);
		// find => shader_handle_temp => uniform.
		S_HANDLE = GraphicShaders->ResourceFind(__ACTOR_SHADER_ITEM);

		// shader vertex resource.
		__ACTOR_VERTEX_ITEM = GenResourceID.PsagGenUniqueKey();
		if (VerticesPosition != nullptr) {
			// vertex coord => shader vertex_group.
			vector<float> DatasetTemp = {};
			for (size_t i = 0; i < VerticesPosition->size(); ++i) {
				// vertex group, std: GL_VERT_01.
				vector<float> VertexGroup = {
					// pos: vec3, color: vec4, uv: vec2, normal: vec3
					(*VerticesPosition)[i].vector_x, (*VerticesPosition)[i].vector_y, 0.0f,
					ShaderDefaultColor.vector_x, ShaderDefaultColor.vector_y,
					ShaderDefaultColor.vector_z, ShaderDefaultColor.vector_w,
					(*VerticesUvCoord)[i].vector_x, (*VerticesUvCoord)[i].vector_y,
					0.0f, 0.0f, 0.0f
				};
				DatasetTemp.insert(DatasetTemp.begin(), VertexGroup.begin(), VertexGroup.end());
			}
			// upload static dataset.
			VerStcDataItemAlloc(__ACTOR_VERTEX_ITEM, DatasetTemp);
			PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader res create.");
			return true;
		}
		// non-vertex-data. => system_default.
		if (!default_is_circle) __ACTOR_VERTEX_ITEM = GetPresetRect();
		if (default_is_circle)  __ACTOR_VERTEX_ITEM = GetPresetCircle();
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader default_res create.");
		return true;
	}

	bool GameActorShader::ShaderVerticesLoad(GameActorShaderVerticesDESC& VER_DESC) {
		if (VER_DESC.VertexShaderEnable) {
			if (VER_DESC.VertexShaderScript.empty ()) {
				// vertex shader non-script.
				PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader vert_script empty.");
				return false;
			}
			// load user vertex_shader script.
			ShaderScript.vector_x = VER_DESC.VertexShaderScript;
		}
		// position num = uv num.
		if (VER_DESC.ShaderVertexCollision.size() != VER_DESC.ShaderVertexUvCoord.size()) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader vert: pos_size != uv_num.");
			return false;
		}
		VerticesPosition = &VER_DESC.ShaderVertexCollision;
		VerticesUvCoord  = &VER_DESC.ShaderVertexUvCoord;
		// shader frag non-texture out color.
		ShaderDefaultColor = VER_DESC.ShaderDefaultColor;
		return true;
	}

	bool GameActorShader::ShaderVirTextureLADD(const string& u_name, VirTextureUnique virtex) {
		GameComponents::VirtualTextureInfo VirTextureInfo = {};

		// check register uniform_name unique.
		if (TUUN_RegisterInfo.find(u_name) != TUUN_RegisterInfo.end()) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL,
				"game_actor shader u_name invalid, name: %s", u_name.c_str()
			);
			return false;
		}
		TUUN_RegisterInfo.insert(u_name);
		// 虚拟纹理引擎 => 验证引用纹理有效性. 
		if (!VirTextureExist(virtex)) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL,
				"game_actor shader ref_texture invalid, name: %s", u_name.c_str()
			);
			return false;
		}
		VirTextureInfo.VirTextureIndex = virtex;
		VirTextureInfo.VirTextureName  = u_name;
		VirTextureInfo.VirTextureMode  = GameComponents::VIR_TEXTURE_REFE;
		// valid virtual texture => add shader tex_group.
		__VIR_TEXTURES_GROUP.push_back(VirTextureInfo);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL,
			"game_actor shader ref_texture add, name: %s", u_name.c_str()
		);
		return true;
	}

	bool GameActorShader::ShaderImageLADD(const string& u_name, const ImageRawData& image) {
		GameComponents::VirtualTextureInfo VirTextureInfo = {};

		// check register uniform_name unique.
		if (TUUN_RegisterInfo.find(u_name) != TUUN_RegisterInfo.end()) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL,
				"game_actor shader u_name invalid, name: %s", u_name.c_str()
			);
			return false;
		}
		TUUN_RegisterInfo.insert(u_name);
		// 生成格式化 uniform.
		bool GenUniformStatus = false;
		VirTextureInfo.VirTextureUname = VirtualTexUniform(&GenUniformStatus, u_name);
		if (!GenUniformStatus) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader gen uniform failed.");
			return false;
		}
		// 虚拟纹理引擎分配纹理空间 3d =map=> 2d.
		VirTextureUnique GenTextureIndex = {};
		if (!ShaderImageTextureLoad(&GenTextureIndex, image)) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader gen vir_tex failed.");
			return false;
		}
		VirTextureInfo.VirTextureIndex = GenTextureIndex;
		VirTextureInfo.VirTextureName  = u_name;
		VirTextureInfo.VirTextureMode  = GameComponents::VIR_TEXTURE_ROOT;
		// valid virtual texture => add shader tex_group.
		__VIR_TEXTURES_GROUP.push_back(VirTextureInfo);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, 
			"game_actor shader texture add, name: %s", u_name.c_str()
		);
		return true;
	}

	// ******************************** upload shader uniform ********************************

	void GameActorShader::UniformSetContext(function<void()> context_func) {
		// opengl api function. [20240804]
		glUseProgram(S_HANDLE);
		context_func();
		glUseProgram(NULL);
	}
	// shader bind context => setting uniform values.
	void GameActorShader::UniformMatrix3x3(const char* name, const PsagMatrix3& matrix) { U_LOADER.UniformMatrix3x3(S_HANDLE, name, matrix); }
	void GameActorShader::UniformMatrix4x4(const char* name, const PsagMatrix4& matrix) { U_LOADER.UniformMatrix4x4(S_HANDLE, name, matrix); }
	void GameActorShader::UniformInt32(const char* name, const int32_t& value)          { U_LOADER.UniformInteger(S_HANDLE, name, value); }
	void GameActorShader::UniformFP32(const char* name, const float& value)             { U_LOADER.UniformFloat(S_HANDLE, name, value); }
	void GameActorShader::UniformVec2(const char* name, const Vector2T<float>& value)   { U_LOADER.UniformVec2(S_HANDLE, name, value); }
	void GameActorShader::UniformVec3(const char* name, const Vector3T<float>& value)   { U_LOADER.UniformVec3(S_HANDLE, name, value); }
	void GameActorShader::UniformVec4(const char* name, const Vector4T<float>& value)   { U_LOADER.UniformVec4(S_HANDLE, name, value); }
}