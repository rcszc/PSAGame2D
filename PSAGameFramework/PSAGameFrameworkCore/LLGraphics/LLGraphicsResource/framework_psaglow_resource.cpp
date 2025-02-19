// framework_psaglow_resource.
#include "framework_psaglow_resource.hpp"

// psag opengl low_level, resource.
namespace PSAGGL_LOWLEVEL {

	PSAG_OGL_RES::PsagResTexSamplerOGL*    PSAG_GRAPHICS_LLRES::GraphicSamplers         = nullptr;
	PSAG_OGL_RES::PsagResShadersOGL*       PSAG_GRAPHICS_LLRES::GraphicShaders          = nullptr;
	PSAG_OGL_RES::PsagResTextureOGL*       PSAG_GRAPHICS_LLRES::GraphicTextures         = nullptr;
	PSAG_OGL_RES::PsagResVertexBufferOGL*  PSAG_GRAPHICS_LLRES::GraphicVertexBuffers    = nullptr;
	PSAG_OGL_RES::PsagResVertexAttribOGL*  PSAG_GRAPHICS_LLRES::GraphicVertexAttributes = nullptr;
	PSAG_OGL_RES::PsagResFrameBufferOGL*   PSAG_GRAPHICS_LLRES::GraphicFrameBuffers     = nullptr;
	PSAG_OGL_RES::PsagResRenderBufferOGL*  PSAG_GRAPHICS_LLRES::GraphicRenderBuffers    = nullptr;
	PSAG_OGL_RES::PsagResUniformBufferOGL* PSAG_GRAPHICS_LLRES::GraphicUniformBuffers   = nullptr;

	RendererLogger PSAG_GRAPHICS_LLRES::LLRESLoggerFunction = {};

	size_t PSAG_GRAPHICS_LLRES::LLRES_GET_ALLSIZE() {
		size_t ResourceTotal = NULL;
		ResourceTotal += GraphicShaders->ResourceSize();
		ResourceTotal += GraphicTextures->ResourceSize();
		ResourceTotal += GraphicVertexBuffers->ResourceSize();
		ResourceTotal += GraphicVertexAttributes->ResourceSize();
		ResourceTotal += GraphicFrameBuffers->ResourceSize();
		ResourceTotal += GraphicRenderBuffers->ResourceSize();
		ResourceTotal += GraphicUniformBuffers->ResourceSize();
		// 不包含纹理采样器(TMU)资源.
		return ResourceTotal;
	}

	void PSAG_GRAPHICS_LLRES::LowLevelResourceCreate(RendererLogger LogFunc, uint32_t ResTMU) {
		// LLRES CREATE SINGLE.
		GraphicSamplers         = new PSAG_OGL_RES::PsagResTexSamplerOGL   (ResTMU);
		GraphicShaders          = new PSAG_OGL_RES::PsagResShadersOGL      ();
		GraphicTextures         = new PSAG_OGL_RES::PsagResTextureOGL      ();
		GraphicVertexBuffers    = new PSAG_OGL_RES::PsagResVertexBufferOGL ();
		GraphicVertexAttributes = new PSAG_OGL_RES::PsagResVertexAttribOGL ();
		GraphicFrameBuffers     = new PSAG_OGL_RES::PsagResFrameBufferOGL  ();
		GraphicRenderBuffers    = new PSAG_OGL_RES::PsagResRenderBufferOGL ();
		GraphicUniformBuffers   = new PSAG_OGL_RES::PsagResUniformBufferOGL();
		// LLRES LOGGER FUNCTION GET.
		LLRESLoggerFunction = LogFunc;
		LLRESLoggerFunction(LogInfo, PSAG_LLRES_LABEL, "graph static_object(graphics) create.");
	}

	bool PSAG_GRAPHICS_LLRES::LowLevelResourceFree() {
		bool ReturnFlag = false;
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicSamplers,         "TEX_SAMPLER"   );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicShaders,          "SHADER"        );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicTextures,         "TEXTURE"       );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicVertexBuffers,    "VERTEX_BUFFER" );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicVertexAttributes, "VERTEX_ATTRIB" );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicFrameBuffers,     "FRAME_BUFFER"  );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicRenderBuffers,    "RENDER_BUFFER" );
		ReturnFlag |= !LLRES_RES_FREE(LLRESLoggerFunction, GraphicUniformBuffers,   "UNIFORM_BUFFER");

		LLRESLoggerFunction(LogInfo, PSAG_LLRES_LABEL, "graph static_object(graphics) delete.");
		// llres free status_flag.
		return !ReturnFlag;
	}
}