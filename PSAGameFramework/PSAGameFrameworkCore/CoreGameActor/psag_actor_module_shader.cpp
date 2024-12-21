// psag_actor_module_shader.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameActorCore {

	bool GameActorShader::ShaderImageTextureLoad(VirTextureUnqiue* ref_texture, const ImageRawData& image) {
		if (!CheckRepeatTex(*ref_texture)) return false;

		if (!image.ImagePixels.empty()) {
			PSAG_SYS_GENERATE_KEY GenResourceID;
			*ref_texture = GenResourceID.PsagGenUniqueKey();
			// alloc virtual sampler texture.
			bool VirTextureFlag = VirTextureItemAlloc(*ref_texture, image);
			if (!VirTextureFlag)
				PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed load_image.");
			return VirTextureFlag;
		}
		PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader image null_pixel.");
		return false;
	}

	GameActorShader::GameActorShader(const std::string& SHADER_FRAG, const Vector2T<uint32_t>& RESOLUTION) {
		// system actor system_default vert_shader.
		ShaderScript.vector_x = GraphicsShaderCode::GLOBALRES.Get().PublicShaders.ShaderVertTemplateActor;
		ShaderScript.vector_y = SHADER_FRAG;

		__RENDER_RESOLUTION = RESOLUTION;
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader create: %u x %u", RESOLUTION.vector_x, RESOLUTION.vector_y);
	}

	GameActorShader::~GameActorShader() {
		// delete virtual static vertex_dataset.
		VerStcDataItemFree(__ACTOR_VERTEX_ITEM);

		// 引用纹理情况下, 不由"GameActorShader"回收.
		if (!MappingTextureFlag && __VIR_TEXTURE_ITEM != NULL)
			VirTextureItemFree(__VIR_TEXTURE_ITEM);

		// delete opengl shader.
		GraphicShaders->ResourceDelete(__ACTOR_SHADER_ITEM);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader delete.");
	}

	bool GameActorShader::CreateShaderResource(bool default_is_circle) {
		PSAG_SYS_GENERATE_KEY GenResourceID;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;

		ShaderProcess.ShaderLoaderPushVS(ShaderScript.vector_x, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GameActorScript::psag_shader_public_frag_header, StringScript);
		ShaderProcess.ShaderLoaderPushFS(GameActorScript::psag_shader_public_frag_texnor, StringScript);
		// hdr_texture index exist => add hdr uniform code.
		if (__VIR_TEXTURE_HDR_ITEM != NULL) {
			ShaderProcess.ShaderLoaderPushFS(GameActorScript::psag_shader_public_frag_texhdr, StringScript);
			PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor enable texture hdr_uniform.");
		}
		// system preset "main".
		ShaderProcess.ShaderLoaderPushFS(ShaderScript.vector_y, StringScript);

		if (ShaderProcess.CreateCompileShader()) {
			__ACTOR_SHADER_ITEM = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(__ACTOR_SHADER_ITEM, &ShaderProcess);
		}
		else {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed create.");
			return false;
		}
		// find => shader_handle_temp => uniform.
		S_HANDLE = GraphicShaders->ResourceFind(__ACTOR_SHADER_ITEM);

		__ACTOR_VERTEX_ITEM = GenResourceID.PsagGenUniqueKey();
		if (VerticesPosition != nullptr) {
			// vertex coord => shader vertex_group.
			vector<float> DatasetTemp = {};
			for (size_t i = 0; i < VerticesPosition->size(); ++i) {
				// vertex group, std: GL_VERT_01.
				vector<float> VertexGroup = {
					// pos: vec3, color: vec4, uv: vec2, normal: vec3
					(*VerticesPosition)[i].vector_x, (*VerticesPosition)[i].vector_y, 0.0f,
					ShaderDefaultColor.vector_x, ShaderDefaultColor.vector_y, ShaderDefaultColor.vector_z, ShaderDefaultColor.vector_w,
					(*VerticesUvCoord)[i].vector_x, (*VerticesUvCoord)[i].vector_y,
					0.0f, 0.0f, 0.0f
				};
				DatasetTemp.insert(DatasetTemp.begin(), VertexGroup.begin(), VertexGroup.end());
			}
			// upload static dataset.
			VerStcDataItemAlloc(__ACTOR_VERTEX_ITEM, DatasetTemp);
		}
		else {
			// non-vertex-data. => system_default.
			if (!default_is_circle) __ACTOR_VERTEX_ITEM = GetPresetRect();
			if (default_is_circle)  __ACTOR_VERTEX_ITEM = GetPresetCircle();
		}
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader resource create.");
		return true;
	}

	bool GameActorShader::CheckRepeatTex(VirTextureUnqiue virtex) {
		if (VirTextureExist(virtex)) {
			PushLogger(LogWarning, PSAGM_ACTOR_CORE_LABEL, "game_actor shader texture duplicate.");
			return false;
		}
		return true;
	}

	bool GameActorShader::ShaderVerticesLoad(GameActorShaderVerticesDESC& VER_DESC) {
		if (VER_DESC.VertexShaderEnable) {
			if (VER_DESC.VertexShaderScript.empty()) {
				// vertex shader non-script.
				PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader vert_script empty.");
				return false;
			}
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

	bool GameActorShader::ShaderLoadVirTexture(VirTextureUnqiue virtex) {
		if (!CheckRepeatTex(__VIR_TEXTURE_ITEM))
			return false;

		if (VirTextureExist(virtex)) {
			__VIR_TEXTURE_ITEM = virtex;
			__VIR_UNIFORM_ITEM = SystemPresetUnameN();
			// mapping texture flag.
			MappingTextureFlag = true;
			return true;
		}
		PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, 
			"game_actor shader invalid ref [nor] virtual_texture."
		);
		return false;
	}

	bool GameActorShader::ShaderLoadVirTextureHDR(VirTextureUnqiue virtex) {
		if (!CheckRepeatTex(__VIR_TEXTURE_ITEM))
			return false;

		if (VirTextureExist(virtex)) {
			__VIR_TEXTURE_ITEM = virtex;
			__VIR_UNIFORM_ITEM = SystemPresetUnameH();
			// mapping texture flag.
			MappingTextureFlag = true;
			return true;
		}
		PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, 
			"game_actor shader invalid ref [hdr] virtual_texture."
		);
		return false;
	}

	bool GameActorShader::ShaderImageLoad(const ImageRawData& image) {
		bool TextureLoaderFlag = ShaderImageTextureLoad(&__VIR_TEXTURE_ITEM, image);
		if (TextureLoaderFlag)
			__VIR_UNIFORM_ITEM = SystemPresetUnameN();
		return TextureLoaderFlag;
	}

	bool GameActorShader::ShaderImageLoadHDR(const ImageRawData& image) {
		bool HDRTextureLoaderFlag = ShaderImageTextureLoad(&__VIR_TEXTURE_HDR_ITEM, image);
		if (HDRTextureLoaderFlag)
			__VIR_UNIFORM_HDR_ITEM = SystemPresetUnameH();
		return HDRTextureLoaderFlag;
	}

	// ******************************** upload shader uniform ********************************

	void GameActorShader::UniformSetContext(function<void()> context_func) {
		// opengl api function. [20240804]
		glUseProgram(S_HANDLE);
		context_func();
		glUseProgram(NULL);
	}

	void GameActorShader::UniformMatrix3x3(const char* name, const PsagMatrix3& matrix) { U_LOADER.UniformMatrix3x3(S_HANDLE, name, matrix); }
	void GameActorShader::UniformMatrix4x4(const char* name, const PsagMatrix4& matrix) { U_LOADER.UniformMatrix4x4(S_HANDLE, name, matrix); }
	void GameActorShader::UniformInt32(const char* name, const int32_t& value)          { U_LOADER.UniformInteger(S_HANDLE, name, value); }
	void GameActorShader::UniformFP32(const char* name, const float& value)             { U_LOADER.UniformFloat(S_HANDLE, name, value); }
	void GameActorShader::UniformVec2(const char* name, const Vector2T<float>& value)   { U_LOADER.UniformVec2(S_HANDLE, name, value); }
	void GameActorShader::UniformVec3(const char* name, const Vector3T<float>& value)   { U_LOADER.UniformVec3(S_HANDLE, name, value); }
	void GameActorShader::UniformVec4(const char* name, const Vector4T<float>& value)   { U_LOADER.UniformVec4(S_HANDLE, name, value); }
}