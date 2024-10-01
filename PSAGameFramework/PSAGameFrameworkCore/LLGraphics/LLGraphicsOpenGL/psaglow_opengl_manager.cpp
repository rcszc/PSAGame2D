// psaglow_opengl_manager.
#include "psaglow_opengl.hpp"

RendererLogger PsagOGLsystemLogger::PsagLowLog = nullptr;

// PSA-V0.1.2 GL-VERT 标准.
// 位置(x,y,z), 颜色(r,g,b,a), 纹理(u,v), 法线(x,y,z).
namespace PSAG_OGL_VERATT {
	// OpenGL 顶点属性预设.
	// poscoord, color, texture, normal. [GL_VERT_01]
	void VERTEX_ATT_PRESET_1(const GLsizei& VER_LEN_BYTES, const uint32_t& BEGIN_COUNT) {
		// vertex_data.poscoord 3 * float, bias = 0, Location = 0.
		// vertex_block.begin = 0, vertex_block.end = 2.
		glVertexAttribPointer(BEGIN_COUNT + 0, FS_VERTEX_ELEMENT, GL_FLOAT, GL_FALSE, VER_LEN_BYTES,
			(void*)0); // bias_pointer.
		glEnableVertexAttribArray(BEGIN_COUNT + 0);

		// vertex_data.color 4 * float, bias = 0 + 3, Location = 1.
		// vertex_block.begin = 3, vertex_block.end = 6.
		glVertexAttribPointer(BEGIN_COUNT + 1, FS_VERTEX_COLOR, GL_FLOAT, GL_FALSE, VER_LEN_BYTES,
			(void*)(3 * sizeof(float))); // bias_pointer.
		glEnableVertexAttribArray(BEGIN_COUNT + 1);

		// vertex_data.texture 2 * float, bias = 0 + 3 + 4, Location = 2.
		// vertex_block.begin = 7, vertex_block.end = 9.
		glVertexAttribPointer(BEGIN_COUNT + 2, FS_VERTEX_UVCOORD, GL_FLOAT, GL_FALSE, VER_LEN_BYTES,
			(void*)(7 * sizeof(float))); // bias_pointer.
		glEnableVertexAttribArray(BEGIN_COUNT + 2);

		// vertex_data.normal 3 * float, bias = 0 + 3 + 4 + 2, Location = 3.
		// vertex_block.begin = 9, vertex_block.end = 11.
		glVertexAttribPointer(BEGIN_COUNT + 3, FS_VERTEX_NORMAL, GL_FLOAT, GL_FALSE, VER_LEN_BYTES,
			(void*)(9 * sizeof(float))); // bias_pointer.
		glEnableVertexAttribArray(BEGIN_COUNT + 3);
	}

	// poscoord, texture. [GL_VERT_02]
	void VERTEX_ATT_PRESET_2(const GLsizei& VER_LEN_BYTES, const uint32_t& BEGIN_COUNT) {
		// vertex_data.poscoord 3 * float, bias = 0, Location = 0.
		// vertex_block.begin = 0, vertex_block.end = 2.
		glVertexAttribPointer(BEGIN_COUNT + 0, FS_VERTEX_ELEMENT, GL_FLOAT, GL_FALSE, VER_LEN_BYTES,
			(void*)0); // bias_pointer.
		glEnableVertexAttribArray(BEGIN_COUNT + 0);

		// vertex_data.texture 2 * float, bias = 0 + 3, Location = 1.
		// vertex_block.begin = 3, vertex_block.end = 5.
		glVertexAttribPointer(BEGIN_COUNT + 1, FS_VERTEX_UVCOORD, GL_FLOAT, GL_FALSE, VER_LEN_BYTES,
			(void*)(3 * sizeof(float))); // bias_pointer.
		glEnableVertexAttribArray(BEGIN_COUNT + 1);
	}
}

namespace PSAG_OGL_MAG {
	std::vector<float> ShaderTemplateRectDep(float zlayer) {
		float ShaderTemplateRect[72] = {
			-10.0f, -10.0f, zlayer, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,0.0f,0.0f,
			 10.0f, -10.0f, zlayer, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,
			 10.0f,  10.0f, zlayer, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,0.0f,0.0f,

			-10.0f, -10.0f, zlayer, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,0.0f,0.0f,
			 10.0f,  10.0f, zlayer, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,0.0f,0.0f,
			-10.0f,  10.0f, zlayer, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,0.0f,0.0f
		};
		return std::vector<float>(ShaderTemplateRect, ShaderTemplateRect + 72);
	}

	StaticStrLABEL PSAG_OGLMAG_INIT = "PSAG_OGL_INIT";
	// renderer framework init config opengl api.
	INIT_RETURN PsagInitOGL::RendererInit(INIT_PARAMETERS init_param, const std::string& version) {
		GlewShaderVersion = version;
		// GLEW API. init opengl core.
		INIT_RETURN ReturnValue = glewInit();
		PsagLowLog(LogTrace, PSAG_OGLMAG_INIT, "framework_graphics config init...");

		// opengl enable comp.
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);

