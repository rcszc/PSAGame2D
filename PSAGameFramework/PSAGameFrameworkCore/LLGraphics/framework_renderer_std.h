// framework_renderer_std. renderer standard interface. RCSZ.
// shader: 顶点着色器, 片元着色器.

#ifndef _FRAMEWORK_RENDERER_STD_H
#define _FRAMEWORK_RENDERER_STD_H

#define ENABLE_OPENGL_API
#include "framework_renderer.h"

// renderer layer initialization.
class PSAGGL_GLOBAL_INIT {
public:
	virtual INIT_RETURN RendererInit(INIT_PARAMETERS init_param, const std::string& version) = 0;
	virtual void LoggerFunc(RendererLogger function) = 0;
};

// ******************************** Manager ********************************
// Renderer PSAGGL, OperManager.

class PsagGLmanagerShader {
public:
	// loader: vertex, fragment shader.
	virtual void ShaderLoaderPushVS(const std::string& vs, ScriptReadMode mode = StringFilepath) = 0;
	virtual void ShaderLoaderPushFS(const std::string& fs, ScriptReadMode mode = StringFilepath) = 0;

	virtual bool CreateCompileShader() = 0;

	virtual PsagShader _MS_GETRES(ResourceFlag& flag) = 0;
};

// texture scale(x,y) param.
struct TextureParam {
	float WidthScale, HeightScale;

	TextureParam() : WidthScale(0), HeightScale(0) {}
	TextureParam(float x, float y) : WidthScale(x), HeightScale(y) {}
};

//  PSA-V0.1.2 GL-TEX-ATTR 标准. [GL_TEX_ATTR_01]
// texture(array) attribute.
struct PsagTextureAttrib {
	std::vector<TextureParam> LayersParams;
	TextureFilterMode FilterModeType;

	uint32_t Layers, Width, Height, Channels;
	PsagTexture Texture;
	uint32_t TextureSamplerCount;

	PsagTextureAttrib() :
		Texture{}, LayersParams({}), FilterModeType(LinearFiltering), TextureSamplerCount(0),
		Layers(0), Width(0), Height(0), Channels(0)
	{}
};

// texture(group) create, layers_size equal.
class PsagGLmanagerTexture {
public:
	virtual bool SetTextureParam(uint32_t width, uint32_t height, TextureFilterMode mode) = 0;
	virtual bool SetTextureSamplerCount(uint32_t count) = 0;

	virtual bool PsuhCreateTexEmpty(const TextureParam& param) = 0;
	virtual bool PushCreateTexData(const TextureParam& param, uint32_t channels, const RawDataStream& data) = 0;
	virtual bool PushCreateTexLoader(const TextureParam& param, const std::string& file) = 0;

	virtual bool CreateTexture() = 0;

	virtual PsagTextureAttrib _MS_GETRES(ResourceFlag& flag) = 0;
};

//  PSA-V0.1.2 GL-TEX-ATTR 标准. [GL_TEX_ATTR_02]
// texture view, static.
struct PsagTextureView {
	uint32_t Width, Height, Channels;
	PsagTexture Texture;
};

// "ImageRawData" define.
struct ImageRawData;
// texture2d, not sampler cunt.
// non "LLRES" manager.
class PsagGLmanagerTextureView {
public:
	virtual bool CreateTexViewEmpty(uint32_t width, uint32_t height, TextureFilterMode mode) = 0;
	virtual bool CreateTexViewData(const ImageRawData& image_data, TextureFilterMode mode) = 0;

	virtual PsagTextureView CreateTexture() = 0;
};

// vertex_buffer & vertex_attribute.
struct PsagVertexBufferAttrib {
	// 顶点组大小(bytes), 顶点数据(集)大小(bytes).
	size_t VertexBytes, VerticesDataBytes;

	PsagVertexBuffer DataBuffer;
	PsagVertexAttribute DataAttrib;

	PsagVertexBufferAttrib() : DataBuffer{}, DataAttrib{}, VertexBytes(0), VerticesDataBytes(0) {}
	PsagVertexBufferAttrib(PsagVertexBuffer vb, PsagVertexAttribute va, size_t ver, size_t size) :
		DataBuffer(vb), DataAttrib(va), VertexBytes(ver), VerticesDataBytes(size)
	{}
};

class PsagGLmanagerModel {
public:
	virtual PsagVertexAttribute CreateVertexAttribute(uint32_t type, uint32_t begin_location = 0) = 0;
	virtual PsagVertexBuffer CreateVertexBuffer() = 0;

	virtual bool CreateStaticModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) = 0;
	virtual bool CreateDynamicModel(PsagVertexAttribute veratt, PsagVertexBuffer verbuf, const float* verptr, size_t bytes) = 0;

	virtual PsagVertexBufferAttrib _MS_GETRES(ResourceFlag& flag) = 0;
};

// render_buffer(rbo) attribute.
struct PsagRenderBufferAttrib {
	uint32_t Width, Height, Channels;
	size_t TextureBytes;

	PsagRenderBuffer RenderBuffer;

	PsagRenderBufferAttrib() : RenderBuffer{}, Width(NULL), Height(NULL), Channels(NULL), TextureBytes(NULL) {}
	PsagRenderBufferAttrib(uint32_t x, uint32_t y, uint32_t ch, PsagRenderBuffer rb) :
		Width(x), Height(y), Channels(ch), TextureBytes(size_t(x * y * ch) * sizeof(float)), RenderBuffer(rb)
	{}
};

