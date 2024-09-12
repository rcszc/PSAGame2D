// psag_graphics_engine.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;
using namespace GraphicsShaderCode;

namespace GraphicsEngineMatrix {
	StaticStrLABEL PSAGM_GLENGINE_MAT_LABEL = "PSAG_GL_MATRIX";

	MatrixTransParams PsagGLEngineMatrix::MatrixWorldCamera = {};
	PsagMatrix4       PsagGLEngineMatrix::MatrixDataRect    = {};
	PsagMatrix4       PsagGLEngineMatrix::MatrixDataWindow  = {};

	glm::mat4 PsagGLEngineMatrix::GetOrthoProjMatrix(float scale_size) {
		return glm::ortho(
			-SystemRenderingOrthoSpace * scale_size, SystemRenderingOrthoSpace * scale_size, -SystemRenderingOrthoSpace,
			SystemRenderingOrthoSpace, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace
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

	void PsagGLEngineMatrix::CreateMatrixUniform() {
		// generate unique_id.
		PSAG_SYS_GENERATE_KEY GenResourceID;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniformBuffer MatrixUniform;

		MatrixUniform.CreateUniformInfo(sizeof(PsagMatrix4));

		if (MatrixUniform.CreateUniformBuffer(1)) {
			UniformBuffer = GenResourceID.PsagGenUniqueKey();
			GraphicUniformBuffers->ResourceStorage(UniformBuffer, &MatrixUniform);
		}
		PushLogger(LogInfo, PSAGM_GLENGINE_MAT_LABEL, "graphics_engine create matrix ubo.");
	}
}

float __GRAPHICS_ENGINE_TIMESETP::GraphicsEngineTimeStep = 1.0f;
namespace GraphicsEnginePost {

	void PsagGLEnginePost::ShaderVertexDefaultParams(PsagShader shader) {
		ShaderUniform.UniformVec2(shader, "RenderMove",  Vector2T<float>(0.0f, 0.0f));
		ShaderUniform.UniformVec2(shader, "RenderScale", Vector2T<float>(1.0f, 1.0f));
	}

	void PsagGLEnginePost::ShaderRenderingLight() {
		auto LightShader = GraphicShaders->ResourceFind(ShaderVolumLight);

		// 2D全局光照(体积光)处理.
		ShaderRender.RenderBindFrameBuffer(GraphicFrameBuffers->ResourceFind(LightFrameBuffer), 0);
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
			ShaderUniform.UniformInteger(LightShader, "LightSampleStep",     RenderParameters.LightSampleStep);

			// COLOR_BUFFER_TEX.
			auto ColorTextureTemp = GraphicTextures->ResourceFind(ProcessTextures);
			ShaderRender.RenderBindTexture(ColorTextureTemp);
			// bind texture context => sampler(tmu) => unbind.
			ShaderUniform.UniformInteger(LightShader, "PostTextures", ColorTextureTemp.TextureSamplerCount);

			// DEPTH_BUFFER_TEX.
			// non: failed_load. [20240705]
			
			// frame draw(command).
			VerStcOperFrameDraw(RenderRect);
		}
		ShaderRender.RenderUnbindShader();
		ShaderRender.RenderUnbindFrameBuffer();
	}

	void PsagGLEnginePost::ShaderRenderingBloom() {
		auto FilterShader = GraphicShaders->ResourceFind(ShaderFilter);

		// 片元纹理(高亮)过滤处理.
		ShaderRender.RenderBindFrameBuffer(GraphicFrameBuffers->ResourceFind(FilterFrameBuffer), 0);
		ShaderRender.RenderBindShader(FilterShader);
		{
			// framework preset uniform.
			ShaderUniform.UniformMatrix4x4(FilterShader, "MvpMatrix", RenderingMatrixMvp);
			ShaderVertexDefaultParams(FilterShader);

			ShaderUniform.UniformVec3 (FilterShader, "PostFilterColor", RenderParameters.GameSceneFilterCOL);
			ShaderUniform.UniformFloat(FilterShader, "PostFilterAvg",   RenderParameters.GameSceneFilterAVG);
			
			auto TextureTempScene = GraphicTextures->ResourceFind(ProcessTextures);
			ShaderRender.RenderBindTexture(TextureTempScene);
			// bind texture context => sampler(tmu) => unbind.
			ShaderUniform.UniformInteger(FilterShader, "PostTextures", TextureTempScene.TextureSamplerCount);
			// frame draw(command).
			VerStcOperFrameDraw(RenderRect);
		}
		ShaderRender.RenderUnbindShader();
		ShaderRender.RenderUnbindFrameBuffer();

		PsagShader ShaderTemp[2] = {};
		ShaderTemp[0] = GraphicShaders->ResourceFind(ShaderBloomH);
		ShaderTemp[1] = GraphicShaders->ResourceFind(ShaderBloomV);

		// 2次(横纵)高斯模糊处理.
		for (size_t i = 0; i < 2; ++i) {
			ShaderRender.RenderBindFrameBuffer(GraphicFrameBuffers->ResourceFind(BloomFrameBuffers[i]), 0);
			ShaderRender.RenderBindShader(ShaderTemp[i]);
			{
				// framework preset uniform.
				ShaderUniform.UniformMatrix4x4(ShaderTemp[i], "MvpMatrix", RenderingMatrixMvp);
				ShaderVertexDefaultParams(ShaderTemp[i]);

				ShaderUniform.UniformInteger(ShaderTemp[i], "PostBloomRadius", RenderParameters.GameSceneBloomRadius);

				auto TextureTempScene = GraphicTextures->ResourceFind(ProcessTextures);
				ShaderRender.RenderBindTexture(TextureTempScene);
				// bind texture context => sampler(tmu) => unbind.
				ShaderUniform.UniformInteger(ShaderTemp[i], "PostTextures", TextureTempScene.TextureSamplerCount);
				// frame draw(command).
				VerStcOperFrameDraw(RenderRect);
			}
			ShaderRender.RenderUnbindShader();
			ShaderRender.RenderUnbindFrameBuffer();
		}
	}

	PsagGLEnginePost::PsagGLEnginePost(const Vector2T<uint32_t>& render_resolution) {
		// generate unique_id.
		PSAG_SYS_GENERATE_KEY GenResourceID;

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderPost;
		ShaderPost.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderPost.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,      StringScript);
		ShaderPost.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragFinalPhase, StringScript);
		
