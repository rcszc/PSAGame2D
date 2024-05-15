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
	// renderer framework init config opengl api.
	INIT_RETURN PsagInitOGL::RendererInit(INIT_PARAMETERS init_param, const std::string& version) {
		GLEWshaderVersion = version;
		// opengl init params.
		// GL_DONT_CARE(默认) OGL.0x1100, GL_FASTEST(性能优先) OGL.0x1101, GL_NICEST(质量优先) OGL.0x1102
		glHint(GL_TEXTURE_COMPRESSION_HINT,        GL_DONT_CARE + (GLint)init_param.TextureCompress);
		glHint(GL_GENERATE_MIPMAP_HINT,            GL_DONT_CARE + (GLint)init_param.MipmapGenerate);
		glHint(GL_LINE_SMOOTH_HINT,                GL_DONT_CARE + (GLint)init_param.ElementsSmooth);
		glHint(GL_POLYGON_SMOOTH_HINT,             GL_DONT_CARE + (GLint)init_param.ElementsSmooth);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,     GL_DONT_CARE + (GLint)init_param.SceneViewCalculate);
		glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_DONT_CARE + (GLint)init_param.FragmentDerivative);
		// GLEW API. init opengl core.
		INIT_RETURN ReturnValue = glewInit();
		PsagLowLog(LogTrace, PSAG_OGLMAG_LABEL, "framework_graphics config init.");

		GLenum OpenGLErrCode = glGetError();
		if (OpenGLErrCode != GL_NO_ERROR)
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "framework_graphics config init err_code: %u", OpenGLErrCode);
		return ReturnValue;
	}

	void PsagInitOGL::LoggerFunc(RendererLogger function) {
		// framework low-level g.logger
		if (function) {
			PsagLowLog = function;
			PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "framework_graphics logger function_ptr: %x", (uintptr_t)function);
			PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "framework_graphics shader gl_version: %s", GLEWshaderVersion.c_str());
		}
	}

	// **************************************** ShaderProgram ****************************************
	// OpenGL 4.6 GLSL vertex,fragment "#version 460 core"
	// Update: 2023_12_30. RCSZ

	std::string PsagShadersOGL::FileLoaderText(const std::string& file) {
		std::ifstream FileLoader(file);

		if (FileLoader.is_open()) {
			// calc file size.
			FileLoader.seekg(NULL, std::ios::end);
			size_t FileSize = (size_t)FileLoader.tellg();
			FileLoader.seekg(NULL, std::ios::beg);

			// read string data.
			std::string FileContent((std::istreambuf_iterator<char>(FileLoader)), std::istreambuf_iterator<char>());

			PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "shader script_loader read: %s read_size: %u", file.c_str(), FileSize);
			return FileContent;
		}
		else {
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "shader script_loader failed open file: %s", file.c_str());
			return std::string("");
		}
	}

	bool PsagShadersOGL::CompilationStatus(const uint32_t& shader, const std::string& label) {
		// gl_func ret status, error_log size.
		int32_t CompileSuccess = NULL, ShaderLogBytes = NULL;

		bool CompileStatus = true;
		char* ShaderLogInfo = nullptr;

		if (label != "program") { // shader handle.
			// get compiler status & length.
			glGetShaderiv(shader, GL_COMPILE_STATUS, &CompileSuccess);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ShaderLogBytes);

			if (!CompileSuccess) {
				// program link failed.
				ShaderLogInfo = new char[ShaderLogBytes];
				glGetShaderInfoLog(shader, ShaderLogBytes, NULL, (GLchar*)ShaderLogInfo);

				// print error.
				std::string ErrorInfo = "opengl_compiler: " + label;
				PsagLowLog(LogError, "shader: ", "%s %s", ErrorInfo.c_str(), ShaderLogInfo);

				delete[] ShaderLogInfo;
			}
			else
				PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "%s shader compilation succeeded.", label);
		}
		else if (!label.empty()) { // program handle.

			glGetProgramiv(shader, GL_LINK_STATUS, &CompileSuccess);
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &ShaderLogBytes);

			if (!CompileSuccess) {
				// shader compilation failed.
				ShaderLogInfo = new char[ShaderLogBytes];
				glGetProgramInfoLog(shader, ShaderLogBytes, NULL, (GLchar*)ShaderLogInfo);

				// print error.
				std::string ErrorInfo = "opengl_link: " + label;
				PsagLowLog(LogError, "program: ", "%s %s", ErrorInfo.c_str(), ShaderLogInfo);

				delete[] ShaderLogInfo;
			}
			else
				PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "%s program link succeeded.", label);
		}
		return (bool)CompileSuccess;
	}

