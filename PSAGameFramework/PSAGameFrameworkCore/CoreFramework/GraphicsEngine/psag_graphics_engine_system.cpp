// psag_graphics_engine.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;
using namespace GraphicsShaderCode;

namespace GraphicsEngineMatrix {
	MatrixTransParams PsagGLEngineMatrix::MatrixWorldCamera = {};
	PsagMatrix4       PsagGLEngineMatrix::MatrixDataRect    = {};
	PsagMatrix4       PsagGLEngineMatrix::MatrixDataWindow  = {};

	glm::mat4 PsagGLEngineMatrix::GetOrthoProjMatrix(float scale_size) {
		return glm::ortho(
			-SystemRenderingOrthoSpace * scale_size, SystemRenderingOrthoSpace * scale_size,
			-SystemRenderingOrthoSpace, SystemRenderingOrthoSpace, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace
		);
	}

	glm::mat4 PsagGLEngineMatrix::UpdateCalcMatrix(const glm::mat4& in_matrix, const MatrixTransParams& params) {
		glm::mat4 MatrixCalcTemp = in_matrix;
		// camera position trans.
		MatrixCalcTemp = glm::translate(
			MatrixCalcTemp, glm::vec3(
				-params.MatrixPosition.vector_x * 0.1f,
				 params.MatrixPosition.vector_y * 0.1f,
				0.0f
			));
		// camera rotate trans.
		MatrixCalcTemp = glm::rotate(MatrixCalcTemp, params.MatrixRotate, glm::vec3(0.0f, 0.0f, 1.0f));
		// camera scale trans.
		MatrixCalcTemp = glm::scale(
			MatrixCalcTemp, glm::vec3(
				params.MatrixScale.vector_x,
				params.MatrixScale.vector_x,
				1.0f
			));
		return MatrixCalcTemp;
	}

	PsagMatrix4 PsagGLEngineMatrix::UpdateEncodeMatrix(const glm::mat4& matrix, float scale) {
		PsagMatrix4 EncMatrixTemp = {};
		// matrix_calc: porj * view.
		glm::mat4 CameraMatrix = GetOrthoProjMatrix(scale) * glm::inverse(matrix);
		// convert: glm matrix => psag matrix.
		const float* glmmatptr = glm::value_ptr(CameraMatrix);
		memcpy_s(EncMatrixTemp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));
		return EncMatrixTemp;
	}
}

float __GRAPHICS_ENGINE_TIMESETP::GraphicsEngineTimeStep = 1.0f;
namespace GraphicsEnginePost {

	void PsagGLEnginePost::ShaderVertexDefaultParams(PsagShader shader) {
		ShaderUniform.UniformVec2(shader, "RenderMove",  Vector2T<float>(0.0f, 0.0f));
		ShaderUniform.UniformVec2(shader, "RenderScale", Vector2T<float>(1.0f, 1.0f));
	}