		// create & storage post_shader.
		if (ShaderPost.CreateCompileShader()) {
			ShaderPostProgram = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderPostProgram, &ShaderPost);
		}

		// **************** (volumetric)light shader ****************

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderLight;
		ShaderLight.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderLight.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader, StringScript);
		ShaderLight.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragLight, StringScript);

		if (ShaderLight.CreateCompileShader()) {
			ShaderVolumLight = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderVolumLight, &ShaderLight);
		}

		// **************** color_filter shader ****************

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderColorFilter;
		ShaderColorFilter.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,       StringScript);
		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,        StringScript);
		ShaderColorFilter.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.shaderFragColorFilter, StringScript);

		if (ShaderColorFilter.CreateCompileShader()) {
			ShaderFilter = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderFilter, &ShaderColorFilter);
		}

		// **************** bloom h&v shader ****************

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderBloomHPCS;
		ShaderBloomHPCS.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,  StringScript);
		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,   StringScript);
		ShaderBloomHPCS.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBloomH, StringScript);

		if (ShaderBloomHPCS.CreateCompileShader()) {
			// bloom_process shader, h(横向采样).	
			ShaderBloomH = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderBloomH, &ShaderBloomHPCS);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderBloomVPCS;
		ShaderBloomVPCS.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,  StringScript);
		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,   StringScript);
		ShaderBloomVPCS.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBloomV, StringScript);

		if (ShaderBloomVPCS.CreateCompileShader()) {
			// bloom_process shader, v(纵向采样).	
			ShaderBloomV = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderBloomV, &ShaderBloomVPCS);
		}

		RenderRect = GenResourceID.PsagGenUniqueKey();
		// fmt: 1.0f, lowest layer.
		VerStcDataItemAlloc(RenderRect, PSAG_OGL_MAG::ShaderTemplateRectDep(10.0f));

		MatrixPorj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => psag matrix.
		// non-view matrix.
		const float* glmmatptr = glm::value_ptr(MatrixPorj);
		memcpy_s(RenderingMatrixMvp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		// **************** create scene(fbo) depth_rbo ****************
		
		PsagLow::PsagSupGraphicsOper::PsagGraphicsRenderBuffer DepRenderBufferCreate;

		if (DepRenderBufferCreate.CreateRenderBufferDepth(render_resolution.vector_x, render_resolution.vector_y)) {
			GameSceneRenderBuffer = GenResourceID.PsagGenUniqueKey();
			GraphicRenderBuffers->ResourceStorage(GameSceneRenderBuffer, &DepRenderBufferCreate);
		}
		
		// **************** create texture & framebuffer ****************
		
		PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture     TextureCreate;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboGameScene;

		TextureCreate.SetTextureParam(render_resolution.vector_x, render_resolution.vector_y, LinearFiltering);
		TextureCreate.SetTextureSamplerCount(GraphicSamplers->AllocTexMapUnitCount());

		// create 5-layers textures. 20240617.
		for (size_t i = 0; i < 5; ++i)
			TextureCreate.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));

		if (TextureCreate.CreateTexture()) {
			ProcessTextures = GenResourceID.PsagGenUniqueKey();
			GraphicTextures->ResourceStorage(ProcessTextures, &TextureCreate);
		}

		// 游戏场景 (捕获输入)
		if (FboGameScene.CreateFrameBuffer()) {
			GameSceneFrameBuffer = GenResourceID.PsagGenUniqueKey();
			// bind scene color_buffer & render_buffer(depth).
			// depth_info get: "gl_FragCoord.z"
			FboGameScene.RenderBufferBindFBO(GraphicRenderBuffers->ResourceFind(GameSceneRenderBuffer));
			FboGameScene.TextureLayerBindFBO(GraphicTextures->ResourceFind(ProcessTextures).Texture, 0);

			GraphicFrameBuffers->ResourceStorage(GameSceneFrameBuffer, &FboGameScene);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboVolumLight;

		// 2D灯光处理 (体积光)
		if (FboVolumLight.CreateFrameBuffer()) {
			LightFrameBuffer = GenResourceID.PsagGenUniqueKey();
			FboVolumLight.TextureLayerBindFBO(GraphicTextures->ResourceFind(ProcessTextures).Texture, 1);
			GraphicFrameBuffers->ResourceStorage(LightFrameBuffer, &FboVolumLight);
		}

		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboColorFilter;

		// 纹理过滤 (图元颜色提取)
		if (FboColorFilter.CreateFrameBuffer()) {
			FilterFrameBuffer = GenResourceID.PsagGenUniqueKey();
			FboColorFilter.TextureLayerBindFBO(GraphicTextures->ResourceFind(ProcessTextures).Texture, 2);
			GraphicFrameBuffers->ResourceStorage(FilterFrameBuffer, &FboColorFilter);
		}

		// 泛光 (2次采样高斯模糊)
		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer FboGameBloom[2] = {};
		// bloom h&v shaders texture.
		for (size_t i = 0; i < 2; ++i) {
			BloomFrameBuffers[i] = GenResourceID.PsagGenUniqueKey();
			if (FboGameBloom[i].CreateFrameBuffer()) {
				// fbo_h => bind layer2, fbo_v => bind layer3.
				FboGameBloom[i].TextureLayerBindFBO(GraphicTextures->ResourceFind(ProcessTextures).Texture, (uint32_t)i + 3);
				GraphicFrameBuffers->ResourceStorage(BloomFrameBuffers[i], &FboGameBloom[i]);
			}
		}
		RenderingResolution = Vector2T<float>((float)render_resolution.vector_x, (float)render_resolution.vector_y);
		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "graphics_engine create post_shader(system).");
	}

	PsagGLEnginePost::~PsagGLEnginePost() {
		// free graphics post resource.
		GraphicRenderBuffers->ResourceDelete(GameSceneRenderBuffer);

		GraphicFrameBuffers->ResourceDelete(BloomFrameBuffers[0]);
		GraphicFrameBuffers->ResourceDelete(BloomFrameBuffers[1]);
		GraphicFrameBuffers->ResourceDelete(FilterFrameBuffer);
		GraphicFrameBuffers->ResourceDelete(GameSceneFrameBuffer);

		GraphicSamplers->FreeTexMapUnitCount(GraphicTextures->ResourceFind(ProcessTextures).TextureSamplerCount);
		GraphicTextures->ResourceDelete(ProcessTextures);

		GraphicShaders->ResourceDelete(ShaderFilter);
		GraphicShaders->ResourceDelete(ShaderBloomH);
		GraphicShaders->ResourceDelete(ShaderBloomV);
		GraphicShaders->ResourceDelete(ShaderPostProgram);

		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "graphics_engine free post_shader(system).");
	}

	bool PsagGLEnginePost::CaptureGameScene(const function<bool()>& rendering_func) {
		bool ReturnFlagTemp = PSAG_FALSE;
		// game scene frame_buffer: color & depth.
		ShaderRender.RenderBindFrameBuffer(GraphicFrameBuffers->ResourceFind(GameSceneFrameBuffer), 0);
		{
			// render game_scene => fbo.
			ReturnFlagTemp = rendering_func();
		}
		ShaderRender.RenderUnbindFrameBuffer();
		return ReturnFlagTemp;
	}

	void PsagGLEnginePost::RenderingPostModule() {
		ShaderRenderingLight();
		ShaderRenderingBloom();

		auto ShaderTemp = GraphicShaders->ResourceFind(ShaderPostProgram);
		ShaderRender.RenderBindShader(ShaderTemp);
		
		// framework preset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix", RenderingMatrixMvp);
		ShaderUniform.UniformVec2     (ShaderTemp, "RenderRes", RenderingResolution);
		ShaderVertexDefaultParams     (ShaderTemp);

		ShaderUniform.UniformFloat(ShaderTemp, "PostBloomSource", RenderParameters.GameSceneBloomBlend.vector_x);
		ShaderUniform.UniformFloat(ShaderTemp, "PostBloomBlur",   RenderParameters.GameSceneBloomBlend.vector_y);

		auto TextureTempScene = GraphicTextures->ResourceFind(ProcessTextures);
		ShaderRender.RenderBindTexture(TextureTempScene);
		// bind texture context => sampler(tmu) => unbind.
		ShaderUniform.UniformInteger(ShaderTemp, "PostTextures", TextureTempScene.TextureSamplerCount);

		// frame draw(command).
		VerStcOperFrameDraw(RenderRect);
		ShaderRender.RenderUnbindShader();
	}
}