#define SHADER_SCRIPT_COUNT (GLsizei)1

	void PsagShadersOGL::ShaderLoaderVS(const std::string& vs, ScriptReadMode mode) {
		// vertex shader.
		ShaderVS = glCreateShader(GL_VERTEX_SHADER);
		if (ShaderVS) {
			if (mode == StringFilepath)
				ShaderSrcVS = FileLoaderText(vs);
			else
				ShaderSrcVS = vs;
			const char* VssrcstrTemp = ShaderSrcVS.c_str();
			// read shader script_source => compile.
			glShaderSource(ShaderVS, SHADER_SCRIPT_COUNT, &VssrcstrTemp, NULL);
			glCompileShader(ShaderVS);

			if (!CompilationStatus(ShaderVS, "vertex"))
				ReturnResFlag = DEFRES_FLAG_INVALID;
			glAttachShader(ShaderProgram, ShaderVS);
		}
		else
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "failed create vertex_shader handle.");
	}

	void PsagShadersOGL::ShaderLoaderFS(const std::string& fs, ScriptReadMode mode) {
		// fragment shader.
		ShaderFS = glCreateShader(GL_FRAGMENT_SHADER);
		if (ShaderFS) {
			if (mode == StringFilepath)
				ShaderSrcFS = FileLoaderText(fs);
			else
				ShaderSrcFS = fs;
			const char* FssrcstrTemp = ShaderSrcFS.c_str();
			// read shader script_source => compile.
			glShaderSource(ShaderFS, SHADER_SCRIPT_COUNT, &FssrcstrTemp, NULL);
			glCompileShader(ShaderFS);

			if (!CompilationStatus(ShaderFS, "fragment"))
				ReturnResFlag = DEFRES_FLAG_INVALID;
			glAttachShader(ShaderProgram, ShaderFS);
		}
		else
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "failed create fragment_shader handle.");
	}

	bool PsagShadersOGL::CreateCompileShader() {
		// fragment,vertex shader =link=> program.
		glLinkProgram(ShaderProgram);
		return CompilationStatus(ShaderProgram, "program");
	}

	PsagShader PsagShadersOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResFlag;
		return ShaderProgram;
	}

	// **************************************** Model ****************************************
	// OpenGL 4.6 vertex_buffer,vertex_attribute static,dynamic
	// Update: 2023_12_30. RCSZ

	void PsagModelOGL::VertexBufferSet(GLuint vao, GLuint vbo, size_t bytes, const float* verptr, uint32_t type) {
		// bind vao,vbo => data => unbind vao,vbo.
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		{
			switch (type) {
			case(0): { glBufferData(GL_ARRAY_BUFFER, bytes, verptr, GL_STATIC_DRAW);  break; }
			case(1): { glBufferData(GL_ARRAY_BUFFER, bytes, verptr, GL_DYNAMIC_DRAW); break; }
			default: 
				PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "failed create vertex_buffer, not mode.");
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	PsagVertexAttribute PsagModelOGL::CreateVertexAttribute(uint32_t type, uint32_t begin_location) {
		uint32_t VertexAttributeHandle = NULL;

		// opengl generate vao handle.
		glGenVertexArrays(1, &VertexAttributeHandle);
		glBindVertexArray(VertexAttributeHandle);

		if (!VertexAttributeHandle)
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "failed create vertex_attribute.");
		else {
			// "FS_VERTEX_BYTES" => "framework_define.hpp" (vertex bytes)
			switch (type) {
			case(0): { PSAG_OGL_VERATT::VERTEX_ATT_PRESET_1(FS_VERTEX_BYTES, begin_location); break;  }
			case(1): { PSAG_OGL_VERATT::VERTEX_ATT_PRESET_2(FS_VERTEX_BYTES, begin_location); break;  }
			default:
				PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "failed create vertex_attribute, not veratt.");
			}
		}
		glBindVertexArray(NULL);
		return VertexAttributeHandle;
	}

	PsagVertexBuffer PsagModelOGL::CreateVertexBuffer() {
		uint32_t VertexBufferHandle = NULL;
		
		// opengl generate vbo handle.
		glGenBuffers(1, &VertexBufferHandle);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferHandle);
		
		if (!VertexBufferHandle)
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "failed create vertex_buffer.");
		
		return VertexBufferHandle;
	}

	bool PsagModelOGL::CreateStaticModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) {
		if (veratt && verbuf) {
			// static_model: write float vertex_data. (mode = 0)
			VertexBufferSet(veratt, verbuf, bytes, verptr, 0);

			// model attribute(vao,vbo) config.
			VerBufferAttrib.DataAttrib        = veratt;
			VerBufferAttrib.DataBuffer        = verbuf;
			VerBufferAttrib.VertexBytes       = FS_VERTEX_BYTES;
			VerBufferAttrib.VerticesDataBytes = bytes;

			ReturnResFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		else {
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "static_model invalid handle(vao | vbo).");
			return DEF_PSAGSTAT_FAILED;
		}
	}

	bool PsagModelOGL::CreateDynamicModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) {
		if (veratt && verbuf) {
			// dynamic_model: write float vertex_data. (mode = 1)
			VertexBufferSet(veratt, verbuf, bytes, verptr, 1);

			// model attribute(vao,vbo) config.
			VerBufferAttrib.DataAttrib        = veratt;
			VerBufferAttrib.DataBuffer        = verbuf;
			VerBufferAttrib.VertexBytes       = FS_VERTEX_BYTES;
			VerBufferAttrib.VerticesDataBytes = bytes;

			ReturnResFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		else {
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "dynamic_model invalid handle(vao | vbo).");
			return DEF_PSAGSTAT_FAILED;
		}
	}

	PsagVertexBufferAttrib PsagModelOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResFlag;
		return VerBufferAttrib;
	}

	// **************************************** RenderBuffer ****************************************
	// OpenGL 4.6 render_buffer oper(fbo)
	// Update: 2024_01_25. RCSZ

	bool PasgRenderbufferOGL::CreateBindRenderbuffer(PsagRenderBuffer& renderbuf) {
		// opengl generate rbo handle.
		if (renderbuf == NULL) {
			glGenRenderbuffers(1, &renderbuf);
			glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);

			// opengl handle index > 0.
			if (renderbuf) { PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "renderbuffer.gen create.");           return DEF_PSAGSTAT_SUCCESS; }
			else           { PsagLowLog(LogWarning, PSAG_OGLMAG_LABEL, "renderbuffer.gen failed create."); return DEF_PSAGSTAT_FAILED;  }
		}
		else {
			// handle duplicate creation.
			PsagLowLog(LogWarning, PSAG_OGLMAG_LABEL, "renderbuffer.gen duplicate create.");
			return DEF_PSAGSTAT_FAILED;
		}
	}

