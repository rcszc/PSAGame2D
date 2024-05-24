// psaglow_opengl. 2024_04_01. RCSZ
// PSAG Framework OpenGL LowLevel.

#ifndef _PSAGLOW_OPENGL_H
#define _PSAGLOW_OPENGL_H

#include "../framework_renderer_std.h"
#include "../framework_renderer_ext.h"

class PsagOGLsystemLogger {
protected: static RendererLogger PsagLowLog;
};

// Manager Module [Renderer "Manager" Interface] 20240101.
// Update: 2024_05_13. RCSZ
namespace PSAG_OGL_MAG {
#define PSAG_OGLMAG_LABEL "PSAG_OGL_MAG"

	// system.persets data, 2 * triangle_att dataset.
	constexpr float ShaderTemplateRect[72] = {
		-10.0f, -10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,0.0f,0.0f,
		 10.0f, -10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,
		 10.0f,  10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,0.0f,0.0f,

		-10.0f, -10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,0.0f,0.0f,
		 10.0f,  10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,0.0f,0.0f,
		-10.0f,  10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,0.0f,0.0f
	};
	constexpr size_t ShaderTemplateRectLen = 72;

	class PsagInitOGL :public PsagOGLsystemLogger, public PSAGGL_GLOBAL_INIT {
	protected:
		std::string GLEWshaderVersion = {};
	public:
		// opengl initialization.
		INIT_RETURN RendererInit(INIT_PARAMETERS init_param, const std::string& version) override;
		// framework logger function, (before init).
		void LoggerFunc(RendererLogger function) override;
	};

	class PsagShadersOGL :public PsagOGLsystemLogger, public PsagGLmanagerShader {
	private:
		std::vector<std::string> ShaderCodeVS = {}, ShaderCodeFS = {};
		GLuint ShaderVS = NULL, ShaderFS = NULL;

		ResourceFlag ReturnResFlag = DEFRES_FLAG_NORMAL;
		PsagShader ShaderProgram = NULL;

		std::string FileLoaderText(const std::string& file);
		bool CompilationStatus(const uint32_t& shader, const std::string& label);

		void LoaderVertShader(const std::vector<std::string>& vs);
		void LoaderFragShader(const std::vector<std::string>& fs);

	public:
		PsagShadersOGL() : ShaderProgram(glCreateProgram()) {}
		// loader: vertex,fragment. script: [glsl], update: 20240524.
		void ShaderLoaderPushVS(const std::string& vs, ScriptReadMode mode = StringFilepath) override;
		void ShaderLoaderPushFS(const std::string& fs, ScriptReadMode mode = StringFilepath) override;

		// multiple shaders code. [non-std] 20240522 RCSZ.
		// [弃用] 20240524 RCSZ.
		// void ShaderLoaderMulVS(const std::vector<std::string>& vs, ScriptReadMode mode = StringFilepath)
		// void ShaderLoaderMulFS(const std::vector<std::string>& vs, ScriptReadMode mode = StringFilepath)

		// compile vert,frag shaders => link program.
		bool CreateCompileShader() override;

		PsagShader _MS_GETRES(ResourceFlag& flag) override;
	};

	class PsagModelOGL :public PsagOGLsystemLogger, public PsagGLmanagerModel {
	private:
		ResourceFlag ReturnResFlag = DEFRES_FLAG_INVALID;
		PsagVertexBufferAttrib VerBufferAttrib = {};

		void VertexBufferSet(GLuint vao, GLuint vbo, size_t bytes, const float* verptr, uint32_t type);

	public:
		PsagVertexAttribute CreateVertexAttribute(uint32_t type, uint32_t begin_location = 0) override;
		PsagVertexBuffer CreateVertexBuffer() override;