		// opengl init params.
		// GL_DONT_CARE(默认) OGL.0x1100, GL_FASTEST(性能优先) OGL.0x1101, GL_NICEST(质量优先) OGL.0x1102
		if (!init_param.PROFILE_CONFIG) {
			glHint(GL_TEXTURE_COMPRESSION_HINT,        GL_DONT_CARE + (GLint)init_param.TextureCompress);
			glHint(GL_GENERATE_MIPMAP_HINT,            GL_DONT_CARE + (GLint)init_param.MipmapGenerate);
			glHint(GL_LINE_SMOOTH_HINT,                GL_DONT_CARE + (GLint)init_param.ElementsSmooth);
			glHint(GL_POLYGON_SMOOTH_HINT,             GL_DONT_CARE + (GLint)init_param.ElementsSmooth);
			glHint(GL_PERSPECTIVE_CORRECTION_HINT,     GL_DONT_CARE + (GLint)init_param.SceneViewCalculate);
			glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_DONT_CARE + (GLint)init_param.FragmentDerivative);
		}
		GLenum OpenGLErrCode = glGetError();
		if (OpenGLErrCode != GL_NO_ERROR) {
			PsagLowLog(LogError, PSAG_OGLMAG_INIT, "framework_graphics config params err_code: %u", OpenGLErrCode);
			PsagLowLog(LogError, PSAG_OGLMAG_INIT, "framework_graphics config err_msg: %s", glewGetErrorString(OpenGLErrCode));
		}
		return ReturnValue;
	}

	bool PsagInitOGL::LoggerFunction(RendererLogger function) {
		// framework low-level => graphics_logger.
		if (function) {
			PsagLowLog = function;
			PsagLowLog(LogInfo, PSAG_OGLMAG_INIT, "framework_graphics logger function_ptr: %x", (uintptr_t)function);
			PsagLowLog(LogInfo, PSAG_OGLMAG_INIT, "framework_graphics shader gl_version: %s", GlewShaderVersion.c_str());
		}
		return function != nullptr;
	}

	// **************************************** ShaderProgram ****************************************
	// OpenGL 4.6 GLSL vertex,fragment "#version 460 core", Update: 2024_09_12. RCSZ
	StaticStrLABEL PSAG_OGLMAG_SHADER = "PSAG_OGL_SHADER";

	std::string PsagShadersOGL::FileLoaderText(const std::string& file) {
		std::ifstream FileLoader(file);
		if (FileLoader.is_open()) {
			// calc file size.
			FileLoader.seekg(NULL, std::ios::end);
			size_t FileSize = (size_t)FileLoader.tellg();
			FileLoader.seekg(NULL, std::ios::beg);

			// read string data.
			std::string FileContent((std::istreambuf_iterator<char>(FileLoader)), std::istreambuf_iterator<char>());

			PsagLowLog(LogInfo, PSAG_OGLMAG_SHADER, "script_loader read: %s read_size: %u", file.c_str(), FileSize);
			return FileContent;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_SHADER, "script_loader failed open file: %s", file.c_str());
		return std::string();
	}

	bool PsagShadersOGL::CompilationStatus(const uint32_t& shader, const std::string& label) {
		// gl_func return status, error_log size.
		int32_t CompileSuccessFlag = NULL, ShaderLogBytes = NULL;

		char* ShaderErrorInfo = nullptr;

		if (label == "vertex" || label == "fragment") {
			// shader compiler status & log_length.
			glGetShaderiv(shader, GL_COMPILE_STATUS, &CompileSuccessFlag);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ShaderLogBytes);

			if (!CompileSuccessFlag) {
				// program link failed.
				ShaderErrorInfo = new char[ShaderLogBytes];
				glGetShaderInfoLog(shader, ShaderLogBytes, NULL, (GLchar*)ShaderErrorInfo);

				// print error.
				std::string ErrorInfo = "opengl_compiler: " + label;
				PsagLowLog(LogError, "shader_err_msg: ", "%s %s", ErrorInfo.c_str(), ShaderErrorInfo);

				delete[] ShaderErrorInfo;
				return (bool)CompileSuccessFlag;
			}
			PsagLowLog(LogInfo, PSAG_OGLMAG_SHADER, "%s compilation succeeded.", label.c_str());
			return (bool)CompileSuccessFlag;
		}
		// program link status & log_length.
		glGetProgramiv(shader, GL_LINK_STATUS, &CompileSuccessFlag);
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &ShaderLogBytes);

		if (!CompileSuccessFlag) {
			// shader compilation failed.
			ShaderErrorInfo = new char[ShaderLogBytes];
			glGetProgramInfoLog(shader, ShaderLogBytes, NULL, (GLchar*)ShaderErrorInfo);

			// print error.
			std::string ErrorInfo = "opengl_link: " + label;
			PsagLowLog(LogError, "program_err_msg: ", "%s %s", ErrorInfo.c_str(), ShaderErrorInfo);

			delete[] ShaderErrorInfo;
			return (bool)CompileSuccessFlag;
		}
		PsagLowLog(LogInfo, PSAG_OGLMAG_SHADER, "shader_program link succeeded.");
		return (bool)CompileSuccessFlag;
	}

	void PsagShadersOGL::LoaderVertShader(const std::vector<std::string>& vs) {
		// vertex shader.
		ShaderVert = glCreateShader(GL_VERTEX_SHADER);
		if (ShaderVert > OPENGL_INVALID_HANDEL) {
			// convert shaders str_ptr.
			std::vector<const GLchar*> ShaderSourcePtr = {};
			for (const auto& CodeItem : vs)
				ShaderSourcePtr.push_back(CodeItem.c_str());

			// read shader script_source => compile.
			glShaderSource(ShaderVert, (GLsizei)vs.size(), ShaderSourcePtr.data(), NULL);
			glCompileShader(ShaderVert);

			PsagLowLog(LogInfo, PSAG_OGLMAG_SHADER, "vertex_shader source script num: %u", vs.size());

			if (!CompilationStatus(ShaderVert, "vertex"))
				ReturnResourceFlag = DEFRES_FLAG_INVALID;

			// vertex_shader =compile=> =attach=> shader_program.
			glAttachShader(ShaderProgram, ShaderVert);
			return;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_SHADER, "failed create vertex_shader handle.");
	}

	void PsagShadersOGL::LoaderFragShader(const std::vector<std::string>& fs) {
		// fragment shader.
		ShaderFrag = glCreateShader(GL_FRAGMENT_SHADER);
		if (ShaderFrag > OPENGL_INVALID_HANDEL) {
			// convert shaders str_ptr.
			std::vector<const GLchar*> ShaderSourcePtr = {};
			for (const auto& CodeItem : fs)
				ShaderSourcePtr.push_back(CodeItem.c_str());

			// read shader script_source => compile.
			glShaderSource(ShaderFrag, (GLsizei)fs.size(), ShaderSourcePtr.data(), NULL);
			glCompileShader(ShaderFrag);

			PsagLowLog(LogInfo, PSAG_OGLMAG_SHADER, "fragment_shader source script num: %u", fs.size());

			if (!CompilationStatus(ShaderFrag, "fragment"))
				ReturnResourceFlag = DEFRES_FLAG_INVALID;

			// fragment_shader =compile=> =attach=> shader_program.
			glAttachShader(ShaderProgram, ShaderFrag);
			return;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_SHADER, "failed create fragment_shader handle.");
	}

	void PsagShadersOGL::ShaderLoaderPushVS(const std::string& vs, ScriptReadMode mode) {
		// load shader vert_script.
		if (mode == StringFilepath) {
			ShaderCodeVS.push_back(FileLoaderText(vs));
			return;
		}
		ShaderCodeVS.push_back(vs);
	}

	void PsagShadersOGL::ShaderLoaderPushFS(const std::string& fs, ScriptReadMode mode) {
		// load shader frag_script.
		if (mode == StringFilepath) {
			ShaderCodeFS.push_back(FileLoaderText(fs));
			return;
		}
		ShaderCodeFS.push_back(fs);
	}

	bool PsagShadersOGL::CreateCompileShader() {
		// compile vertex,fragment shaders.
		LoaderVertShader(ShaderCodeVS);
		LoaderFragShader(ShaderCodeFS);
		// fragment,vertex shaders =link=> program.
		glLinkProgram(ShaderProgram);
		return CompilationStatus(ShaderProgram, "program");
	}

	PsagShader PsagShadersOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResourceFlag;
		return ShaderProgram;
	}

	// **************************************** vertex,attribute ****************************************
	// OpenGL 4.6 vertex_buffer,vertex_attribute static,dynamic, Update: 2023_12_30. RCSZ
	StaticStrLABEL PSAG_OGLMAG_MODLE = "PSAG_OGL_MODULE";

	void PsagVertexSystemOGL::VertexBufferUpload(GLuint vao, GLuint vbo, size_t bytes, const float* verptr, uint32_t type) {
		// bind vao,vbo => data => unbind vao,vbo.
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		{
			switch (type) {
			case(0): { glBufferData(GL_ARRAY_BUFFER, bytes, verptr, GL_STATIC_DRAW);  break; }
			case(1): { glBufferData(GL_ARRAY_BUFFER, bytes, verptr, GL_DYNAMIC_DRAW); break; }
			default: 
				PsagLowLog(LogError, PSAG_OGLMAG_MODLE, "failed create vertex_buffer, not mode.");
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	PsagVertexAttribute PsagVertexSystemOGL::CreateVertexAttribute(uint32_t type, uint32_t begin_location) {
		uint32_t VertexAttributeHandle = NULL;

		// opengl generate vao handle.
		glGenVertexArrays(1, &VertexAttributeHandle);
		glBindVertexArray(VertexAttributeHandle);

		if (!VertexAttributeHandle)
			PsagLowLog(LogError, PSAG_OGLMAG_MODLE, "failed create vertex_attribute.");
		else {
			// "FS_VERTEX_BYTES" => "framework_define.hpp" (vertex bytes)
			switch (type) {
			case(0): { PSAG_OGL_VERATT::VERTEX_ATT_PRESET_1(FS_VERTEX_BYTES, begin_location); break;  }
			case(1): { PSAG_OGL_VERATT::VERTEX_ATT_PRESET_2(FS_VERTEX_BYTES, begin_location); break;  }
			default:
				PsagLowLog(LogError, PSAG_OGLMAG_MODLE, "failed create vertex_attribute, no-preset.");
			}
		}
		glBindVertexArray(NULL);
		return VertexAttributeHandle;
	}

	PsagVertexBuffer PsagVertexSystemOGL::CreateVertexBuffer() {
		uint32_t VertexBufferHandle = NULL;
		
		// opengl generate vbo handle.
		glGenBuffers(1, &VertexBufferHandle);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferHandle);
		
		if (!VertexBufferHandle)
			PsagLowLog(LogError, PSAG_OGLMAG_MODLE, "failed create vertex_buffer.");

		return VertexBufferHandle;
	}

	bool PsagVertexSystemOGL::CreateStaticModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) {
		if (veratt && verbuf) {
			// static_model: write float vertex_data. (mode = 0)
			VertexBufferUpload(veratt, verbuf, bytes, verptr, 0);

			// model attribute(vao,vbo) config.
			VertexBufferAttribute.DataAttribute     = veratt;
			VertexBufferAttribute.DataBuffer        = verbuf;
			VertexBufferAttribute.VertexBytes       = FS_VERTEX_BYTES;
			VertexBufferAttribute.VerticesDataBytes = bytes;

			ReturnResourceFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_MODLE, "static_vertex invalid handle(vao | vbo).");
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagVertexSystemOGL::CreateDynamicModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) {
		if (veratt && verbuf) {
			// dynamic_model: write float vertex_data. (mode = 1)
			VertexBufferUpload(veratt, verbuf, bytes, verptr, 1);

			// model attribute(vao,vbo) config.
			VertexBufferAttribute.DataAttribute     = veratt;
			VertexBufferAttribute.DataBuffer        = verbuf;
			VertexBufferAttribute.VertexBytes       = FS_VERTEX_BYTES;
			VertexBufferAttribute.VerticesDataBytes = bytes;

			ReturnResourceFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_MODLE, "dynamic_vertex invalid handle(vao | vbo).");
		return DEF_PSAGSTAT_FAILED;
	}

	PsagVertexBufferAttribute PsagVertexSystemOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResourceFlag;
		return VertexBufferAttribute;
	}

	// **************************************** RenderBuffer ****************************************
	// OpenGL 4.6 render_buffer oper(fbo), Update: 2024_01_25. RCSZ
	StaticStrLABEL PSAG_OGLMAG_RENDREBUFFER = "PSAG_OGL_RENDER";

	bool PasgRenderbufferOGL::CreateBindRenderBuffer(PsagRenderBuffer& render_buffer) {
		// opengl generate rbo object.
		if (render_buffer == OPENGL_INVALID_HANDEL) {
			// psag 24-standard: rbo warn_num: 128.
			glGenRenderbuffers(1, &render_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);

			PsagLowLog(LogInfo, PSAG_OGLMAG_RENDREBUFFER, "render_buffer object create.");
			return DEF_PSAGSTAT_SUCCESS;
		}
		// object duplicate creation.
		PsagLowLog(LogWarning, PSAG_OGLMAG_RENDREBUFFER, "render_buffer object duplicate create.");
		return DEF_PSAGSTAT_FAILED;
	}

