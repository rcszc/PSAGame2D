// framework_renderer_ext. renderer extension interface. RCSZ.
// shader: 顶点着色器, 片元着色器, 几何着色器, 曲面细分控制着色器, 曲面细分评估着色器, 计算着色器, 光线追踪着色器.

#ifndef _FRAMEWORK_RENDERER_EXT_H
#define _FRAMEWORK_RENDERER_EXT_H

#define ENABLE_OPENGL_API
#include "framework_renderer.h"

// graphics pipeline.
class Ext_PsagGLmanagerShaderGP {
public:
	// loader: vertex, fragment shader.
	virtual void ShaderLoaderPushVS(const std::string& vs, ScriptReadMode mode = StringFilepath) = 0;
	virtual void ShaderLoaderPushFS(const std::string& fs, ScriptReadMode mode = StringFilepath) = 0;

	// loader: geometry, tessellation_control, tessellation_evaluation shader.
	virtual void ShaderLoaderGES(const std::string& exts, ScriptReadMode mode = StringFilepath) = 0;
	virtual void ShaderLoaderTCS(const std::string& exts, ScriptReadMode mode = StringFilepath) = 0;
	virtual void ShaderLoaderTES(const std::string& exts, ScriptReadMode mode = StringFilepath) = 0;

	// create: graphics shader res.
	virtual bool CreateCompileShader() = 0;

	virtual PsagShader _MS_GETRES(ResourceFlag& flag) = 0;
};

// compute pipeline.
class Ext_PsagGLmanagerShaderCP {
public:
	// loader: compute shader.
	virtual void ShaderLoaderCS(const std::string& exts, ScriptReadMode mode = StringFilepath) = 0;

	// create: compute(calc) shader res.
	virtual bool CreateCompileShader() = 0;

	virtual PsagShader _MS_GETRES(ResourceFlag& flag) = 0;
};

// ray_tracing pipeline.
class Ext_PsagGLmanagerShaderRTP {
public:
	// loader: ray_tracing shader.
	virtual void ShaderLoaderRTS(const std::string& exts, ScriptReadMode mode = StringFilepath) = 0;
	virtual bool DeviceRayTracing() = 0;

	// create: ray_tracing shader res.
	virtual bool CreateCompileShader() = 0;

	virtual PsagShader _MS_GETRES(ResourceFlag& flag) = 0;
};

#endif