		bool CreateStaticModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) override;
		bool CreateDynamicModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) override;

		PsagVertexBufferAttrib _MS_GETRES(ResourceFlag& flag) override;
	};

	class PasgRenderbufferOGL :public PsagOGLsystemLogger, public PsagGLmanagerRenderBuffer {
	private:
		ResourceFlag ReturnResFlag = DEFRES_FLAG_INVALID;
		PsagRenderBufferAttrib RenderBuffer = {};

		bool CreateBindRenderbuffer(PsagRenderBuffer& renderbuf);

	public:
		bool CreateRenderBuffer(uint32_t width, uint32_t height);
		ImageRawData ReadRenderBuffer(PsagRenderBufferAttrib buffer);

		PsagRenderBufferAttrib _MS_GETRES(ResourceFlag& flag);
	};

	class PsagFramebufferOGL :public PsagOGLsystemLogger, public PsagGLmanagerFrameBuffer {
	private:
		ResourceFlag ReturnResFlag = DEFRES_FLAG_INVALID;
		PsagFrameBuffer FrameBuffer = NULL;

		bool CreateBindFamebuffer(PsagFrameBuffer& framebuf);
		bool CheckFramebuffer(PsagFrameBuffer& framebuf, ResourceFlag& flag);

	public:
		bool CreateFrameBuffer() override;
		bool TextureBindFBO(const PsagTextureAttrib& texture, uint32_t attachment = 0) override;
		bool RenderBufferBindFBO(PsagRenderBufferAttrib buffer) override;

		// bind => texture_array: layer, ext.20240507
		bool TextureLayerBindFBO(PsagTexture texture_hd, uint32_t layer, uint32_t attachment = 0);

		PsagFrameBuffer _MS_GETRES(ResourceFlag& flag) override;
	};

	// uniform error(true): invalid shader program, atomic_boolean.
	extern std::atomic<bool> ATC_UNIFORM_ERROR_FLAG;

	class PsagUniformOGL :public PsagOGLsystemLogger, public PsagGLmanagerUniform {
	private:
		bool ProgramHandle(GLuint program, const char* label);
	public:
		void UniformMatrix3x3(PsagShader program, const char* name, const PsagMatrix3& matrix) override;
		void UniformMatrix4x4(PsagShader program, const char* name, const PsagMatrix4& matrix) override;

		void UniformInteger(PsagShader program, const char* name, const int32_t& value) override;
		void UniformFloat(PsagShader program, const char* name, const float& value) override;

		void UniformVec2(PsagShader program, const char* name, const Vector2T<float>& value) override;
		void UniformVec3(PsagShader program, const char* name, const Vector3T<float>& value) override;
		void UniformVec4(PsagShader program, const char* name, const Vector4T<float>& value) override;
	};

	class TextureSystemBase :public PsagOGLsystemLogger {
	protected:
		uint8_t* ImageFileLoader(const std::string& file, int& width, int& height, int& channels);
		bool CreateBindTexture(PsagTexture& texture);
	};

	struct __SystemCreateTex {
		RawDataStream IamgeRawData;
		TextureParam  Param;

		__SystemCreateTex() : IamgeRawData({}), Param({}) {}
		__SystemCreateTex(const RawDataStream& data, const TextureParam& param) : IamgeRawData(data), Param(param) {}
	};
	// psag texture system, texture(array).
	class PsagTextureOGL :public TextureSystemBase, public PsagGLmanagerTexture {
	private:
		std::vector<__SystemCreateTex> CreateDataIndex = {};
		bool SetTextureAttrFlag = false;

		PsagTextureAttrib TextureAttrCreate = {};
		ResourceFlag ReturnResFlag = DEFRES_FLAG_INVALID;

	public:
		bool SetTextureParam(uint32_t width, uint32_t height, TextureFilterMode mode) override;
		bool SetTextureSamplerCount(uint32_t count) override;

		bool PsuhCreateTexEmpty(const TextureParam& param) override;
		bool PushCreateTexData(const TextureParam& param, uint32_t channels, const RawDataStream& data) override;
		bool PushCreateTexLoader(const TextureParam& param, const std::string& file) override;

		bool CreateTexture() override;

		PsagTextureAttrib _MS_GETRES(ResourceFlag& flag) override;
	};

	// psag texture view, texture2d. non "LLRES" manager.
	class PsagTextureViewOGL :public PsagOGLsystemLogger, public PsagGLmanagerTextureView {
	private:
		PsagTextureView TextureViewCreate = {};

		bool CreateBindTextureView(PsagTexture& texture);
	public:
		bool CreateTexViewEmpty(uint32_t width, uint32_t height, TextureFilterMode mode = LinearFiltering);
		bool CreateTexViewData(const ImageRawData& image_data, TextureFilterMode mode = LinearFiltering);

		PsagTextureView CreateTexture();
	};
}