	void PsagGLEnginePost::ShaderRenderingLight() {
		auto LightShader = LLRES_Shaders->ResourceFind(ShaderVoluLight);

		// 2D体积光处理.
		ShaderRender.RenderBindFrameBuffer(LLRES_FrameBuffers->ResourceFind(LightFrameBuffer), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ShaderRender.RenderBindShader(LightShader);
		{
			// framework preset uniform.
			ShaderUniform.UniformMatrix4x4(LightShader, "MvpMatrix", RenderingMatrixMvp);
			ShaderVertexDefaultParams(LightShader);

			Vector2T<float> LightTexPosition(
				RenderParameters.LightPosition.vector_x / RenderingResolution.vector_x,
				RenderParameters.LightPosition.vector_y / RenderingResolution.vector_y
			);
			ShaderUniform.UniformVec2(LightShader, "LightPosition", LightTexPosition);
			ShaderUniform.UniformVec3(LightShader, "LightColor",    RenderParameters.LightColor);

			ShaderUniform.UniformFloat  (LightShader, "LightIntensity",      RenderParameters.LightIntensity);
			ShaderUniform.UniformFloat  (LightShader, "LightIntensityDecay", RenderParameters.LightIntensityDecay);
			ShaderUniform.UniformInteger(LightShader, "LightSampleStep", (int32_t)RenderParameters.LightSampleStep);

			auto TextureTempScene = LLRES_Textures->ResourceFind(ProcessTextures);
			ShaderRender.RenderBindTexture(TextureTempScene);
			// bind texture context => sampler(tmu) => unbind.
			ShaderUniform.UniformInteger(LightShader, "PostTextures", TextureTempScene.TextureSamplerCount);
			// frame draw(command).
			VerStcOperFrameDraw(GetPresetRect());
		}
		ShaderRender.RenderUnbindShader();
		ShaderRender.RenderUnbindFrameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void PsagGLEnginePost::ShaderRenderingBloomHV() {
		auto FilterShader = LLRES_Shaders->ResourceFind(ShaderFilter);

		// 片元纹理过滤.
		ShaderRender.RenderBindFrameBuffer(LLRES_FrameBuffers->ResourceFind(FilterFrameBuffer), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ShaderRender.RenderBindShader(FilterShader);
		{
			// framework preset uniform.
			ShaderUniform.UniformMatrix4x4(FilterShader, "MvpMatrix", RenderingMatrixMvp);
			ShaderVertexDefaultParams(FilterShader);

			ShaderUniform.UniformVec3 (FilterShader, "PostFilterColor", RenderParameters.GameSceneFilterCOL);
			ShaderUniform.UniformFloat(FilterShader, "PostFilterAvg",   RenderParameters.GameSceneFilterAVG);
			
			auto TextureTempScene = LLRES_Textures->ResourceFind(ProcessTextures);
			ShaderRender.RenderBindTexture(TextureTempScene);
			// bind texture context => sampler(tmu) => unbind.
			ShaderUniform.UniformInteger(FilterShader, "PostTextures", TextureTempScene.TextureSamplerCount);
			// frame draw(command).
			VerStcOperFrameDraw(GetPresetRect());
		}
		ShaderRender.RenderUnbindShader();
		ShaderRender.RenderUnbindFrameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PsagShader ShaderTemp[2] = {};
		ShaderTemp[0] = LLRES_Shaders->ResourceFind(ShaderBloomH);
		ShaderTemp[1] = LLRES_Shaders->ResourceFind(ShaderBloomV);

		// 2次采样高斯模糊(泛光).
		for (size_t i = 0; i < 2; ++i) {
			ShaderRender.RenderBindFrameBuffer(LLRES_FrameBuffers->ResourceFind(BloomFrameBuffers[i]), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ShaderRender.RenderBindShader(ShaderTemp[i]);
			{
				// framework preset uniform.
				ShaderUniform.UniformMatrix4x4(ShaderTemp[i], "MvpMatrix", RenderingMatrixMvp);
				ShaderVertexDefaultParams(ShaderTemp[i]);

				ShaderUniform.UniformInteger(ShaderTemp[i], "PostBloomRadius", RenderParameters.GameSceneBloomRadius);

				auto TextureTempScene = LLRES_Textures->ResourceFind(ProcessTextures);
				ShaderRender.RenderBindTexture(TextureTempScene);
				// bind texture context => sampler(tmu) => unbind.
				ShaderUniform.UniformInteger(ShaderTemp[i], "PostTextures", TextureTempScene.TextureSamplerCount);
				// frame draw(command).
				VerStcOperFrameDraw(GetPresetRect());
			}
			ShaderRender.RenderUnbindShader();
			ShaderRender.RenderUnbindFrameBuffer();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}

	PsagGLEnginePost::PsagGLEnginePost(const Vector2T<uint32_t>& render_resolution) {
		// generate unique_id.
		PSAG_SYSGEN_TIME_KEY GenResourceID;

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;
		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,      StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragFinalPhase, StringScript);
		
		// create & storage post_shader.
		if (ShaderProcess.CreateCompileShader()) {
			ShaderProgramItem = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderProgramItem, &ShaderProcess);
		}

		// **************** volumetric_light shader ****************

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderVolumLight;
		ShaderVolumLight.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderVolumLight.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader, StringScript);
		ShaderVolumLight.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragLight, StringScript);

		if (ShaderVolumLight.CreateCompileShader()) {
			ShaderVoluLight = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderVoluLight, &ShaderVolumLight);
		}

		// **************** color_filter shader ****************

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderColorFilter;
		ShaderColorFilter.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,       StringScript);
		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,        StringScript);
		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.shaderFragColorFilter, StringScript);

		if (ShaderColorFilter.CreateCompileShader()) {
			ShaderFilter = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderFilter, &ShaderColorFilter);
		}

		// **************** bloom H&V shader ****************

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderBloomHPCS;
		ShaderBloomHPCS.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,  StringScript);
		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,   StringScript);
		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBloomH, StringScript);

		if (ShaderBloomHPCS.CreateCompileShader()) {
			// bloom_process shader, h(横向采样).	
			ShaderBloomH = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderBloomH, &ShaderBloomHPCS);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderBloomVPCS;
		ShaderBloomVPCS.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,  StringScript);
		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,   StringScript);
		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBloomV, StringScript);

		if (ShaderBloomVPCS.CreateCompileShader()) {
			// bloom_process shader, v(纵向采样).	
			ShaderBloomV = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderBloomV, &ShaderBloomVPCS);
		}

		// => mag"GraphicsEngineDataset::GLEngineStcVertexData". 

		MatrixPorj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => psag matrix.
		// non-view matrix.
		const float* glmmatptr = glm::value_ptr(MatrixPorj);
		memcpy_s(RenderingMatrixMvp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		// **************** create texture & FBO ****************
		
		PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture     TextureCreate;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboGameScene;

		TextureCreate.SetTextureParam(render_resolution.vector_x, render_resolution.vector_y, LinearFiltering);
		TextureCreate.SetTextureSamplerCount(LLRES_Samplers->AllocTmuCount());

		// create 5-layers textures. 20240617.
		for (size_t i = 0; i < 5; ++i)
			TextureCreate.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));

		if (TextureCreate.CreateTexture()) {
			ProcessTextures = GenResourceID.PsagGenTimeKey();
			LLRES_Textures->ResourceStorage(ProcessTextures, &TextureCreate);
		}

		// 游戏场景 (捕获输入)
		if (FboGameScene.CreateFrameBuffer()) {
			GameSceneFrameBuffer = GenResourceID.PsagGenTimeKey();
			FboGameScene.TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, 0);
			LLRES_FrameBuffers->ResourceStorage(GameSceneFrameBuffer, &FboGameScene);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboVoluLight;

		// 2D灯光处理 (体积光)
		if (FboVoluLight.CreateFrameBuffer()) {
			LightFrameBuffer = GenResourceID.PsagGenTimeKey();
			FboVoluLight.TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, 1);
			LLRES_FrameBuffers->ResourceStorage(LightFrameBuffer, &FboVoluLight);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboColorFilter;

		// 纹理过滤 (图元颜色提取)
		if (FboColorFilter.CreateFrameBuffer()) {
			FilterFrameBuffer = GenResourceID.PsagGenTimeKey();
			FboColorFilter.TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, 2);
			LLRES_FrameBuffers->ResourceStorage(FilterFrameBuffer, &FboColorFilter);
		}

		// 泛光 (2次采样高斯模糊)
		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboGameBloom[2] = {};
		// bloom h&v shaders texture.
		for (size_t i = 0; i < 2; ++i) {
			BloomFrameBuffers[i] = GenResourceID.PsagGenTimeKey();
			if (FboGameBloom[i].CreateFrameBuffer()) {
				// fbo_h => bind layer2, fbo_v => bind layer3.
				FboGameBloom[i].TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, (uint32_t)i + 3);
				LLRES_FrameBuffers->ResourceStorage(BloomFrameBuffers[i], &FboGameBloom[i]);
			}
		}
		RenderingResolution = Vector2T<float>((float)render_resolution.vector_x, (float)render_resolution.vector_y);
		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "graphics_engine create post_shader(system).");
	}

	PsagGLEnginePost::~PsagGLEnginePost() {
		// free graphics post resource.
		LLRES_FrameBuffers->ResourceDelete(BloomFrameBuffers[0]);
		LLRES_FrameBuffers->ResourceDelete(BloomFrameBuffers[1]);
		LLRES_FrameBuffers->ResourceDelete(FilterFrameBuffer);
		LLRES_FrameBuffers->ResourceDelete(GameSceneFrameBuffer);

		LLRES_Samplers->FreeTmuCount(LLRES_Textures->ResourceFind(ProcessTextures).TextureSamplerCount);
		LLRES_Textures->ResourceDelete(ProcessTextures);

		LLRES_Shaders->ResourceDelete(ShaderFilter);
		LLRES_Shaders->ResourceDelete(ShaderBloomH);
		LLRES_Shaders->ResourceDelete(ShaderBloomV);
		LLRES_Shaders->ResourceDelete(ShaderProgramItem);

		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "graphics_engine free post_shader(system).");
	}

	bool PsagGLEnginePost::CaptureGameScene(const function<bool()>& rendering_func) {
		bool ReturnFlagTemp = PSAG_FALSE;

		ShaderRender.RenderBindFrameBuffer(LLRES_FrameBuffers->ResourceFind(GameSceneFrameBuffer), 0);
		// opengl api color buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			// render game_scene => fbo.
			ReturnFlagTemp = rendering_func();
		}
		ShaderRender.RenderUnbindFrameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return ReturnFlagTemp;
	}

	void PsagGLEnginePost::RenderingPostModule() {
		ShaderRenderingLight();
		ShaderRenderingBloomHV();

		auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderProgramItem);
		ShaderRender.RenderBindShader(ShaderTemp);
		
		// framework preset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix", RenderingMatrixMvp);
		ShaderUniform.UniformVec2     (ShaderTemp, "RenderRes", RenderingResolution);
		ShaderVertexDefaultParams(ShaderTemp);

		ShaderUniform.UniformFloat(ShaderTemp, "PostBloomSource", RenderParameters.GameSceneBloomBlend.vector_x);
		ShaderUniform.UniformFloat(ShaderTemp, "PostBloomBlur",   RenderParameters.GameSceneBloomBlend.vector_y);

		auto TextureTempScene = LLRES_Textures->ResourceFind(ProcessTextures);
		ShaderRender.RenderBindTexture(TextureTempScene);
		// bind texture context => sampler(tmu) => unbind.
		ShaderUniform.UniformInteger(ShaderTemp, "PostTextures", TextureTempScene.TextureSamplerCount);

		// frame draw(command).
		VerStcOperFrameDraw(GetPresetRect());
		ShaderRender.RenderUnbindShader();
	}
}

