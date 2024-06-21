// framework_renderer. 2024_04_01. RCSZ
// 渲染抽象层: OpenGL 4.6 64bit C++17

#ifndef _FRAMEWORK_RENDERER_H
#define _FRAMEWORK_RENDERER_H

#include "../LLDefine/framework_def_header.h"
#include "../LLDefine/framework_def_math.hpp"

#ifdef ENABLE_OPENGL_API
#define OPENGL_INVALID_HANDEL (GLuint)NULL
// opengl version 4.6 api.
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#define DEFRES_FLAG_NORMAL   1 // 正常资源.
#define DEFRES_FLAG_INVALID -1 // 无效资源.

#define DEF_PSAGSTAT_SUCCESS 1 // TRUE
#define DEF_PSAGSTAT_FAILED  0 // FALSE

#define DEF_IMG_CHANNEL_RGB  3 // texture channels 3 bytes.
#define DEF_IMG_CHANNEL_RGBA 4 // texture channels 4 bytes.

// renderer std attachment_max.
#define DEF_GL_COLOR_ATTACHMENT_MAX 15

enum OPENGL_INIT_PARAM {
	GLINIT_MODE_NORMAL  = 0, // 默认选项.
	GLINIT_MODE_QUALITY = 1, // 质量优先.
	GLINIT_MODE_PERF    = 2  // 性能优先.
};
struct OpenGLInitParams {
	bool PROFILE_CONFIG;
	// parameters items:
	// 贴图压缩, Mipmap纹理生成, 图元平滑, 视图(透视正交)计算, 片段(着色器)自动微分.
	OPENGL_INIT_PARAM TextureCompress;
	OPENGL_INIT_PARAM MipmapGenerate;
	OPENGL_INIT_PARAM ElementsSmooth;
	OPENGL_INIT_PARAM SceneViewCalculate;
	OPENGL_INIT_PARAM FragmentDerivative;

	OpenGLInitParams() :
		PROFILE_CONFIG     (true),
		TextureCompress    (GLINIT_MODE_NORMAL),
		MipmapGenerate     (GLINIT_MODE_NORMAL),
		ElementsSmooth     (GLINIT_MODE_NORMAL),
		SceneViewCalculate (GLINIT_MODE_NORMAL),
		FragmentDerivative (GLINIT_MODE_NORMAL)
	{}
};

using INIT_PARAMETERS = OpenGLInitParams;
using INIT_RETURN     = GLenum;

using ResourceFlag = int;

using PsagShader          = GLuint;
using PsagTexture         = GLuint;
using PsagVertexBuffer    = GLuint;
using PsagVertexAttribute = GLuint;
using PsagFrameBuffer     = GLuint;
using PsagRenderBuffer    = GLuint;

#endif

// shader read script mode. 
enum ScriptReadMode {
	StringFilepath = 1 << 1, // 着色器文件路径
	StringScript   = 1 << 2  // 着色器文本
};

// image_load mode type.
enum SaveImageMode {
	ImageJPG = 1 << 1, // .jpg 图像文件, 3x8bit
	ImagePNG = 1 << 2, // .png 图像文件, 4x8bit
	ImageBMP = 1 << 3  // .bmp 图像文件.
};

// PSA-V0.1.2 GL-TEX 标准. [GL_TEX_04] FILTER.
// shader sampling texture filter mode.
enum TextureFilterMode {
	NearestNeighborFiltering = 1 << 1, // 邻近过滤
	LinearFiltering          = 1 << 2, // 线性过滤
	AnisotropicFiltering     = 1 << 3, // 各向异性过滤
	MipmapFiltering          = 1 << 4  // Mipmap过滤
};
inline TextureFilterMode operator|(TextureFilterMode a, TextureFilterMode b) {
	return static_cast<TextureFilterMode>(static_cast<int>(a) | static_cast<int>(b));
}

// renderer logger function object.
using RendererLogger = void (*)(const LOGLABEL&, const char*, const char*, ...);

#endif