// not_std: imfx_renderer interface.
namespace RenderingSupport {
	// 2023_12_31 [OpenGL渲染支持函数].
	void RenderBindShader(const PsagShader& program);
	void RenderUnbindShader();

	void RenderBindTexture(const PsagTextureAttrib& texture);
	void RenderUnbindTexture();

	void RnenderBindFrameBuffer(const PsagFrameBuffer& framebuffer, uint32_t attachment = 0);
	void RnenderUnbindFrameBuffer();

	void DrawVertexGroup(const PsagVertexBufferAttrib& model);
	void DrawVertexGroupExt(const PsagVertexBufferAttrib& model, size_t length_vertex, size_t offset_vertex);
	// 配合'LLRES'VBO持久映射.
	void UploadVertexGroup(PsagVertexBufferAttrib* model, float* verptr, size_t bytes);
}
// not_std: upload gpu texture_data.
namespace UploadTextureGPU {
	void UploadTextureLayerRGBA(
		const PsagTexture& texture, uint32_t layer, const Vector2T<uint32_t>& size, uint8_t* dataptr
	);
}

// not_std: framework_renderer interface.
class SysPsagInformationOGL :public PsagOGLsystemLogger {
public:
	bool LoggerPrintInformation = false;

	// @return x:vao_max, y:vbo_max
	Vector2T<int32_t> InfoGetVertexUnitsMax();
	// @return device tmu_max
	int32_t InfoGetTextureUnitsMax();
	// @return device gpu memory_capacity [mib]
	float InfoGetGPUmemoryCapacity();
	// @return device gpu memory_usage [mib]
	float InfoGetGPUmemoryUsage();
	// logger print device info
	void InfoPrintPlatformParameters();
	// @return x:vert_ufh_max, y:frag_ufh_max.
	Vector2T<int32_t> InfoGetShaderUniform();
};

// Resource Module [Renderer "Resource" Interface] ThreadSafe, 20240101.
// Update: 2024_04_01. RCSZ
namespace PSAG_OGL_RES {
#define PSAG_OGLRES_LABEL "PSAG_OGL_RES"

	class PsagResTexSamplerOGL :public PsagOGLsystemLogger, public PsagGLresourceTMU {
	protected:
		std::vector<bool> TmuStateFlag  = {};
		std::mutex TmuStateMutex = {};

	public:
		PsagResTexSamplerOGL(size_t tmu_size) {
			std::lock_guard<std::mutex> Lock(TmuStateMutex);
			TmuStateFlag.resize(tmu_size);
		}
		// GPU-TMU unit alloc & free, res_count.
		uint32_t AllocTmuCount() override;
		void FreeTmuCount(uint32_t count) override;
	};

	class PsagResShadersOGL :public PsagOGLsystemLogger, public PsagGLresourceShader {
	protected:
		std::unordered_map<std::string, PsagShader> ResourceShaderMap = {};
		std::mutex ResourceShaderMutex = {};

	public:
		PsagShader ResourceFind(ResUnique key) override;
		bool ResourceStorage(ResUnique key, PsagGLmanagerShader* res) override;
		bool ResourceDelete(ResUnique key) override;

		size_t ResourceSize() override {
			std::lock_guard<std::mutex> Lock(ResourceShaderMutex);
			return ResourceShaderMap.size();
		}

