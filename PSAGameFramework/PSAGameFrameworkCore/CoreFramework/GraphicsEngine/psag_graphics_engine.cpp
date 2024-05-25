// psag_graphics_engine.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;
using namespace GraphicsShaderCode;

namespace GraphicsEnginePost {
	void PsagGLEnginePost::ShaderVertexDefaultParams(PsagShader shader) {
		ShaderUniform.UniformVec2(shader, "RenderMove",  Vector2T<float>(0.0f, 0.0f));
		ShaderUniform.UniformVec2(shader, "RenderScale", Vector2T<float>(1.0f, 1.0f));
	}

	void PsagGLEnginePost::BloomShaderProcessHV() {
		auto FilterShader = LLRES_Shaders->ResourceFind(ShaderFilter);

		// 片元纹理过滤.
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextBind(LLRES_FrameBuffers->ResourceFind(FilterFrameBuffer), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextBind(FilterShader);
		{
			StaticVertexFrameDraw();
			// framework preset uniform.
			ShaderUniform.UniformMatrix4x4(FilterShader, "MvpMatrix", RenderingMatrixMvp);
			ShaderVertexDefaultParams(FilterShader);

			ShaderUniform.UniformVec3 (FilterShader, "PostFilterColor", RenderParameters.GameSceneFilterCOL);
			ShaderUniform.UniformFloat(FilterShader, "PostFilterAvg",   RenderParameters.GameSceneFilterAVG);

			auto TextureTempScene = LLRES_Textures->ResourceFind(ProcessTextures);
			PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncTextureContextBind(TextureTempScene);
			// bind texture context => sampler(tmu) => unbind.
			ShaderUniform.UniformInteger(FilterShader, "PostTextures", TextureTempScene.TextureSamplerCount);
		}
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextUnbind();
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextUnbind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PsagShader ShaderTemp[2] = {};
		ShaderTemp[0] = LLRES_Shaders->ResourceFind(ShaderBloomH);
		ShaderTemp[1] = LLRES_Shaders->ResourceFind(ShaderBloomV);

		// 2次采样高斯模糊(泛光).
		for (size_t i = 0; i < 2; ++i) {
			PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextBind(LLRES_FrameBuffers->ResourceFind(BloomFrameBuffers[i]), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextBind(ShaderTemp[i]);
			{
				StaticVertexFrameDraw();
				// framework preset uniform.
				ShaderUniform.UniformMatrix4x4(ShaderTemp[i], "MvpMatrix", RenderingMatrixMvp);
				ShaderVertexDefaultParams(ShaderTemp[i]);

				ShaderUniform.UniformInteger(ShaderTemp[i], "PostBloomRadius", RenderParameters.GameSceneBloomRadius);

				auto TextureTempScene = LLRES_Textures->ResourceFind(ProcessTextures);
				PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncTextureContextBind(TextureTempScene);
				// bind texture context => sampler(tmu) => unbind.
				ShaderUniform.UniformInteger(ShaderTemp[i], "PostTextures", TextureTempScene.TextureSamplerCount);
			}
			PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextUnbind();
			PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextUnbind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}

	PsagGLEnginePost::PsagGLEnginePost(const Vector2T<uint32_t>& render_resolution) {
		// generate unique_id.
		PSAG_SYSGEN_TIME_KEY GenResourceID;

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsShader ShaderProcess;
		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,      StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragFinalPhase, StringScript);
		
		// create & storage post_shader.
		if (ShaderProcess.CreateCompileShader()) {
			ShaderProgramItem = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderProgramItem, &ShaderProcess);
		}

		// **************** color_filter shader ****************

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsShader ShaderColorFilter;
		ShaderColorFilter.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,       StringScript);
		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,        StringScript);
		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.shaderFragColorFilter, StringScript);

		if (ShaderColorFilter.CreateCompileShader()) {
			ShaderFilter = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderFilter, &ShaderColorFilter);
		}

		// **************** bloom H&V shader ****************

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsShader ShaderBloomHPCS;
		ShaderBloomHPCS.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,  StringScript);
		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,   StringScript);
		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBloomH, StringScript);

		if (ShaderBloomHPCS.CreateCompileShader()) {
			// bloom_process shader, h(横向采样).	
			ShaderBloomH = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(ShaderBloomH, &ShaderBloomHPCS);
		}

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsShader ShaderBloomVPCS;
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

		glm::mat4 ProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => imfx matrix.
		const float* glmmatptr = glm::value_ptr(ProjectionMatrix);
		memcpy_s(RenderingMatrixMvp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		// **************** create texture & FBO ****************
		
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsTexture     TextureCreate;
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFrameBuffer FboGameScene;

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

		// 游戏场景 (捕获输入)
		if (FboGameScene.CreateFrameBuffer()) {
			GameSceneFrameBuffer = GenResourceID.PsagGenTimeKey();
			FboGameScene.TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, 0);
			LLRES_FrameBuffers->ResourceStorage(GameSceneFrameBuffer, &FboGameScene);
		}

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFrameBuffer FboColorFilter;

		// 纹理过滤 (图元颜色提取)
		if (FboColorFilter.CreateFrameBuffer()) {
			FilterFrameBuffer = GenResourceID.PsagGenTimeKey();
			FboColorFilter.TextureLayerBindFBO(LLRES_Textures->ResourceFind(ProcessTextures).Texture, 1);
			LLRES_FrameBuffers->ResourceStorage(FilterFrameBuffer, &FboColorFilter);
		}

		// 泛光 (2次采样高斯模糊)
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFrameBuffer FboGameBloom[2] = {};
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

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextBind(LLRES_FrameBuffers->ResourceFind(GameSceneFrameBuffer), 0);
		// opengl api color buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			// render game_scene => fbo.
			ReturnFlagTemp = rendering_func();
		}
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextUnbind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return ReturnFlagTemp;
	}

	bool PsagGLEnginePost::RenderingPostModule() {
		BloomShaderProcessHV();
		auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderProgramItem);
		
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextBind(ShaderTemp);
		StaticVertexFrameDraw();

		// framework preset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix", RenderingMatrixMvp);
		ShaderUniform.UniformVec2     (ShaderTemp, "RenderRes", RenderingResolution);
		ShaderVertexDefaultParams(ShaderTemp);

		ShaderUniform.UniformFloat(ShaderTemp, "PostBloomSource", RenderParameters.GameSceneBloomBlend.vector_x);
		ShaderUniform.UniformFloat(ShaderTemp, "PostBloomBlur",   RenderParameters.GameSceneBloomBlend.vector_y);

		auto TextureTempScene = LLRES_Textures->ResourceFind(ProcessTextures);
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncTextureContextBind(TextureTempScene);
		// bind texture context => sampler(tmu) => unbind.
		ShaderUniform.UniformInteger(ShaderTemp, "PostTextures", TextureTempScene.TextureSamplerCount);
		
		// shader context end.
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextUnbind();
		return ShaderTemp != OPENGL_INVALID_HANDEL;
	}
}

