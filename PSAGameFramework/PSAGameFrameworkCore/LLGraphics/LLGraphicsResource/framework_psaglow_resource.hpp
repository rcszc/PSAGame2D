// framework_psaglow_resource. 2024_04_01. RCSZ
// LLRES(LowLevel-Resource)

#ifndef _FRAMEWORK_IMFXGL_RESOURCE_H
#define _FRAMEWORK_IMFXGL_RESOURCE_H
// low_graphics_xxx => framework_psaglow_resource => framework.
#include "../LLGraphicsOpenGL/psaglow_opengl.hpp"

namespace PSAGGL_LOWLEVEL {
#define PSAG_LLRES_DELETE(PTR) delete PTR; PTR = nullptr;
	StaticStrLABEL PSAG_LLRES_LABEL = "PSAG_LLRES";

	// PSAG framework lowlevel graphic dataset.
	class PSAG_GRAPHICS_LLRES {
	private:
		static RendererLogger LLRESLoggerFunction;

		template<typename T>
		bool LLRES_RES_FREE(RendererLogger LFUNC, T* POINTER, const char* LABEL) {
			if (POINTER == nullptr) {
				LFUNC(LogError, PSAG_LLRES_LABEL, "LLRES FREE_RES %s = NULLPTR.", LABEL);
				return DEF_PSAGSTAT_FAILED;
			}
			PSAG_LLRES_DELETE(POINTER)
			return DEF_PSAGSTAT_SUCCESS;
		}
	protected:
		// resource data(lowlevel_resource).
		static PSAG_OGL_RES::PsagResTexSamplerOGL*    GraphicSamplers;
		static PSAG_OGL_RES::PsagResShadersOGL*       GraphicShaders;
		static PSAG_OGL_RES::PsagResTextureOGL*       GraphicTextures;
		static PSAG_OGL_RES::PsagResVertexBufferOGL*  GraphicVertexBuffers;
		static PSAG_OGL_RES::PsagResVertexAttribOGL*  GraphicVertexAttributes;
		static PSAG_OGL_RES::PsagResFrameBufferOGL*   GraphicFrameBuffers;
		static PSAG_OGL_RES::PsagResRenderBufferOGL*  GraphicRenderBuffers;
		static PSAG_OGL_RES::PsagResUniformBufferOGL* GraphicUniformBuffers;

		static size_t LLRES_GET_ALLSIZE();

		void LowLevelResourceCreate(RendererLogger LogFunc, uint32_t ResTMU = 32);
		bool LowLevelResourceFree();
	};

	namespace GRAPHICS_OPER {
		namespace PsagRender = RenderingSupport;
		// psag graphics class. low_level => framework middle_level.
		// init, shader, texture, vertex, rbo, fbo, ubo, uniform.

		using PsagGraphicsSystemInit    = PSAG_OGL_MAG::PsagInitOGL;
		using PsagGraphicsShader        = PSAG_OGL_MAG::PsagShadersOGL;
		using PsagGraphicsTexture       = PSAG_OGL_MAG::PsagTextureOGL;
		using PsagGraphicsDepTexture    = PSAG_OGL_MAG::PsagTextureDepthOGL;
		using PsagGraphicsVertex        = PSAG_OGL_MAG::PsagVertexSystemOGL;
		using PsagGraphicsRenderBuffer  = PSAG_OGL_MAG::PasgRenderbufferOGL;
		using PsagGraphicsFrameBuffer   = PSAG_OGL_MAG::PsagFramebufferOGL;
		using PsagGraphicsUniform       = PSAG_OGL_MAG::PsagUniformOGL;
		using PsagGraphicsUniformBuffer = PSAG_OGL_MAG::PsagUniformBufferOGL;

		// non-texture 'LLRES' manger. 
		using PsagGraphicsTextureView = PSAG_OGL_MAG::PsagTextureViewOGL;

		using PsagGraphicsImageLoader = PSAG_OGL_IMG::PsagIOImageFileSTB;
		using PsagGraphicsImageRawDat = PSAG_OGL_IMG::PsagIOImageRawDataSTB;
		
		// NST CLASS.
		using GraphicsINFO = SysPsagInformationOGL;
	}
}

#endif