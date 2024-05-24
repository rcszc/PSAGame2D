// psag_graphics_engine_fx.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;
using namespace GraphicsShaderCode;

namespace GraphicsEnginePVFX {

	PsagGLEngineFxImageView::PsagGLEngineFxImageView(const ImageRawData& image_data) {
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsTextureView CreateTexView;

		if (!CreateTexView.CreateTexViewData(image_data))
			PushLogger(LogError, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx image_view system: failed create tex_view.");

		TextureViewItem = CreateTexView.CreateTexture();
		PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx image_view system, create: %u x %u", 
			image_data.Width, image_data.Height);
	}

	PsagGLEngineFxImageView::~PsagGLEngineFxImageView() {
		// free image_view resource.
		glDeleteTextures(1, &TextureViewItem.Texture);
		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "psag_fx image_view system free.");
	}

	PsagTexture PsagGLEngineFxImageView::GetTextureView() {
		return TextureViewItem.Texture;
	};

	PsagGLEngineFxCaptureView::PsagGLEngineFxCaptureView(const Vector2T<uint32_t>& render_resolution) {
		// generate unique_id.
		PSAG_SYSGEN_TIME_KEY GenResourceID;

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsTextureView CreateTexView;
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFrameBuffer CreateFrameBuffer;

		if (!CreateTexView.CreateTexViewEmpty(render_resolution.vector_x, render_resolution.vector_y))
			PushLogger(LogError, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx capture_view system: failed create tex_view.");

		TextureViewItem = CreateTexView.CreateTexture();

		FrameBufferItem = to_string(GenResourceID.PsagGenTimeKey());
		if (CreateFrameBuffer.CreateFrameBuffer()) {
			// create bind frame_buffer.
			PsagTextureAttrib TexAttribTemp = {};
			TexAttribTemp.Texture = TextureViewItem.Texture;

			CreateFrameBuffer.TextureBindFBO(TexAttribTemp, 0);
			LLRES_FrameBuffers->ResourceStorage(FrameBufferItem, &CreateFrameBuffer);
			PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx capture_view system init.");
			return;
		}
		PushLogger(LogError, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx capture_view system: failed create fbo.");
	}

	void PsagGLEngineFxCaptureView::CaptureContextBind() {
		// opengl api context bind.
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextBind(LLRES_FrameBuffers->ResourceFind(FrameBufferItem), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void PsagGLEngineFxCaptureView::CaptureContextUnBind() {
		// opengl api context unbind.
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncFramebufContextUnbind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	PsagGLEngineFxCaptureView::~PsagGLEngineFxCaptureView() {
		// free capture_view resource.
		LLRES_FrameBuffers->ResourceDelete(FrameBufferItem);
		glDeleteTextures(1, &TextureViewItem.Texture);
		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "psag_fx capture_view system free.");
	}

	PsagTexture PsagGLEngineFxCaptureView::GetCaptureTexView() {
		return TextureViewItem.Texture;
	};

	constexpr float FxSpaceSize = 10.0f;
	PsagGLEngineFxSequence::PsagGLEngineFxSequence(const ImageRawData& image, const SequencePlayer& params) {
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		PlayerParams = params;

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsShader ShaderProcess;

		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate,    StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragFxSequence, StringScript);

		if (ShaderProcess.CreateCompileShader()) {
			// sequence process shader.	
			ShaderProgramItem = to_string(GenResourceID.PsagGenTimeKey());
			LLRES_Shaders->ResourceStorage(ShaderProgramItem, &ShaderProcess);
		}

		// model(mag): "GraphicsEngineDataset::GLEngineStcVertexData". 

		// porj matrix + scale.
		glm::mat4 ProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => imfx matrix.
		const float* glmmatptr = glm::value_ptr(ProjectionMatrix);
		memcpy_s(RenderMatrix.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		VirTextureItem = to_string(GenResourceID.PsagGenTimeKey());
		if (!VirTextureItemAlloc(VirTextureItem, image)) {
			PushLogger(LogError, PSAGM_GLENGINE_POST_LABEL, "psag_fx sequence system: failed create vir_tex.");
			return;
		}
		// set uniform params name.
		VirTextureUniform.TexParamSampler  = "ParticleVirTex";
		VirTextureUniform.TexParamLayer    = "ParticleVirTexLayer";
		VirTextureUniform.TexParamCropping = "ParticleVirTexCropping";
		VirTextureUniform.TexParamSize     = "ParticleVirTexSize";

		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "psag_fx sequence system init.");
	}

	PsagGLEngineFxSequence::~PsagGLEngineFxSequence() {
		// free graphics sequence resource.
		VirTextureItemFree(VirTextureItem);
		LLRES_Shaders->ResourceDelete(ShaderProgramItem);
		PushLogger(LogInfo, PSAGM_GLENGINE_POST_LABEL, "graphics_engine free post_shader(system).");
	}

	bool PsagGLEngineFxSequence::DrawFxSequence(const Vector4T<float>& blend_color) {
		auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderProgramItem);

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextBind(ShaderTemp);
		StaticVertexFrameDraw();

		// system parset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix",  RenderMatrix);
		ShaderUniform.UniformFloat    (ShaderTemp, "RenderTime", RenderTimer);

		ShaderUniform.UniformVec2(ShaderTemp, "RenderMove",  Vector2T<float>(0.0f, 0.0f));
		ShaderUniform.UniformVec2(ShaderTemp, "RenderScale", Vector2T<float>(1.0f, 1.0f));
		// player sequence frame.
		ShaderUniform.UniformVec2(ShaderTemp, "RenderUvSize",
			Vector2T<float>(1.0f / PlayerParams.UaxisFrameNumber, 1.0f / PlayerParams.VaxisFrameNumber)
		);
		ShaderUniform.UniformVec2(ShaderTemp, "RenderUvOffset", PlayerPosition);
		ShaderUniform.UniformVec4(ShaderTemp, "RenderColorBlend", blend_color);

		// player size & offset uv.
		if (PlayerTimer * PlayerParams.PlayerSpeedScale >= 1.0f) {
			if (PlayerPosition.vector_x >= 1.0f - 1.0f / PlayerParams.UaxisFrameNumber) {
				PlayerPosition.vector_x = 0.0f;
				PlayerPosition.vector_y += 1.0f / PlayerParams.VaxisFrameNumber;
			}
			if (PlayerPosition.vector_y >= 1.0f - 1.0f / PlayerParams.VaxisFrameNumber) {
				PlayerPosition = Vector2T<float>(0.0f, 0.0f);
				++PlayerCyclesCount;
			}
			PlayerTimer = 0.0f;
			PlayerPosition.vector_x += 1.0f / PlayerParams.UaxisFrameNumber;
		}
		PlayerTimer += PSAGM_VIR_TICKSTEP_GL * 2.0f;

		// draw virtual texture.
		VirTextureItemDraw(VirTextureItem, ShaderTemp, VirTextureUniform);

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextUnbind();
		RenderTimer += PSAGM_VIR_TICKSTEP_GL;
		return ShaderTemp != OPENGL_INVALID_HANDEL;
	}
}