namespace GraphicsEngineBackground {

	PsagGLEngineBackground::PsagGLEngineBackground(
		const Vector2T<uint32_t>& render_resolution, const vector<ImageRawData>& imgdataset
	) {
		// generate unique_id.
		PSAG_SYS_GENERATE_KEY GenResourceID;

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;
		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,      StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragBackground, StringScript);

		// create & storage background_shader.
		if (ShaderProcess.CreateCompileShader()) {
			ShaderPostProgram = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderPostProgram, &ShaderProcess);
		}

		BackgroundRect = GenResourceID.PsagGenUniqueKey();
		// fmt: 1.0f, lowest layer.
		VerStcDataItemAlloc(BackgroundRect, PSAG_OGL_MAG::ShaderTemplateRectDep(-10.0f));

		glm::mat4x4 MatrixPorj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => psag matrix.
		// non-view matrix.
		const float* glmmatptr = glm::value_ptr(MatrixPorj);
		memcpy_s(RenderingMatrixMvp.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		// **************** load multiple backgrounds ****************

		if (imgdataset.size() > NULL) {
			PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture BackTexture;

			BackTexture.SetTextureParam(imgdataset[0].Width, imgdataset[0].Height, LinearFiltering);
			BackTexture.SetTextureSamplerCount(GraphicSamplers->AllocTexMapUnitCount());

			for (const auto& LayerData : imgdataset)
				BackTexture.PushCreateTexData(TextureParam(1.0f, 1.0f), DEF_IMG_CHANNEL_RGBA, LayerData.ImagePixels);

			if (BackTexture.CreateTexture()) {
				BackgroundTextures = GenResourceID.PsagGenUniqueKey();
				GraphicTextures->ResourceStorage(BackgroundTextures, &BackTexture);
			}
			TextureTopLayer = float(imgdataset.size() - 1);
		}
		else
			PushLogger(LogWarning, PSAGM_GLENGINE_BACK_LABEL, "background layers = 0.");
		PushLogger(LogInfo, PSAGM_GLENGINE_BACK_LABEL, "graphics_engine create background.");
	}

	PsagGLEngineBackground::~PsagGLEngineBackground() {
		// free graphics back resource.
		GraphicSamplers->FreeTexMapUnitCount(GraphicTextures->ResourceFind(BackgroundTextures).TextureSamplerCount);
		GraphicTextures->ResourceDelete(BackgroundTextures);
		GraphicShaders->ResourceDelete(ShaderPostProgram);
		PushLogger(LogInfo, PSAGM_GLENGINE_BACK_LABEL, "graphics_engine free back_shader(system).");
	}

	void PsagGLEngineBackground::RenderingBackgroundModule() {
		auto ShaderTemp = GraphicShaders->ResourceFind(ShaderPostProgram);
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

		auto TextureTemp = GraphicTextures->ResourceFind(BackgroundTextures);
		ShaderRender.RenderBindTexture(TextureTemp);
		ShaderUniform.UniformInteger(ShaderTemp, "MultipleBackTex", TextureTemp.TextureSamplerCount);

		// frame draw(command).
		VerStcOperFrameDraw(BackgroundRect);
		ShaderRender.RenderUnbindShader();
	}
}