// framework_psaglow_resource.
#include "framework_psaglow_resource.hpp"

// psag opengl low_level, resource.
namespace PSAGGL_LOWLEVEL {

	PSAG_OGL_RES::PsagResTexSamplerOGL*   PSAG_GRAPHICS_LLRES::LLRES_Samplers         = nullptr;
	PSAG_OGL_RES::PsagResShadersOGL*      PSAG_GRAPHICS_LLRES::LLRES_Shaders          = {};
	PSAG_OGL_RES::PsagResTextureOGL*      PSAG_GRAPHICS_LLRES::LLRES_Textures         = {};
	PSAG_OGL_RES::PsagResVertexBufferOGL* PSAG_GRAPHICS_LLRES::LLRES_VertexBuffers    = {};
	PSAG_OGL_RES::PsagResVertexAttribOGL* PSAG_GRAPHICS_LLRES::LLRES_VertexAttributes = {};
	PSAG_OGL_RES::PsagResFrameBufferOGL*  PSAG_GRAPHICS_LLRES::LLRES_FrameBuffers     = {};
	PSAG_OGL_RES::PsagResRenderBufferOGL* PSAG_GRAPHICS_LLRES::LLRES_RenderBuffers    = {};

	RendererLogger PSAG_GRAPHICS_LLRES::LLRESLoggerFunction = {};

	size_t PSAG_GRAPHICS_LLRES::LLRES_GET_ALLSIZE() {
		// 不包含纹理采样器(TMU)资源.
		return LLRES_Shaders->ResourceSize() + LLRES_Textures->ResourceSize() + LLRES_VertexBuffers->ResourceSize() +
			LLRES_VertexAttributes->ResourceSize() + LLRES_FrameBuffers->ResourceSize() + LLRES_RenderBuffers->ResourceSize();
	}

	void PSAG_GRAPHICS_LLRES::LowLevelResourceCreate(RendererLogger LogFunc, uint32_t ResTMU) {
		// LLRES CREATE SINGLE.
		LLRES_Samplers         = new PSAG_OGL_RES::PsagResTexSamplerOGL   (ResTMU);
		LLRES_Shaders          = new PSAG_OGL_RES::PsagResShadersOGL      ();
		LLRES_Textures         = new PSAG_OGL_RES::PsagResTextureOGL      ();
		LLRES_VertexBuffers    = new PSAG_OGL_RES::PsagResVertexBufferOGL ();
		LLRES_VertexAttributes = new PSAG_OGL_RES::PsagResVertexAttribOGL ();
		LLRES_FrameBuffers     = new PSAG_OGL_RES::PsagResFrameBufferOGL  ();
		LLRES_RenderBuffers    = new PSAG_OGL_RES::PsagResRenderBufferOGL ();
		// LLRES LOGGER FUNCTION GET.
		LLRESLoggerFunction = LogFunc;
		LLRESLoggerFunction(LogInfo, PSAG_LLRES_LABEL, "LLRES static_object create(global).");
	}

	bool PSAG_GRAPHICS_LLRES::LowLevelResourceFree() {
		bool ReturnFlag = false;
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, LLRES_Samplers,         "TEX_SAMPLER"  );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, LLRES_Shaders,          "SHADER"       );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, LLRES_Textures,         "TEXTURE"      );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, LLRES_VertexBuffers,    "VER_BUFFER"   );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, LLRES_VertexAttributes, "VER_ATTRIB"   );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, LLRES_FrameBuffers,     "FRAME_BUFFER" );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, LLRES_RenderBuffers,    "RENDER_BUFFER");
		// llres free status_flag.
		return !ReturnFlag;
	}

	namespace GRAPHICS_OPER {
		// define func_obj.
		std::function<void(const PsagShader&)>                PsagGraphicsFuncShaderContextBind     = RenderingSupport::RenderBindShader;
		std::function<void(const PsagTextureAttrib&)>         PsagGraphicsFuncTextureContextBind    = RenderingSupport::RenderBindTexture;
		std::function<void(const PsagFrameBuffer&, uint32_t)> PsagGraphicsFuncFramebufContextBind   = RenderingSupport::RnenderBindFrameBuffer;

		std::function<void()> PsagGraphicsFuncShaderContextUnbind   = RenderingSupport::RenderUnbindShader;
		std::function<void()> PsagGraphicsFuncTextureContextUnbind  = RenderingSupport::RenderUnbindTexture;
		std::function<void()> PsagGraphicsFuncFramebufContextUnbind = RenderingSupport::RnenderUnbindFrameBuffer;

		std::function<void(const PsagVertexBufferAttrib&)>                 PsagGraphicsFuncDrawModel   = RenderingSupport::DrawVertexGroup;
		std::function<void(const PsagVertexBufferAttrib&, size_t, size_t)> PsagGraphicsFuncDrawVert    = RenderingSupport::DrawVertexGroupExt;
		std::function<void(PsagVertexBufferAttrib*, float*, size_t)>       PsagGraphicsFuncUpdateModel = RenderingSupport::UploadVertexGroup;
	}
}