#define RBO_PIXEL_SIZE 4
	bool PasgRenderbufferOGL::CreateConfigRenderBuffer(uint32_t width, uint32_t height, bool depth) {
		if (CreateBindRenderBuffer(RenderBufferSize.RenderBufferSize)) {
			// GL_DEPTH24_STENCIL8 - d.24bit s.8bit 32bit(4byte)
			// alloc rbo memory(set render size).
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)width, (int)height);
			if (depth) {
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderBufferSize.RenderBufferSize);
				PsagLowLog(LogInfo, PSAG_OGLMAG_RENDREBUFFER, "render_buffer config depth.");
			}
			RenderBufferSize.Width        = width;
			RenderBufferSize.Height       = height;
			RenderBufferSize.Channels     = 4;
			RenderBufferSize.TextureBytes = RenderBufferSize.Width * RenderBufferSize.Height * RBO_PIXEL_SIZE;

			// render buffer pixel memory_size(mib)
			float PixelMemory = (float)width * (float)height * RBO_PIXEL_SIZE / 1048576.0f;
			PsagLowLog(LogInfo, PSAG_OGLMAG_RENDREBUFFER, "alloc render_buffer %u x %u, size: %.3f mib.", width, height, PixelMemory);
			// unbind render_buffer.
			glBindRenderbuffer(GL_RENDERBUFFER, NULL);

			ReturnResourceFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		return DEF_PSAGSTAT_FAILED;
	}

	bool PasgRenderbufferOGL::CreateRenderBufferDepth(uint32_t width, uint32_t height) {
		return CreateConfigRenderBuffer(width, height, true);
	}

	bool PasgRenderbufferOGL::CreateRenderBuffer(uint32_t width, uint32_t height) {
		return CreateConfigRenderBuffer(width, height, false);
	}

	ImageRawData PasgRenderbufferOGL::ReadRenderBuffer(PsagRenderBufferAttribute buffer) {
		ImageRawData RenderRawData = {};

		RenderRawData.ImagePixels.resize(RBO_PIXEL_SIZE * buffer.Width * buffer.Height);

		RenderRawData.Width    = buffer.Width;
		RenderRawData.Height   = buffer.Height;
		RenderRawData.Channels = buffer.Channels;

		glBindFramebuffer(GL_FRAMEBUFFER, buffer.RenderBufferSize);
		// read render_buffer pixel.
		glReadPixels(NULL, NULL, RenderRawData.Width, RenderRawData.Height, GL_RGBA, GL_UNSIGNED_BYTE, RenderRawData.ImagePixels.data());
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);

		return RenderRawData;
	}

	PsagRenderBufferAttribute PasgRenderbufferOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResourceFlag;
		return RenderBufferSize;
	}

	// **************************************** FrameBuffer ****************************************
	// OpenGL 4.6 frame_buffer bind => texture, bind => render_buffer, Update: 2023_12_30. RCSZ
	StaticStrLABEL PSAG_OGLMAG_FRAMEBUFFER = "PSAG_OGL_FRAME";
	
	bool PsagFramebufferOGL::CreateBindFameBuffer(PsagFrameBuffer& frame_buffer) {
		// opengl generate fbo object.
		if (frame_buffer == OPENGL_INVALID_HANDEL) {
			// psag 24-standard: rbo warn_num: 128.
			glGenFramebuffers(1, &frame_buffer);
			glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

			PsagLowLog(LogInfo, PSAG_OGLMAG_FRAMEBUFFER, "frame_buffer object create.");
			return DEF_PSAGSTAT_SUCCESS;
		}
		// object duplicate creation.
		PsagLowLog(LogWarning, PSAG_OGLMAG_FRAMEBUFFER, "frame_buffer object duplicate create.");
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagFramebufferOGL::CheckFrameBuffer(PsagFrameBuffer& framebuf, ResourceFlag& flag) {
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			glDeleteRenderbuffers(1, &framebuf);

			PsagLowLog(LogError, PSAG_OGLMAG_FRAMEBUFFER, "incomplete frame_buffer, err_code: %u", glGetError());
			return DEF_PSAGSTAT_FAILED;
		}
		flag = DEFRES_FLAG_NORMAL;
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagFramebufferOGL::CreateFrameBuffer() {
		if (CreateBindFameBuffer(FrameBuffer)) return DEF_PSAGSTAT_SUCCESS;
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagFramebufferOGL::TextureBaseBind(const PsagTextureAttribute& texture, uint32_t attachment, bool dep_flag) {
		bool ResultFlag = false;

		if (attachment > DEF_GL_COLOR_ATTACHMENT_MAX) {
			PsagLowLog(LogError, PSAG_OGLMAG_FRAMEBUFFER, "frame_buffer attachment num > max.");
			return DEF_PSAGSTAT_FAILED;
		}
		// texture => framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glBindTexture(GL_TEXTURE_2D, texture.Texture);
		{
			if (texture.Texture < OPENGL_INVALID_HANDEL) {
				PsagLowLog(LogError, PSAG_OGLMAG_FRAMEBUFFER, "invalid handle texture.");
				return DEF_PSAGSTAT_FAILED;
			}
			if (!dep_flag) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, texture.Texture, NULL);
				PsagLowLog(LogInfo, PSAG_OGLMAG_FRAMEBUFFER, "frame_buffer std_texture.");
			}
			else {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture.Texture, NULL);
				glDrawBuffer(GL_NONE); // non color_buffer.
				PsagLowLog(LogInfo, PSAG_OGLMAG_FRAMEBUFFER, "frame_buffer dep_texture.");
			}
			ResultFlag = CheckFrameBuffer(FrameBuffer, ReturnResourceFlag);
		}
		// unbind fbo,texture.
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D, NULL);

		return ResultFlag;
	}

	bool PsagFramebufferOGL::TextureBindFBO(const PsagTextureAttribute& texture, uint32_t attachment) {
		return TextureBaseBind(texture, attachment, false);
	}

	bool PsagFramebufferOGL::TextureDepBindFBO(const PsagTextureAttribute& texture) {
		return TextureBaseBind(texture, NULL, true);
	}

	bool PsagFramebufferOGL::RenderBufferBindFBO(PsagRenderBufferAttribute buffer) {
		bool ResultFlag = false;
		// texture => framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, buffer.RenderBufferSize);
		{
			if (buffer.RenderBufferSize == OPENGL_INVALID_HANDEL) {
				PsagLowLog(LogError, PSAG_OGLMAG_FRAMEBUFFER, "invalid handle render_buffer.");
				return DEF_PSAGSTAT_FAILED;
			}
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer.RenderBufferSize);
			ResultFlag = CheckFrameBuffer(FrameBuffer, ReturnResourceFlag);
		}
		// unbind fbo,rbo.
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindRenderbuffer(GL_RENDERBUFFER, NULL);

		return ResultFlag;
	}

	bool PsagFramebufferOGL::TextureLayerBindFBO(PsagTexture texture_hd, uint32_t layer, uint32_t attachment) {
		bool ResultFlag = false;

		if (attachment > DEF_GL_COLOR_ATTACHMENT_MAX) {
			PsagLowLog(LogError, PSAG_OGLMAG_FRAMEBUFFER, "frame_buffer attachment num > max.");
			return DEF_PSAGSTAT_FAILED;
		}
		// texture_array => framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_hd);
		{
			if (texture_hd > OPENGL_INVALID_HANDEL)
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_hd, NULL, layer);
			else {
				PsagLowLog(LogError, PSAG_OGLMAG_FRAMEBUFFER, "invalid handle texture_array.");
				return DEF_PSAGSTAT_FAILED;
			}
			ResultFlag = CheckFrameBuffer(FrameBuffer, ReturnResourceFlag);
		}
		// unbind fbo,texture_array.
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);
		
		return ResultFlag;
	}

	PsagFrameBuffer PsagFramebufferOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResourceFlag;
		return FrameBuffer;
	}

	// **************************************** UniformBuffer ****************************************
	// Update: 2024_09_12. RCSZ
	StaticStrLABEL PSAG_OGLMAG_UNIFORMBUFFER = "PSAG_OGL_UNIFORM";

	bool PsagUniformBufferOGL::CreateBindUniformBuffer(PsagUniformBuffer* uniform_buffer) {
		// opengl generate ubo handle.
		if (*uniform_buffer == OPENGL_INVALID_HANDEL) {
			// psag 24-standard: rbo warn_num: 64.
			glGenBuffers(1, uniform_buffer);
			glBindBuffer(GL_UNIFORM_BUFFER, *uniform_buffer);

			PsagLowLog(LogInfo, PSAG_OGLMAG_UNIFORMBUFFER, "uniform_buffer.gen create.");
			return DEF_PSAGSTAT_SUCCESS;
		}
		// handle duplicate creation.
		PsagLowLog(LogWarning, PSAG_OGLMAG_UNIFORMBUFFER, "uniform_buffer.gen duplicate create.");
		return DEF_PSAGSTAT_FAILED;
	}

	void PsagUniformBufferOGL::CreateUniformInfo(size_t struct_size) {
		if (struct_size == NULL)
			PsagLowLog(LogError, PSAG_OGLMAG_UNIFORMBUFFER, "uniform_buffer struct_size = null.");
		UniformStructDataBytes = struct_size;
	}

	bool PsagUniformBufferOGL::CreateUniformBuffer(uint32_t binding) {
		if (CreateBindUniformBuffer(&UniformBuffer)) {
			// alloc memory => uniform_buffer.
			glBufferData(GL_UNIFORM_BUFFER, UniformStructDataBytes, nullptr, GL_STATIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, binding, UniformBuffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			ReturnResourceFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		return DEF_PSAGSTAT_FAILED;
	}

	PsagUniformBuffer PsagUniformBufferOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResourceFlag;
		return UniformBuffer;
	}

	// **************************************** Uniform ****************************************
	// OpenGL 4.6 GLSL uniform variable, Update: 2023_12_30. RCSZ
	std::atomic<bool> ATC_UNIFORM_ERROR_FLAG = false;

	void PsagUniformOGL::UniformMatrix3x3(PsagShader program, const char* name, const PsagMatrix3& matrix) {
		// uniform upload matrix.3x3
		GLint UniformLocation = glGetUniformLocation(program, name);
		glUniformMatrix3fv(UniformLocation, 1, false, matrix.matrix);
	}

	void PsagUniformOGL::UniformMatrix4x4(PsagShader program, const char* name, const PsagMatrix4& matrix) {
		// uniform upload matrix.4x4
		GLint UniformLocation = glGetUniformLocation(program, name);
		glUniformMatrix4fv(UniformLocation, 1, false, matrix.matrix);
	}

	void PsagUniformOGL::UniformInteger(PsagShader program, const char* name, const int32_t& value) {
		// uniform upload matrix.int32
		GLint UniformLocation = glGetUniformLocation(program, name);
		glUniform1i(UniformLocation, value);
	}

	void PsagUniformOGL::UniformFloat(PsagShader program, const char* name, const float& value) {
		// uniform upload matrix.float32
		GLint UniformLocation = glGetUniformLocation(program, name);
		glUniform1f(UniformLocation, value);
	}

	void PsagUniformOGL::UniformVec2(PsagShader program, const char* name, const Vector2T<float>& value) {
		// uniform upload matrix.vec2(float32: xy)
		GLint UniformLocation = glGetUniformLocation(program, name);
		glUniform2f(UniformLocation, value.vector_x, value.vector_y);
	}

	void PsagUniformOGL::UniformVec3(PsagShader program, const char* name, const Vector3T<float>& value) {
		// uniform upload matrix.vec3(float32: xyz)
		GLint UniformLocation = glGetUniformLocation(program, name);
		glUniform3f(UniformLocation, value.vector_x, value.vector_y, value.vector_z);
	}

	void PsagUniformOGL::UniformVec4(PsagShader program, const char* name, const Vector4T<float>& value) {
		// uniform upload matrix.vec4(float32: xyzw)
		GLint UniformLocation = glGetUniformLocation(program, name);
		glUniform4f(UniformLocation, value.vector_x, value.vector_y, value.vector_z, value.vector_w);
	}
}