		~PsagResShadersOGL() override {
			uint32_t InvalidCountHD = NULL;
			// clear hashmap data(shader).
			for (const auto& item : ResourceShaderMap) {
				// 删除无效句柄无害 [OGLAPI]
				// func_pointer: __glewDeleteProgram
				glDeleteProgram(item.second);
				if (item.second == OPENGL_INVALID_HANDEL)
					++InvalidCountHD;
			}
			// print_log: free_count, invalid_count.
			PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "free resource(shader): %u items", ResourceSize());
			if (InvalidCountHD)
				PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "invalid resource(shader): %u items", InvalidCountHD);
		}
	};

	class PsagResTextureOGL :public PsagOGLsystemLogger, public PsagGLresourceTexture {
	protected:
		std::unordered_map<std::string, PsagTextureAttrib>       ResourceTextureMap    = {}; // Tex2D.
		std::unordered_map<std::string, PsagTextureAttrib> ResourceExtTextureMap = {}; // Tex3D.

		std::mutex ResourceTextureMutex    = {};
		std::mutex ResourceExtTextureMutex = {};

	public:
		PsagTextureAttrib ResourceFind(ResUnique key) override;
		bool ResourceStorage(ResUnique key, PsagGLmanagerTexture* res) override;
		bool ResourceDelete(ResUnique key) override;
		
		size_t ResourceSize() override {
			std::lock_guard<std::mutex> Lock(ResourceTextureMutex);
			return ResourceTextureMap.size();
		}

		~PsagResTextureOGL() override {
			uint32_t InvalidCountHD = NULL;
			// clear hashmap data(texture).
			for (const auto& item : ResourceTextureMap) {
				// func_pointer: 'GLAPI void GLAPIENTRY glDeleteTextures'
				glDeleteTextures(1, &item.second.Texture);
				if (item.second.Texture == OPENGL_INVALID_HANDEL)
					++InvalidCountHD;
			}
			// print_log: free_count, invalid_count.
			PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "free resource(texture): %u items", ResourceSize());
			if (InvalidCountHD)
				PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "invalid resource(texture): %u items", InvalidCountHD);
		}
	};

	class PsagResVertexBufferOGL :public PsagOGLsystemLogger, public PsagGLresourceVertexBuffer {
	protected:
		std::unordered_map<std::string, PsagVertexBufferAttrib> ResourceVertexBufferMap = {};
		std::mutex ResourceVertexBufferMutex = {};

	public:
		PsagVertexBufferAttrib ResourceFind(ResUnique key) override;
		bool ResourceStorage(ResUnique key, PsagGLmanagerModel* res) override;
		bool ResourceDelete(ResUnique key) override;

		// VBO 资源持久映射(用于动态更新,非标准接口).
		// warning: non-ptr-safe.
		PsagVertexBufferAttrib* ExtResourceMapping(ResUnique key);

		size_t ResourceSize() override {
			std::lock_guard<std::mutex> Lock(ResourceVertexBufferMutex);
			return ResourceVertexBufferMap.size();
		}

		~PsagResVertexBufferOGL() override {
			uint32_t InvalidCountHD = NULL;
			// clear hashmap data(vertex_buffer).
			for (const auto& item : ResourceVertexBufferMap) {
				// func_pointer: __glewDeleteBuffers
				glDeleteBuffers(1, &item.second.DataBuffer);
				if (item.second.DataBuffer == OPENGL_INVALID_HANDEL) 
					++InvalidCountHD;
			}
			// print_log: free_count, invalid_count.
			PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "free resource(vertex_buffer): %u items", ResourceSize());
			if (InvalidCountHD)
				PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "invalid resource(vertex_buffer): %u items", InvalidCountHD);
		}
	};

	class PsagResVertexAttribOGL :public PsagOGLsystemLogger, public PsagGLresourceVertexAttribute {
	protected:
		std::unordered_map<std::string, PsagVertexAttribute> ResourceVertexAttrMap = {};
		std::mutex ResourceVertexAttrMutex = {};

	public:
		PsagVertexAttribute ResourceFind(ResUnique key) override;
		bool ResourceStorage(ResUnique key, PsagVertexAttribute res) override;
		bool ResourceDelete(ResUnique key) override;

		size_t ResourceSize() override {
			std::lock_guard<std::mutex> Lock(ResourceVertexAttrMutex);
			return ResourceVertexAttrMap.size();
		}

		~PsagResVertexAttribOGL() override {
			uint32_t InvalidCountHD = NULL;
			// clear hashmap data(vertex_attribute).
			for (const auto& item : ResourceVertexAttrMap) {
				// func_pointer: __glewDeleteVertexArrays
				glDeleteVertexArrays(1, &item.second);
				if (item.second == OPENGL_INVALID_HANDEL)
					++InvalidCountHD;
			}
			// print_log: free_count, invalid_count.
			PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "free resource(vertex_attribute): %u items", ResourceSize());
			if (InvalidCountHD)
				PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "invalid resource(vertex_attribute): %u items", InvalidCountHD);
		}
	};

	class PsagResFrameBufferOGL :public PsagOGLsystemLogger, public PsagGLresourceFrameBuffer {
	protected:
		std::unordered_map<std::string, PsagFrameBuffer> ResourceFrameBufferMap = {};
		std::mutex ResourceFrameBufferMutex = {};

	public:
		PsagFrameBuffer ResourceFind(ResUnique key) override;
		bool ResourceStorage(ResUnique key, PsagGLmanagerFrameBuffer* res) override;
		bool ResourceDelete(ResUnique key) override;

		size_t ResourceSize() override {
			std::lock_guard<std::mutex> Lock(ResourceFrameBufferMutex);
			return ResourceFrameBufferMap.size();
		}

		~PsagResFrameBufferOGL() override {
			uint32_t InvalidCountHD = NULL;
			// clear hashmap data(frame_buffer).
			for (const auto& item : ResourceFrameBufferMap) {
				// func_pointer: __glewDeleteFramebuffers
				glDeleteFramebuffers(1, &item.second);
				if (item.second == OPENGL_INVALID_HANDEL)
					++InvalidCountHD;
			}
			// print_log: free_count, invalid_count.
			PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "free resource(frame_buffer): %u items", ResourceSize());
			if (InvalidCountHD)
				PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "invalid resource(frame_buffer): %u items", InvalidCountHD);
		}
	};

	class PsagResRenderBufferOGL :public PsagOGLsystemLogger, public PsagGLresourceRenderBuffer {
	protected:
		std::unordered_map<std::string, PsagRenderBufferAttrib> ResourceRenderBufferMap = {};
		std::mutex ResourceRenderBufferMutex = {};

	public:
		PsagRenderBufferAttrib ResourceFind(ResUnique key) override;
		bool ResourceStorage(ResUnique key, PsagGLmanagerRenderBuffer* res) override;
		bool ResourceDelete(ResUnique key) override;

		size_t ResourceSize() override {
			std::lock_guard<std::mutex> Lock(ResourceRenderBufferMutex);
			return ResourceRenderBufferMap.size();
		}

		~PsagResRenderBufferOGL() override {
			uint32_t InvalidCountHD = NULL;
			// clear hashmap data(render_buffer).
			for (const auto& item : ResourceRenderBufferMap) {
				// func_pointer: __glewDeleteRenderbuffers
				glDeleteRenderbuffers(1, &item.second.RenderBuffer);
				if (item.second.RenderBuffer == OPENGL_INVALID_HANDEL)
					++InvalidCountHD;
			}
			// print_log: free_count, invalid_count.
			PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "free resource(render_buffer): %u items", ResourceSize());
			if (InvalidCountHD)
				PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "invalid resource(render_buffer): %u items", InvalidCountHD);
		}
	};
}

// ImageLoader Module [Renderer "IMG" Interface].
// Update: 2024_04_01. RCSZ
namespace PSAG_OGL_IMG {
#define PSAG_OGLIMG_LABEL "PSAG_OGL_IMG"

	class PsagIOImageFileSTB :public PsagOGLsystemLogger, public PsagIOmanagerImage {
	public:
		bool WriteImageFile(const ImageRawData& rawdata, const std::string& file, SaveImageMode mode, float quality = 1.0f) override;
		ImageRawData ReadImageFile(std::string file) override;
	};

	class PsagIOImageRawDataSTB :public PsagOGLsystemLogger, public PsagIOmanagerImageRawData {
	public:
		RawDataStream EncodeImageRawData(const ImageRawData& rawdata, SaveImageMode mode, float quality) override;
		ImageRawData DecodeImageRawData(const RawDataStream& rawdata) override;
	};
}

#endif