class PsagGLmanagerFrameBuffer {
public:
	virtual bool CreateFrameBuffer() = 0;

	virtual bool TextureBindFBO(const PsagTextureAttrib& texture, uint32_t attachment) = 0;
	virtual bool RenderBufferBindFBO(PsagRenderBufferAttrib buffer) = 0;

	virtual PsagFrameBuffer _MS_GETRES(ResourceFlag& flag) = 0;
};

// raw_image pixel data.
struct ImageRawData {
	uint32_t Width, Height, Channels;
	RawDataStream ImagePixels;
};

class PsagGLmanagerRenderBuffer {
public:
	virtual bool CreateRenderBuffer(uint32_t width, uint32_t height) = 0;
	virtual ImageRawData ReadRenderBuffer(PsagRenderBufferAttrib buffer) = 0;

	virtual PsagRenderBufferAttrib _MS_GETRES(ResourceFlag& flag) = 0;
};

class PsagGLmanagerUniform {
public:
	virtual void UniformMatrix3x3 (PsagShader program, const char* name, const PsagMatrix3& matrix) = 0;
	virtual void UniformMatrix4x4 (PsagShader program, const char* name, const PsagMatrix4& matrix) = 0;

	virtual void UniformFloat   (PsagShader program, const char* name, const float&           value) = 0;
	virtual void UniformVec2    (PsagShader program, const char* name, const Vector2T<float>& value) = 0;
	virtual void UniformVec3    (PsagShader program, const char* name, const Vector3T<float>& value) = 0;
	virtual void UniformVec4    (PsagShader program, const char* name, const Vector4T<float>& value) = 0;
	virtual void UniformInteger (PsagShader program, const char* name, const int32_t&         value) = 0;
};

// ******************************** Resource(ThreadSafe) ********************************
// Renderer PSAGGL, ResourceProcessing.

// resource: texture mapping unit.
class PsagGLresourceTMU {
public:
	virtual uint32_t AllocTmuCount() = 0;
	virtual void FreeTmuCount(uint32_t count) = 0;
};
 
class PsagGLresourceShader {
public:
	virtual PsagShader ResourceFind(ResUnique key) = 0;
	virtual bool ResourceStorage(ResUnique key, PsagGLmanagerShader* res) = 0;
	virtual bool ResourceDelete(ResUnique key) = 0;

	virtual size_t ResourceSize() = 0;
	virtual ~PsagGLresourceShader() = default;
};

class PsagGLresourceTexture {
public:
	virtual PsagTextureAttrib ResourceFind(ResUnique key) = 0;
	virtual bool ResourceStorage(ResUnique key, PsagGLmanagerTexture* res) = 0;
	virtual bool ResourceDelete(ResUnique key) = 0;

	virtual size_t ResourceSize() = 0;
	virtual ~PsagGLresourceTexture() = default;
};

// VertexBufferHD + VertexAttributeHD => VertexBufferAttr
class PsagGLresourceVertexBuffer {
public:
	virtual PsagVertexBufferAttrib ResourceFind(ResUnique key) = 0;
	virtual bool ResourceStorage(ResUnique key, PsagGLmanagerModel* res) = 0;
	virtual bool ResourceDelete(ResUnique key) = 0;

	virtual size_t ResourceSize() = 0;
	virtual ~PsagGLresourceVertexBuffer() = default;
};

class PsagGLresourceVertexAttribute {
public:
	virtual PsagVertexAttribute ResourceFind(ResUnique key) = 0;
	virtual bool ResourceStorage(ResUnique key, PsagVertexAttribute res) = 0;
	virtual bool ResourceDelete(ResUnique key) = 0;

	virtual size_t ResourceSize() = 0;
	virtual ~PsagGLresourceVertexAttribute() = default;
};

class PsagGLresourceFrameBuffer {
public:
	virtual PsagFrameBuffer ResourceFind(ResUnique key) = 0;
	virtual bool ResourceStorage(ResUnique key, PsagGLmanagerFrameBuffer* res) = 0;
	virtual bool ResourceDelete(ResUnique key) = 0;

	virtual size_t ResourceSize() = 0;
	virtual ~PsagGLresourceFrameBuffer() = default;
};

class PsagGLresourceRenderBuffer {
public:
	virtual PsagRenderBufferAttrib ResourceFind(ResUnique key) = 0;
	virtual bool ResourceStorage(ResUnique key, PsagGLmanagerRenderBuffer* res) = 0;
	virtual bool ResourceDelete(ResUnique key) = 0;

	virtual size_t ResourceSize() = 0;
	virtual ~PsagGLresourceRenderBuffer() = default;
};

// ******************************** Input/Output ********************************
// Renderer PSAGGL, IO_Manager.

class PsagIOmanagerImage {
public:
	virtual bool WriteImageFile(const ImageRawData& rawdata, const std::string& file, SaveImageMode mode, float quality) = 0;
	virtual ImageRawData ReadImageFile(std::string file) = 0;
};

class PsagIOmanagerImageRawData {
public:
	virtual RawDataStream EncodeImageRawData(const ImageRawData& rawdata, SaveImageMode mode, float quality) = 0;
	virtual ImageRawData DecodeImageRawData(const RawDataStream& rawdata) = 0;
};

#endif