namespace RenderingSupport {
	OpenGLApiContext PsagOpenGLApiRenderState::ApiThisStateContext = NullContext;

	void PsagOpenGLApiDataFlow::FrameUpdateSampler() {
#if PSAG_DEBUG_MODE
		if (std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - SampleTimer
		).count() >= 500 ) {
			DataBytesUpload.vector_y   = DataBytesUpload.vector_x * 2;
			DataBytesDownload.vector_y = DataBytesDownload.vector_x * 2;
			// clear counter state.
			DataBytesUpload.vector_x   = NULL;
			DataBytesDownload.vector_x = NULL;
		}
#endif
	}

	void PsagOpenGLApiRenderState::RenderBindShader(const PsagShader& program) {
		// rendering context enable shader_program.
		glUseProgram(program);
		ApiThisStateContext = ShaderContext;
	}

	void PsagOpenGLApiRenderState::RenderBindTexture(const PsagTextureAttribute& texture) {
		glActiveTexture(GL_TEXTURE0 + texture.TextureSamplerCount);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture.Texture);
		ApiThisStateContext = TextureContext;
	}

	void PsagOpenGLApiRenderState::RenderBindFrameBuffer(const PsagFrameBuffer& framebuffer, uint32_t attachment) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
		// clear color_buffer & depth_buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ApiThisStateContext = FrameContext;
	}

	void PsagOpenGLApiRenderState::RenderBindFrameBufferNCC(const PsagFrameBuffer& framebuffer, uint32_t attachment) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
		ApiThisStateContext = FrameContext;
	}

	void PsagOpenGLApiRenderState::DrawVertexGroup(const PsagVertexBufferAttribute& model) {
		glBindVertexArray(model.DataAttribute);
		glBindBuffer(GL_ARRAY_BUFFER, model.DataBuffer);
		glDrawArrays(GL_TRIANGLES, NULL, GLsizei(model.VerticesDataBytes / model.VertexBytes));
		glBindVertexArray(NULL);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
	}

	void PsagOpenGLApiRenderState::DrawVertexGroupSeg(
		const PsagVertexBufferAttribute& model, size_t vertex_num, size_t vertex_offset
	) {
		// 非全部绘制: "model.VertexBytes", "model.VerticesDataBytes" 成员无效.
		glBindVertexArray(model.DataAttribute);
		glBindBuffer(GL_ARRAY_BUFFER, model.DataBuffer);
		glDrawArrays(GL_TRIANGLES, (GLint)vertex_offset, (GLsizei)vertex_num);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	void PsagOpenGLApiRenderState::UploadVertexDataset(PsagVertexBufferAttribute* model, float* verptr, size_t bytes, GLenum type) {
		// update vertices dataset bytes_param.
		model->VerticesDataBytes = bytes;
		glBindBuffer(GL_ARRAY_BUFFER, model->DataBuffer);
		glBufferData(GL_ARRAY_BUFFER, bytes, verptr, type);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
#if PSAG_DEBUG_MODE
		DataOperateUpload(bytes);
#endif
	}

	void PsagOpenGLApiRenderState::UploadTextureLayer(
		const PsagTexture& texture, uint32_t layer, const Vector2T<uint32_t>& size, uint8_t* dataptr, uint32_t channels
	) {
		GLenum ColorChannelsType = GL_RGBA;
		if (channels == 3) ColorChannelsType -= 1;

		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY, NULL, 0, 0, (GLint)layer, size.vector_x, size.vector_y, 1, ColorChannelsType,
			GL_UNSIGNED_BYTE, dataptr
		);
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);
#if PSAG_DEBUG_MODE
		DataOperateUpload(size_t(size.vector_x * size.vector_y * channels));
