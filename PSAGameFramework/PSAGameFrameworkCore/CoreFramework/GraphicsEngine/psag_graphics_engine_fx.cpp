// psag_graphics_engine_fx.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;
using namespace GraphicsShaderCode;

namespace GraphicsEnginePVFX {

	PsagGLEngineFxImageView::PsagGLEngineFxImageView(const ImageRawData& image_data) {
		PsagLow::PsagSupGraphicsOper::PsagGraphicsTextureView CreateTexView;

		if (!CreateTexView.CreateTexViewData(image_data))
			PushLogger(LogError, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx image_view system: failed create tex_view.");

		TextureViewItem = CreateTexView.CreateTexture();
		PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx image_view system, create: %u x %u", 
			image_data.Width, image_data.Height
		);
		// image_view: managed by object.
	}

	PsagGLEngineFxImageView::~PsagGLEngineFxImageView() {
		// free image_view resource.
		glDeleteTextures(1, &TextureViewItem.Texture);
		PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx image_view system free.");
	}

	PsagTexture PsagGLEngineFxImageView::GetTextureView() {
		return TextureViewItem.Texture;
	};

	PsagGLEngineFxCaptureView::PsagGLEngineFxCaptureView(
		const Vector2T<uint32_t>& render_resolution, bool clear_oper
	) {
		// generate unique_id.
		PSAG_SYS_GENERATE_KEY GenResourceID;

		PsagLow::PsagSupGraphicsOper::PsagGraphicsTextureView CreateTexView;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsFrameBuffer CreateFrameBuffer;

		if (!CreateTexView.CreateTexViewEmpty(render_resolution.vector_x, render_resolution.vector_y))
			PushLogger(LogError, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx capture_view system: failed create tex_view.");

		TextureViewItem = CreateTexView.CreateTexture();

		if (clear_oper)
			BindFrameBufferFunc = [&]() { OGLAPI_OPER.RenderBindFrameBuffer(GraphicFrameBuffers->ResourceFind(FrameBufferItem), 0); };
		BindFrameBufferFunc = [&]() {
			// NCC: non clear frame_buffer.
			OGLAPI_OPER.RenderBindFrameBufferNCC(GraphicFrameBuffers->ResourceFind(FrameBufferItem), 0);
		};

		if (!CreateFrameBuffer.CreateFrameBuffer()) {
			PushLogger(LogError, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx capture_view system: failed create fbo.");
			return;
		}
		FrameBufferItem = GenResourceID.PsagGenUniqueKey();
		// create bind frame_buffer.
		PsagTextureAttribute TexAttributeTemp = {};
		TexAttributeTemp.Texture = TextureViewItem.Texture;

		CreateFrameBuffer.TextureBindFBO(TexAttributeTemp, 0);
		GraphicFrameBuffers->ResourceStorage(FrameBufferItem, &CreateFrameBuffer);

		PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx capture_view system init.");
	}

	void PsagGLEngineFxCaptureView::CaptureContextBegin() {
		// opengl api context bind.
		BindFrameBufferFunc();
	}

	void PsagGLEngineFxCaptureView::CaptureContextEnd() {
		// opengl api context unbind.
		OGLAPI_OPER.RenderUnbindFrameBuffer();
	}

	PsagGLEngineFxCaptureView::~PsagGLEngineFxCaptureView() {
		// free capture_view resource.
		GraphicFrameBuffers->ResourceDelete(FrameBufferItem);
		glDeleteTextures(1, &TextureViewItem.Texture);
		PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx capture_view system free.");
	}

	PsagTexture PsagGLEngineFxCaptureView::GetCaptureTexView() {
		return TextureViewItem.Texture;
	};

	constexpr float FxSpaceSize = 10.0f;
	PsagGLEngineFxSequence::PsagGLEngineFxSequence(const ImageRawData& image, const SequencePlayer& params) {
		PSAG_SYS_GENERATE_KEY GenResourceID;
		PlayerParams = params;

		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;
		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,      StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragFxSequence, StringScript);

		// create & storage fx_sequence_shader.
		if (ShaderProcess.CreateCompileShader()) {
			ShaderProcessFinal = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderProcessFinal, &ShaderProcess);
		}

		// porj matrix + scale.
		glm::mat4 ProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
		// convert: glm matrix => imfx matrix.
		const float* glmmatptr = glm::value_ptr(ProjectionMatrix);
		memcpy_s(RenderMatrix.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		VirTextureItem = GenResourceID.PsagGenUniqueKey();
		if (!VirTextureItemAlloc(VirTextureItem, image)) {
			PushLogger(LogError, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx sequence system: failed create vir_tex.");
			return;
		}
		// set uniform params name.
		VirTextureUniform.TexParamSampler  = "SequeVirTex";
		VirTextureUniform.TexParamLayer    = "SequeVirTexLayer";
		VirTextureUniform.TexParamCropping = "SequeVirTexCropping";
		VirTextureUniform.TexParamSize     = "SequeVirTexSize";

		PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "psag_fx sequence system init.");
	}

	PsagGLEngineFxSequence::~PsagGLEngineFxSequence() {
		// free graphics sequence resource.
		VirTextureItemFree(VirTextureItem);
		GraphicShaders->ResourceDelete(ShaderProcessFinal);
		PushLogger(LogInfo, PSAGM_GLENGINE_PVFX_LABEL, "graphics_engine free post_shader(system).");
	}

	bool PsagGLEngineFxSequence::DrawFxSequence(const Vector4T<float>& blend_color) {
		auto ShaderTemp = GraphicShaders->ResourceFind(ShaderProcessFinal);
		OGLAPI_OPER.RenderBindShader(ShaderTemp);
		
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
		PlayerTimer += PSAGM_VIR_TICKSTEP_GL * 2.0f * GraphicsEngineTimeStep;
		RenderTimer += PSAGM_VIR_TICKSTEP_GL;

		// draw virtual texture.
		VirTextureItemDraw(VirTextureItem, ShaderTemp, VirTextureUniform);
		// frame draw(command).
		VerStcOperFrameDraw(GetPresetRect());
		OGLAPI_OPER.RenderUnbindShader();

#if PSAG_DEBUG_MODE
		if (ShaderTemp == OPENGL_INVALID_HANDEL)
			return false;
#endif
		return true;
	}
}