namespace GraphicsEngineBackground {

	PsagGLEngineBackground::PsagGLEngineBackground(
		const Vector2T<uint32_t>& render_resolution, const vector<ImageRawData>& imgdataset
	) {
		// generate unique_id.
		PSAG_SYSGEN_TIME_KEY GenResourceID;

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;
		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,      StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBackground, StringScript);

		// create & storage background_shader.
		if (ShaderProcess.CreateCompileShader()) {
			ShaderProgramItem = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderProgramItem, &ShaderProcess);
		}
		// model(mag): "GraphicsEngineDataset::GLEngineStcVertexData". 

		glm::mat4x4 MatrixPorj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => psag matrix.
		// non-view matrix.
		const float* glmmatptr = glm::value_ptr(MatrixPorj);
		memcpy_s(RenderingMatrixMvp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		// **************** load multiple backgrounds ****************

		if (imgdataset.size() > NULL) {
			PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture BackTexture;

			BackTexture.SetTextureParam(imgdataset[0].Width, imgdataset[0].Height, LinearFiltering);
			BackTexture.SetTextureSamplerCount(LLRES_Samplers->AllocTmuCount());

			for (const auto& LayerData : imgdataset)
				BackTexture.PushCreateTexData(TextureParam(1.0f, 1.0f), DEF_IMG_CHANNEL_RGBA, LayerData.ImagePixels);

			if (BackTexture.CreateTexture()) {
				BackgroundTextures = GenResourceID.PsagGenTimeKey();
				LLRES_Textures->ResourceStorage(BackgroundTextures, &BackTexture);
			}
			TextureTopLayer = float(imgdataset.size() - 1);
		}
		else
			PushLogger(LogWarning, PSAGM_GLENGINE_BACK_LABEL, "background layers = 0.");
		PushLogger(LogInfo, PSAGM_GLENGINE_BACK_LABEL, "graphics_engine create background.");
	}

	PsagGLEngineBackground::~PsagGLEngineBackground() {
		// free graphics back resource.
		LLRES_Samplers->FreeTmuCount(LLRES_Textures->ResourceFind(BackgroundTextures).TextureSamplerCount);
		LLRES_Textures->ResourceDelete(BackgroundTextures);
		LLRES_Shaders->ResourceDelete(ShaderProgramItem);
		PushLogger(LogInfo, PSAGM_GLENGINE_BACK_LABEL, "graphics_engine free back_shader(system).");
	}

	void PsagGLEngineBackground::RenderingBackgroundModule() {
		auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderProgramItem);
		ShaderRender.RenderBindShader(ShaderTemp);
		
		// framework preset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix", RenderingMatrixMvp);
		ShaderUniform.UniformVec2(ShaderTemp,      "RenderRes", RenderingResolution);

		ShaderUniform.UniformVec2(ShaderTemp, "RenderMove",  RenderParameters.BackgroundMove);
		ShaderUniform.UniformVec2(ShaderTemp, "RenderScale", RenderParameters.BackgroundScale);

		ShaderUniform.UniformFloat(ShaderTemp, "TextureBackIndex", TextureTopLayer);
		
		ShaderUniform.UniformFloat(ShaderTemp, "BackVisibility", RenderParameters.BackgroundVisibility);
		ShaderUniform.UniformVec2(ShaderTemp,  "BackStrength",   RenderParameters.BackgroundStrength);
		ShaderUniform.UniformVec4(ShaderTemp,  "BackBlendColor", RenderParameters.BackgroundColor);

		auto TextureTemp = LLRES_Textures->ResourceFind(BackgroundTextures);
		ShaderRender.RenderBindTexture(TextureTemp);
		ShaderUniform.UniformInteger(ShaderTemp, "MultipleBackTex", TextureTemp.TextureSamplerCount);

		// frame draw(command).
		VerStcOperFrameDraw(GetPresetRect());
		ShaderRender.RenderUnbindShader();
	}
}