#define RBO_PIXEL_SIZE 4
	bool PasgRenderbufferOGL::CreateRenderBuffer(uint32_t width, uint32_t height) {
		if (CreateBindRenderbuffer(RenderBuffer.RenderBuffer)) {
			// GL_DEPTH24_STENCIL8 - d.24bit s.8bit 32bit(4byte)
			// alloc rbo memory(set render size).
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)width, (int)height);

			RenderBuffer.Width = width;
			RenderBuffer.Height = height;
			RenderBuffer.Channels = 4;
			RenderBuffer.TextureBytes = RenderBuffer.Width * RenderBuffer.Height * RBO_PIXEL_SIZE;

			// render buffer pixel memory_size(mib)
			float PixelMemory = (float)width * (float)height * RBO_PIXEL_SIZE / 1048576.0f;
			PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "alloc renderbuffer_mem %u x %u, size: %.3f mib.", width, height, PixelMemory);

			// unbind rbo.
			glBindRenderbuffer(GL_RENDERBUFFER, NULL);
			return DEF_PSAGSTAT_SUCCESS;
		}
		else
			return DEF_PSAGSTAT_FAILED;
	}

	ImageRawData PasgRenderbufferOGL::ReadRenderBuffer(PsagRenderBufferAttrib buffer) {
		ImageRawData RenderPixelData = {};

		RenderPixelData.ImagePixels.resize(RBO_PIXEL_SIZE * buffer.Width * buffer.Height);

		RenderPixelData.Width    = buffer.Width;
		RenderPixelData.Height   = buffer.Height;
		RenderPixelData.Channels = buffer.Channels;

		glBindFramebuffer(GL_FRAMEBUFFER, buffer.RenderBuffer);
		// read render_buffer pixel.
		glReadPixels(NULL, NULL, RenderPixelData.Width, RenderPixelData.Height, GL_RGBA, GL_UNSIGNED_BYTE, RenderPixelData.ImagePixels.data());
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);

		return RenderPixelData;
	}

	PsagRenderBufferAttrib PasgRenderbufferOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResFlag;
		return RenderBuffer;
	}

	// **************************************** FrameBuffer ****************************************
	// OpenGL 4.6 frame_buffer bind => texture, bind => render_buffer
	// Update: 2023_12_30. RCSZ
	
	bool PsagFramebufferOGL::CreateBindFamebuffer(PsagFrameBuffer& framebuf) {
		// opengl generate fbo handle.
		if (framebuf == NULL) {
			glGenFramebuffers(1, &framebuf);
			glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

			// opengl handle index > OPENGL_INVALID_HANDEL.
			if (framebuf) { PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "framebuffer.gen create.");           return DEF_PSAGSTAT_SUCCESS; }
			else          { PsagLowLog(LogWarning, PSAG_OGLMAG_LABEL, "framebuffer.gen failed create."); return DEF_PSAGSTAT_FAILED;  }
		}
		else {
			// handle duplicate creation.
			PsagLowLog(LogWarning, PSAG_OGLMAG_LABEL, "framebuffer.gen duplicate create.");
			return DEF_PSAGSTAT_FAILED;
		}
	}

	bool PsagFramebufferOGL::CheckFramebuffer(PsagFrameBuffer& framebuf, ResourceFlag& flag) {
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			glDeleteRenderbuffers(1, &framebuf);

			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "incomplete framebuffer.");
			return DEF_PSAGSTAT_FAILED;
		}
		flag = DEFRES_FLAG_NORMAL;
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagFramebufferOGL::CreateFrameBuffer() {
		if (CreateBindFamebuffer(FrameBuffer))
			return DEF_PSAGSTAT_SUCCESS;
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagFramebufferOGL::TextureBindFBO(const PsagTextureAttrib& texture, uint32_t attachment) {
		bool ResultFlag = false;

		if (attachment > DEF_GL_COLOR_ATTACHMENT_MAX) {
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "frame_buffer attachment num > max.");
			return DEF_PSAGSTAT_FAILED;
		}
		// texture => framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glBindTexture(GL_TEXTURE_2D, texture.Texture);
		{
			if (texture.Texture > OPENGL_INVALID_HANDEL)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, texture.Texture, NULL);
			else {
				PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "invalid handle texture.");
				return DEF_PSAGSTAT_FAILED;
			}
			ResultFlag = CheckFramebuffer(FrameBuffer, ReturnResFlag);
		}
		// unbind fbo,texture.
		glBindTexture(GL_TEXTURE_2D, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		return ResultFlag;
	}

	bool PsagFramebufferOGL::RenderBufferBindFBO(PsagRenderBufferAttrib buffer) {
		bool ResultFlag = false;
		// texture => framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, buffer.RenderBuffer);
		{
			if (buffer.RenderBuffer > OPENGL_INVALID_HANDEL)
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer.RenderBuffer);
			else {
				PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "invalid handle render_buffer.");
				return DEF_PSAGSTAT_FAILED;
			}
			ResultFlag = CheckFramebuffer(FrameBuffer, ReturnResFlag);
		}
		// unbind fbo,rbo.
		glBindRenderbuffer(GL_RENDERBUFFER, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);

		return ResultFlag;
	}

	bool PsagFramebufferOGL::TextureLayerBindFBO(PsagTexture texture_hd, uint32_t layer, uint32_t attachment) {
		bool ResultFlag = false;

		if (attachment > DEF_GL_COLOR_ATTACHMENT_MAX) {
			PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "frame_buffer attachment num > max.");
			return DEF_PSAGSTAT_FAILED;
		}
		// texture_array => framebuffer.
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_hd);
		{
			if (texture_hd > OPENGL_INVALID_HANDEL)
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_hd, NULL, layer);
			else {
				PsagLowLog(LogError, PSAG_OGLMAG_LABEL, "invalid handle texture_array.");
				return DEF_PSAGSTAT_FAILED;
			}
			ResultFlag = CheckFramebuffer(FrameBuffer, ReturnResFlag);
		}
		// unbind fbo,texture_array.
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);

		return ResultFlag;
	}

	PsagFrameBuffer PsagFramebufferOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResFlag;
		return FrameBuffer;
	}

	// **************************************** Uniform ****************************************
	// OpenGL 4.6 GLSL uniform variable.
	// Update: 2023_12_30. RCSZ
	std::atomic<bool> ATC_UNIFORM_ERROR_FLAG = false;

	bool PsagUniformOGL::ProgramHandle(GLuint program, const char* label) {
		if (program) {
			return DEF_PSAGSTAT_SUCCESS;
			ATC_UNIFORM_ERROR_FLAG = false;
		}
		ATC_UNIFORM_ERROR_FLAG = true;
		return DEF_PSAGSTAT_FAILED;
	}

	void PsagUniformOGL::UniformMatrix3x3(PsagShader program, const char* name, const PsagMatrix3& matrix) {
		if (ProgramHandle(program, "mat3x3")) {
			int32_t UniformLocation = glGetUniformLocation(program, name);
			glUniformMatrix3fv(UniformLocation, 1, false, matrix.matrix);
		}
	}

	void PsagUniformOGL::UniformMatrix4x4(PsagShader program, const char* name, const PsagMatrix4& matrix) {
		if (ProgramHandle(program, "mat4x4")) {
			int32_t UniformLocation = glGetUniformLocation(program, name);
			glUniformMatrix4fv(UniformLocation, 1, false, matrix.matrix);
		}
	}

	void PsagUniformOGL::UniformInteger(PsagShader program, const char* name, const int32_t& value) {
		if (ProgramHandle(program, "int")) {
			int32_t UniformLocation = glGetUniformLocation(program, name);
			glUniform1i(UniformLocation, value);
		}
	}

	void PsagUniformOGL::UniformFloat(PsagShader program, const char* name, const float& value) {
		if (ProgramHandle(program, "float")) {
			int32_t UniformLocation = glGetUniformLocation(program, name);
			glUniform1f(UniformLocation, value);
		}
	}

	void PsagUniformOGL::UniformVec2(PsagShader program, const char* name, const Vector2T<float>& value) {
		if (ProgramHandle(program, "vec2f")) {
			int32_t UniformLocation = glGetUniformLocation(program, name);
			glUniform2f(UniformLocation, value.vector_x, value.vector_y);
		}
	}

	void PsagUniformOGL::UniformVec3(PsagShader program, const char* name, const Vector3T<float>& value) {
		if (ProgramHandle(program, "vec3f")) {
			int32_t UniformLocation = glGetUniformLocation(program, name);
			glUniform3f(UniformLocation, value.vector_x, value.vector_y, value.vector_z);
		}
	}

	void PsagUniformOGL::UniformVec4(PsagShader program, const char* name, const Vector4T<float>& value) {
		if (ProgramHandle(program, "vec4f")) {
			int32_t UniformLocation = glGetUniformLocation(program, name);
			glUniform4f(UniformLocation, value.vector_x, value.vector_y, value.vector_z, value.vector_w);
		}
	}
}

