// psag_actor_module_render.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameActorCore {

	GameActorShader::GameActorShader(const std::string& SHADER_FRAG, const Vector2T<uint32_t>& RESOLUTION) {
		// system actor default vert_shader.
		ShaderScript.vector_x = GameActorScript::PsagShaderPublicVS;
		ShaderScript.vector_y = SHADER_FRAG;

		__RENDER_RESOLUTION = RESOLUTION;
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader create: %u x %u", RESOLUTION.vector_x, RESOLUTION.vector_y);
	}

	GameActorShader::~GameActorShader() {
		// delete virtual static vertex_dataset.
		VerStcDataItemFree(__ACTOR_VERTEX_ITEM);

		// 引用虚拟纹理情况下, 不由"GameActorShader"回收.
		if (!ReferVirTextureFlag && __VIR_TEXTURE_ITEM != NULL)
			VirTextureItemFree(__VIR_TEXTURE_ITEM);

		// delete opengl shader.
		LLRES_Shaders->ResourceDelete(__ACTOR_SHADER_ITEM);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader delete.");
	}

	bool GameActorShader::CreateShaderResource() {
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;

		ShaderProcess.ShaderLoaderPushVS(ShaderScript.vector_x, StringScript);
		ShaderProcess.ShaderLoaderPushFS(ShaderScript.vector_y, StringScript);

		if (ShaderProcess.CreateCompileShader()) {
			__ACTOR_SHADER_ITEM = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(__ACTOR_SHADER_ITEM, &ShaderProcess);
		}
		else {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed create.");
			return false;
		}
		// find => shader_handle_temp => uniform.
		OpenGLShaderTemp = LLRES_Shaders->ResourceFind(__ACTOR_SHADER_ITEM);

		__ACTOR_VERTEX_ITEM = GenResourceID.PsagGenTimeKey();
		if (VerPosition != nullptr) {
			// vertex coord => shader vertex_group.
			vector<float> DatasetTemp = {};
			for (size_t i = 0; i < VerPosition->size(); ++i) {
				// vertex group, std: GL_VERT_01.
				vector<float> VertexGroup = {
					// pos: vec3, color: vec4, uv: vec2, normal: vec3
					(*VerPosition)[i].vector_x, (*VerPosition)[i].vector_y, 0.0f,
					ShaderDebugColor.vector_x, ShaderDebugColor.vector_y, ShaderDebugColor.vector_z, ShaderDebugColor.vector_w,
					(*VerUvCoord)[i].vector_x, (*VerUvCoord)[i].vector_y,
					0.0f, 0.0f, 0.0f
				};
				DatasetTemp.insert(DatasetTemp.begin(), VertexGroup.begin(), VertexGroup.end());
			}
			// upload static dataset.
			VerStcDataItemAlloc(__ACTOR_VERTEX_ITEM, DatasetTemp);
		}
		else {
			// non-vertex-data. => system_default.
			__ACTOR_VERTEX_ITEM = GetPresetRect();
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

	bool GameActorShader::ShaderLoadVertices(GameActorShaderVerticesDESC& VER_DESC) {
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
		VerPosition = &VER_DESC.ShaderVertexCollision;
		VerUvCoord = &VER_DESC.ShaderVertexUvCoord;
		// shader frag non-texture out color.
		ShaderDebugColor = VER_DESC.ShaderDebugColor;
		return true;
	}

	bool GameActorShader::ShaderLoadVirTexture(VirTextureUnqiue virtex) {
		if (!CheckRepeatTex(__VIR_TEXTURE_ITEM))
			return false;

		if (VirTextureExist(virtex)) {
			__VIR_TEXTURE_ITEM = virtex;
			__VIR_UNIFORM_ITEM = SystemPresetUname();
			ReferVirTextureFlag = true;
			return true;
		}
		PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader invalid vir_texture.");
		return false;
	}

	bool GameActorShader::ShaderLoadImage(const ImageRawData& image) {
		if (!CheckRepeatTex(__VIR_TEXTURE_ITEM)) return false;

		if (!image.ImagePixels.empty()) {
			PSAG_SYSGEN_TIME_KEY GenResourceID;
			__VIR_TEXTURE_ITEM = GenResourceID.PsagGenTimeKey();
			// alloc virtual sampler texture.
			if (!VirTextureItemAlloc(__VIR_TEXTURE_ITEM, image)) {
				PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed load_image.");
				return false;
			}
			__VIR_UNIFORM_ITEM = SystemPresetUname();
			return true;
		}
		return false;
	}

	// ******************************** upload shader uniform ********************************

	void GameActorShader::UniformSetContext(function<void()> context_func) {
		// opengl api function. [20240804]
		glUseProgram(OpenGLShaderTemp);
		context_func();
		glUseProgram(NULL);
	}

	void GameActorShader::UniformMatrix3x3(const char* name, const PsagMatrix3& matrix) {
		ShaderUniformLoader.UniformMatrix3x3(OpenGLShaderTemp, name, matrix);
	}
	void GameActorShader::UniformMatrix4x4(const char* name, const PsagMatrix4& matrix) {
		ShaderUniformLoader.UniformMatrix4x4(OpenGLShaderTemp, name, matrix);
	}
	void GameActorShader::UniformInt32(const char* name, const int32_t& value) {
		ShaderUniformLoader.UniformInteger(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformFP32(const char* name, const float& value) {
		ShaderUniformLoader.UniformFloat(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformVec2(const char* name, const Vector2T<float>& value) {
		ShaderUniformLoader.UniformVec2(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformVec3(const char* name, const Vector3T<float>& value) {
		ShaderUniformLoader.UniformVec3(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformVec4(const char* name, const Vector4T<float>& value) {
		ShaderUniformLoader.UniformVec4(OpenGLShaderTemp, name, value);
	}
}