#endif
	}

	void PsagOpenGLApiRenderState::UploadUniformData(const PsagUniformBuffer& uniform_buffer, void* dataptr, size_t bytes) {
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, bytes, dataptr);
		glBindBuffer(GL_UNIFORM_BUFFER, NULL);
#if PSAG_DEBUG_MODE
		DataOperateUpload(bytes);
#endif
	}

	std::vector<float> PsagOpenGLApiRenderState::ReadVertexDatasetFP32(PsagVertexBuffer vbo) {
		std::vector<float> ReadDataTemp = {};
		GLint GpuBufferSize = NULL;

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &GpuBufferSize);

		size_t ElementCount = (size_t)GpuBufferSize / sizeof(float);
		float* DataPtr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		ReadDataTemp.insert(ReadDataTemp.begin(), DataPtr, DataPtr + ElementCount);

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
#if PSAG_DEBUG_MODE
		DataOperateDownload(ReadDataTemp.size() * sizeof(float));
#endif
		return ReadDataTemp;
	}

	void PsagOpenGLApiRenderState::RenderUnbindShader() {
		glUseProgram(NULL);
		ApiThisStateContext = NullContext;
	}
	void PsagOpenGLApiRenderState::RenderUnbindTexture() {
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);
		ApiThisStateContext = NullContext;
	}
	void PsagOpenGLApiRenderState::RenderUnbindFrameBuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		ApiThisStateContext = NullContext;
	}

	void PsagOpenGLApiRenderState::SwitchRenderModeLINES() {
		glLineWidth(2.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void PsagOpenGLApiRenderState::SwitchRenderModeFILL() {
		glLineWidth(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

// **************************************** Information ****************************************
// OpenGL 4.6 get device info. [2023_12_30]
// Update: 2024_01_25. RCSZ
StaticStrLABEL PSAG_OGLMAG_INFORMATION = "PSAG_OGL_INFORMATION";

Vector2T<int32_t> SysPsagInformationOGL::InfoGetVertexUnitsMax() {
	Vector2T<int32_t> GraphVaoVboUnits = {};

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &GraphVaoVboUnits.vector_x);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &GraphVaoVboUnits.vector_y);

	if (LoggerPrintInformation) {
		PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device (vao)attribs_max units: %d entry", GraphVaoVboUnits.vector_x);
		PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device (vbo)buffer_max units: %d entry",  GraphVaoVboUnits.vector_y);
	}
	return GraphVaoVboUnits;
}

int32_t SysPsagInformationOGL::InfoGetTextureUnitsMax() {
	GLint MaterialMappingUnitsMax = NULL;

	// GL_MAX_TEXTURE_UNITS       - 基本纹理单元数量(老版本固定管线).
	// GL_MAX_TEXTURE_IMAGE_UNITS - 可以用于Shader的纹理单元数量.
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaterialMappingUnitsMax);

	if (LoggerPrintInformation)
		PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device [tmu]texture_max units: %d entry", MaterialMappingUnitsMax);

	return MaterialMappingUnitsMax;
}

float SysPsagInformationOGL::InfoGetGPUmemoryCapacity() {
	GLint GraphMemoryCapacity = NULL;

	// GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX - 获取图形设备(GPU)显存容量.
	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &GraphMemoryCapacity);
	float GMCmibytes = float(GraphMemoryCapacity) / 1024.0f;

	if (LoggerPrintInformation)
		PsagLowLog(LogPerfmac, PSAG_OGLMAG_INFORMATION, "device memory[capacity]: %.2f mib", GMCmibytes);

	return GMCmibytes;
}