namespace RenderingSupport {

	void RenderBindShader(const PsagShader& program) { glUseProgram(program); }
	void RenderUnbindShader()                        { glUseProgram(NULL);    }

	void RenderBindTexture(const PsagTextureAttrib& texture) {
		glActiveTexture(GL_TEXTURE0 + texture.TextureSamplerCount);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture.Texture);
	}
	void RenderUnbindTexture() {
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);
	}

	void RnenderBindFrameBuffer(const PsagFrameBuffer& framebuffer, uint32_t attachment) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
	}
	void RnenderUnbindFrameBuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	}

	void DrawVertexGroup(const PsagVertexBufferAttrib& model) {
		glBindVertexArray(model.DataAttrib);
		glBindBuffer(GL_ARRAY_BUFFER, model.DataBuffer);
		glDrawArrays(GL_TRIANGLES, NULL, GLsizei(model.VerticesDataBytes / model.VertexBytes));
		glBindVertexArray(NULL);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
	}

	void DrawVertexGroupExt(const PsagVertexBufferAttrib& model, size_t length_vertex, size_t offset_vertex) {
		glBindVertexArray(model.DataAttrib);
		glBindBuffer(GL_ARRAY_BUFFER, model.DataBuffer);
		glDrawArrays(GL_TRIANGLES, (GLint)offset_vertex, (GLsizei)length_vertex);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	void UploadVertexGroup(PsagVertexBufferAttrib* model, float* verptr, size_t bytes) {
		// update vertices dataset bytes_param.
		model->VerticesDataBytes = bytes;
		glBindBuffer(GL_ARRAY_BUFFER, model->DataBuffer);
		glBufferData(GL_ARRAY_BUFFER, bytes, verptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
	}
}
namespace UploadTextureGPU {

	void UploadTextureLayerRGBA(const PsagTexture& texture, uint32_t layer, const Vector2T<uint32_t>& size, uint8_t* dataptr) {
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY, NULL, 0, 0, (GLint)layer, size.vector_x, size.vector_y, 1, GL_RGBA,
			GL_UNSIGNED_BYTE, dataptr
		);
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);
	}
}

