// framework_psaglow_resource. 2024_04_01. RCSZ
// LLRES(LowLevel-Resource)

#ifndef _FRAMEWORK_IMFXGL_RESOURCE_H
#define _FRAMEWORK_IMFXGL_RESOURCE_H
// low_graphics_xxx => framework_psaglow_resource => framework.
#include "../LLGraphicsOpenGL/psaglow_opengl.hpp"

#define PSAG_LLRES_LABEL "PSAG_LLRES"
#define PSAG_LLRES_DELETE(PTR) delete PTR; PTR = nullptr;

namespace PSAGGL_LOWLEVEL {
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
			else {
				PSAG_LLRES_DELETE(POINTER)
				return DEF_PSAGSTAT_SUCCESS;
			}
		}
	protected:
		// resource data(lowlevel_resource).
		static PSAG_OGL_RES::PsagResTexSamplerOGL*   LLRES_Samplers;
		static PSAG_OGL_RES::PsagResShadersOGL*      LLRES_Shaders;
		static PSAG_OGL_RES::PsagResTextureOGL*      LLRES_Textures;
		static PSAG_OGL_RES::PsagResVertexBufferOGL* LLRES_VertexBuffers;
		static PSAG_OGL_RES::PsagResVertexAttribOGL* LLRES_VertexAttributes;
		static PSAG_OGL_RES::PsagResFrameBufferOGL*  LLRES_FrameBuffers;
		static PSAG_OGL_RES::PsagResRenderBufferOGL* LLRES_RenderBuffers;

		static size_t LLRES_GET_ALLSIZE();

		void LowLevelResourceCreate(RendererLogger LogFunc, uint32_t ResTMU = 32);
		bool LowLevelResourceFree();
	};

	namespace GRAPHICS_OPER {
		// psag graphics class. low_level => framework_middle level.
		// init, shader, texture, model, rbo, fbo, uniform.

		using PsagGraphicsSysinit      = PSAG_OGL_MAG::PsagInitOGL;
		using PsagGraphicsShader       = PSAG_OGL_MAG::PsagShadersOGL;
		using PsagGraphicsTexture      = PSAG_OGL_MAG::PsagTextureOGL;
		using PsagGraphicsModel        = PSAG_OGL_MAG::PsagModelOGL;
		using PsagGraphicsRenderBuffer = PSAG_OGL_MAG::PasgRenderbufferOGL;
		using PsagGraphicsFrameBuffer  = PSAG_OGL_MAG::PsagFramebufferOGL;
		using PsagGraphicsUniform      = PSAG_OGL_MAG::PsagUniformOGL;

		// non-texture 'LLRES' manger. 
		using PsagGraphicsTextureView = PSAG_OGL_MAG::PsagTextureViewOGL;

		using PsagGraphicsImageLoader = PSAG_OGL_IMG::PsagIOImageFileSTB;
		using PsagGraphicsImageRawDat = PSAG_OGL_IMG::PsagIOImageRawDataSTB;

		namespace PsagUploadTex = UploadTextureGPU;

		// rendering support func: non thread safe.
		// PSA-V1.0 R-FUNC 0.1, 2024_04_01. RCSZ

		extern std::function<void(const PsagShader&)>                PsagGraphicsFuncShaderContextBind;
		extern std::function<void(const PsagTextureAttrib&)>         PsagGraphicsFuncTextureContextBind;
		extern std::function<void(const PsagFrameBuffer&, uint32_t)> PsagGraphicsFuncFramebufContextBind;

		extern std::function<void()> PsagGraphicsFuncShaderContextUnbind;
		extern std::function<void()> PsagGraphicsFuncTextureContextUnbind;
		extern std::function<void()> PsagGraphicsFuncFramebufContextUnbind;

		extern std::function<void(const PsagVertexBufferAttrib&)>                 PsagGraphicsFuncDrawModel;
		extern std::function<void(const PsagVertexBufferAttrib&, size_t, size_t)> PsagGraphicsFuncDrawVert;
		extern std::function<void(PsagVertexBufferAttrib*, float*, size_t)>       PsagGraphicsFuncUpdateModel;

		// NST CLASS.
		using GraphicsINFO = SysPsagInformationOGL;
	}
}

#endif