float SysPsagInformationOGL::InfoGetGPUmemoryUsage() {
	GLint GraphMemoryUsage = NULL;

	// GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX - 获取图形设备(GPU)当前使用显存.
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &GraphMemoryUsage);
	float GMUmibytes = float(GraphMemoryUsage) / 1024.0f;

	if (LoggerPrintInformation)
		PsagLowLog(LogPerfmac, PSAG_OGLMAG_INFORMATION, "device memory[usage]: %.2f mib", GMUmibytes);

	return GMUmibytes;
}

Vector2T<int32_t> SysPsagInformationOGL::InfoGetShaderUniform() {
	Vector2T<int32_t> GraphShaderUniform = {};

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,   &GraphShaderUniform.vector_x);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &GraphShaderUniform.vector_y);

	if (LoggerPrintInformation) {
		PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device (vert)uniform_max items: %d", GraphShaderUniform.vector_x);
		PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device (frag)uniform_max items: %d", GraphShaderUniform.vector_y);
	}
	return GraphShaderUniform;
}

void SysPsagInformationOGL::InfoPrintPlatformParameters() {
	PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device gpu supplier: %s",   (const char*)glGetString(GL_VENDOR));
	PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device gpu model: %s",      (const char*)glGetString(GL_RENDERER));
	PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device version opengl: %s", (const char*)glGetString(GL_VERSION));
	PsagLowLog(LogInfo, PSAG_OGLMAG_INFORMATION, "device version glsl: %s",   (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
}