// psag_graphics_engine.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;
using namespace GraphicsShaderCode;

float __GRAPHICS_ENGINE_TIMESETP::GraphicsEngineTimeStep = 1.0f;
namespace GraphicsEnginePost {

	void PsagGLEnginePost::ShaderVertexDefaultParams(PsagShader shader) {
		ShaderUniform.UniformVec2(shader, "RenderMove",  Vector2T<float>(0.0f, 0.0f));
		ShaderUniform.UniformVec2(shader, "RenderScale", Vector2T<float>(1.0f, 1.0f));
	}

	void PsagGLEnginePost::BloomShaderProcessHV() {
		auto FilterShader = LLRES_Shaders->ResourceFind(ShaderFilter);

		// ƬԪ��������.
		ShaderRender.RenderBindFrameBuffer(LLRES_FrameBuffers->ResourceFind(FilterFrameBuffer), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ShaderRender.RenderBindShader(FilterShader);
		{
			// framework preset uniform.
			ShaderUniform.UniformMatrix4x4(FilterShader, "MvpMatrix", BloomShaderMvp);
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

		// 2�β�����˹ģ��(����).
		for (size_t i = 0; i < 2; ++i) {
			ShaderRender.RenderBindFrameBuffer(LLRES_FrameBuffers->ResourceFind(BloomFrameBuffers[i]), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ShaderRender.RenderBindShader(ShaderTemp[i]);
			{
				// framework preset uniform.
				ShaderUniform.UniformMatrix4x4(ShaderTemp[i], "MvpMatrix", BloomShaderMvp);
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

	void PsagGLEnginePost::CameraMatrixTrans() {
		MatrixView = glm::mat4(1.0f);
		// camera position trans.
		MatrixView = glm::translate(
			MatrixView, glm::vec3(
				-RenderParameters.GameCameraTrans.CameraOffset.vector_x * 0.1f,
				RenderParameters.GameCameraTrans.CameraOffset.vector_y * 0.1f,
				0.0f
			));
		// camera rotate trans.
		MatrixView = glm::rotate(MatrixView, RenderParameters.GameCameraTrans.CameraRotate, glm::vec3(0.0f, 0.0f, 1.0f));
		// camera scale trans.
		MatrixView = glm::scale(
			MatrixView, glm::vec3(
				RenderParameters.GameCameraTrans.CameraLens.vector_x,
				RenderParameters.GameCameraTrans.CameraLens.vector_x,
				1.0f
			));
		// matrix_calc: porj * view.
		glm::mat4 CameraMatrix = MatrixPorj * glm::inverse(MatrixView);
		// convert: glm matrix => psag matrix.
		const float* glmmatptr = glm::value_ptr(CameraMatrix);
		memcpy_s(RenderingMatrixMvp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));
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
			// bloom_process shader, h(�������).	
			ShaderBloomH = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderBloomH, &ShaderBloomHPCS);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderBloomVPCS;
		ShaderBloomVPCS.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,  StringScript);
		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,   StringScript);
		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBloomV, StringScript);

		if (ShaderBloomVPCS.CreateCompileShader()) {
			// bloom_process shader, v(�������).	
			ShaderBloomV = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderBloomV, &ShaderBloomVPCS);
		}

		// => mag"GraphicsEngineDataset::GLEngineStcVertexData". 

		MatrixPorj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => psag matrix.
		// non-view matrix.
		const float* glmmatptr = glm::value_ptr(MatrixPorj);
		memcpy_s(BloomShaderMvp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		// **************** create texture & FBO ****************
		
		PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture     TextureCreate;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboGameScene;

		TextureCreate.SetTextureParam(render_resolution.vector_x, render_resolution.vector_y, LinearFiltering);
		TextureCreate.SetTextureSamplerCount(LLRES_Samplers->AllocTmuCount());

		// create 4-layers texture res.
		TextureCreate.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));
		TextureCreate.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));
		TextureCreate.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));
		TextureCreate.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));

		if (TextureCreate.CreateTexture()) {
			ProcessTextures = GenResourceID.PsagGenTimeKey();
			LLRES_Textures->ResourceStorage(ProcessTextures, &TextureCreate);
		}

		// ��Ϸ���� (��������)
		if (FboGameScene.CreateFrameBuffer()) {
			GameSceneFrameBuffer = GenResourceID.PsagGenTimeKey();
			FboGameScene.TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, 0);
			LLRES_FrameBuffers->ResourceStorage(GameSceneFrameBuffer, &FboGameScene);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboColorFilter;

		// �������� (ͼԪ��ɫ��ȡ)
		if (FboColorFilter.CreateFrameBuffer()) {
			FilterFrameBuffer = GenResourceID.PsagGenTimeKey();
			FboColorFilter.TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, 1);
			LLRES_FrameBuffers->ResourceStorage(FilterFrameBuffer, &FboColorFilter);
		}

		// ���� (2�β�����˹ģ��)
		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboGameBloom[2] = {};
		// bloom h&v shaders texture.
		for (size_t i = 0; i < 2; ++i) {
			BloomFrameBuffers[i] = GenResourceID.PsagGenTimeKey();
			if (FboGameBloom[i].CreateFrameBuffer()) {
				// fbo_h => bind layer2, fbo_v => bind layer3.
				FboGameBloom[i].TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, (uint32_t)i + 2);
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

	bool PsagGLEnginePost::RenderingPostModule() {
		// trans camera_matrix => bloom process.
		CameraMatrixTrans();
		BloomShaderProcessHV();

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

		// shader context end.
		ShaderRender.RenderUnbindShader();
		return ShaderTemp != OPENGL_INVALID_HANDEL;
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

		glm::mat4 ProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => imfx matrix.
		const float* glmmatptr = glm::value_ptr(ProjectionMatrix);
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

	bool PsagGLEngineBackground::RenderingBackgroundModule() {
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
		return ShaderTemp != OPENGL_INVALID_HANDEL;
	}
}