// **************************************** Information ****************************************
// OpenGL 4.6 get device info. [2023_12_30]
// Update: 2024_01_25. RCSZ

Vector2T<int32_t> SysPsagInformationOGL::InfoGetVertexUnitsMax() {
	GLint DeviceVaoMax = NULL;
	GLint DeviceVboMax = NULL;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &DeviceVaoMax);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &DeviceVboMax);

	if (LoggerPrintInformation) {
		PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "device (vao)attribs_max units: %d entry", DeviceVaoMax);
		PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "device (vbo)buffer_max units: %d entry", DeviceVboMax);
	}

	return Vector2T<int32_t>(DeviceVaoMax, DeviceVboMax);
}

int32_t SysPsagInformationOGL::InfoGetTextureUnitsMax() {
	GLint MaterialMappingUnitsMax = NULL;

	// GL_MAX_TEXTURE_UNITS       - 基本纹理单元数量(老版本固定管线).
	// GL_MAX_TEXTURE_IMAGE_UNITS - 可以用于Shader的纹理单元数量.
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaterialMappingUnitsMax);

	if (LoggerPrintInformation)
		PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "device [tmu]texture_max units: %d entry", MaterialMappingUnitsMax);

	return MaterialMappingUnitsMax;
}

float SysPsagInformationOGL::InfoGetGPUmemoryCapacity() {
	GLint GraphMemoryCapacity = NULL;

	// GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX - 获取图形设备(GPU)显存容量.
	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &GraphMemoryCapacity);
	float GMCmibytes = float(GraphMemoryCapacity) / 1024.0f;

	if (LoggerPrintInformation)
		PsagLowLog(LogPerfmac, PSAG_OGLMAG_LABEL, "device memory[capacity]: %.2f mib", GMCmibytes);

	return GMCmibytes;
}

float SysPsagInformationOGL::InfoGetGPUmemoryUsage() {
	GLint GraphMemoryUsage = NULL;

	// GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX - 获取图形设备(GPU)当前使用显存.
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &GraphMemoryUsage);
	float GMUmibytes = float(GraphMemoryUsage) / 1024.0f;

	if (LoggerPrintInformation)
		PsagLowLog(LogPerfmac, PSAG_OGLMAG_LABEL, "device memory[usage]: %.2f mib", GMUmibytes);

	return GMUmibytes;
}

void SysPsagInformationOGL::InfoPrintPlatformParameters() {
	PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "device gpu supplier: %s",   (const char*)glGetString(GL_VENDOR));
	PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "device gpu model: %s",      (const char*)glGetString(GL_RENDERER));
	PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "device version opengl: %s", (const char*)glGetString(GL_VERSION));
	PsagLowLog(LogInfo, PSAG_OGLMAG_LABEL, "device version glsl: %s",   (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
}