namespace GraphicsEngineBackground {

	PsagGLEngineBackground::PsagGLEngineBackground(
		const Vector2T<uint32_t>& render_resolution, const vector<ImageRawData>& imgdataset
	) {
		// generate unique_id.
		PSAG_SYSGEN_TIME_KEY GenResourceID;

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsShader ShaderProcess;
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
			PsagLow::PsagSupGraphicsFunc::PsagGraphicsTexture BackTexture;

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

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextBind(ShaderTemp);
		StaticVertexFrameDraw();

		// framework preset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix",  RenderingMatrixMvp);
		ShaderUniform.UniformVec2(ShaderTemp,      "RenderRes",  RenderingResolution);
		ShaderUniform.UniformFloat(ShaderTemp,     "RenderTime", RunTimeTick);

		ShaderUniform.UniformVec2(ShaderTemp, "RenderMove",  RenderParameters.BackgroundMove);
		ShaderUniform.UniformVec2(ShaderTemp, "RenderScale", RenderParameters.BackgroundScale);

		ShaderUniform.UniformFloat(ShaderTemp, "TextureBackIndex", TextureTopLayer);
		
		ShaderUniform.UniformFloat(ShaderTemp, "BackVisibility", RenderParameters.BackgroundVisibility);
		ShaderUniform.UniformVec2(ShaderTemp,  "BackStrength",   RenderParameters.BackgroundStrength);
		ShaderUniform.UniformVec4(ShaderTemp,  "BackBlendColor", RenderParameters.BackgroundColor);

		auto TextureTemp = LLRES_Textures->ResourceFind(BackgroundTextures);
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncTextureContextBind(TextureTemp);
		ShaderUniform.UniformInteger(ShaderTemp, "MultipleBackTex", TextureTemp.TextureSamplerCount);

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextUnbind();
		RunTimeTick += PSAGM_VIR_TICKSTEP_GL;
		return ShaderTemp != OPENGL_INVALID_HANDEL;
	}
}