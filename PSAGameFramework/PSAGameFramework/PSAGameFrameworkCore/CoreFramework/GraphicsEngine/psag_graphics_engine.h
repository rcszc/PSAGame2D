// psag_graphics_engine.

#ifndef __PSAG_GRAPHICS_ENGINE_H
#define __PSAG_GRAPHICS_ENGINE_H

#define ENABLE_LOWMODULE_GRAPHICS
#include "../psag_lowlevel_support.h"

#define PSAGM_VIR_TICKSTEP 0.058f

// 统一数据集管理,单一共享 VAO,VBO,TEXTURE.
namespace GraphicsEngineDataset {
#define PSAGM_GLENGINE_DATA_LABEL "PSAG_GL_DATASET"

	// LLRES => stc_vertex_data system. (static vertex data) [静态矩形]
	// => MODULE(update: 20240506):
	// derive class:
	// "GraphicsEnginePost::PsagGLEnginePost"
	// "GraphicsEngineBackground::PsagGLEngineBackground"
	class GLEngineStcVertexData :public PsagLow::PsagSupGraphicsLLRES {
	private:
		// opengl vao,vbo handle(res).
		static std::string VertexAttribute;
		static std::string VertexBuffer;
	public:
		void StaticVertexFrameDraw();

		// static vertex system: framework oper.
		void StaticVertexDataObjectCreate();
		void StaticVertexDataObjectDelete();
	};

	struct VABO_DATASET_INFO {
		size_t DatasetLength;
		size_t DatasetOffsetLength;

		VABO_DATASET_INFO() : DatasetLength(NULL), DatasetOffsetLength(NULL) {}
	};

	// LLRES => dy_vertex_data system. (dynamic vertex data)
	// => MODULE(update: 20240505):
	// derive class:
	// "GraphicsEngineParticle::PsagGLEngineParticle"
	class GLEngineDyVertexData :public PsagLow::PsagSupGraphicsLLRES {
	private:
		// opengl vao,vbo handle(res).
		static std::string VertexAttribute;
		static std::string VertexBuffer;

		static std::unordered_map<std::string, VABO_DATASET_INFO> IndexItems;
		static std::vector<float> VertexRawDataset;

		static bool GLOBAL_UPDATE_FLAG;
		void UpdateVertexDyDataset();
	protected:
		bool VerDataItemAlloc(ResUnique strkey);
		bool VerDataItemFree(ResUnique strkey);

		bool VerOperFramePushData(ResUnique strkey, const std::vector<float>& data);
		bool VerOperFrameDraw(ResUnique strkey);

		// dynamic vertex system: framework oper.
		void VertexDataObjectCreate();
		void VertexDataObjectDelete();

		// framework core system_update.
		void SystemFrameUpdateNewState();
	};

	// 纹理数层组分配器 => "VirTexturesGenParams".
	namespace TextureLayerAlloc {
		class TEX_LAYER_ALLOC {
		protected:
			std::vector<bool> TexArrayStateFlag = {};
		public:
			TEX_LAYER_ALLOC(size_t tmu_size) {
				TexArrayStateFlag.resize(tmu_size);
			}
			// texture(array) system: alloc & free, res_count.
			uint32_t AllocLayerCount();
			void FreeLayerCount(uint32_t count);
		};
	}

	// sampler textures(global) attribute.
	struct SamplerTextures {
		Vector2T<uint32_t> ArraySize; // x:max, y:usage.
		Vector2T<uint32_t> TextureResolution;

		std::string TextureArrayIndex;
		TextureLayerAlloc::TEX_LAYER_ALLOC* LayerAllotter;
	};
	// virtual texture(item) params: cropping,size.
	struct VirTextureParam {
		// 1:1/1, 2:1/2, 3:1/4, 4:1/8.
		uint32_t ResolutionType;
		std::string Texture;

		uint32_t        SampleLayers;   // simpler-z offset
		Vector2T<float> SampleCropping; // x,y:[0.0f,1.0f]
		Vector2T<float> SampleSize;     // x,y:[image_size]

		// str process buffer.
		std::string StrProcess = {};

		VirTextureParam() : ResolutionType(NULL), Texture({}), SampleLayers(NULL), SampleCropping({}), SampleSize({}) {};
		VirTextureParam(uint32_t type, const std::string strkey, uint32_t layer, const Vector2T<float>& smpcro, const Vector2T<float>& smpsz) :
			ResolutionType(type), Texture(strkey), SampleLayers(layer), SampleCropping(smpcro), SampleSize(smpsz)
		{};
	};

	struct VirTextureUniformName {
		const char* TexParamSampler;  // u_type"sampler2DArray"
		const char* TexParamLayer;    // u_type"int"
		const char* TexParamCropping; // u_type"vec2"
		const char* TexParamSize;     // u_type"vec2"
	};

	// global textures generate numbers.
	struct VirTexturesGenParams {
		// size: 1/8, 1/4, 1/2, 1/1.
		size_t Tex1Xnum, Tex2Xnum, Tex4Xnum, Tex8Xnum;

		VirTexturesGenParams() : Tex1Xnum(128), Tex2Xnum(96), Tex4Xnum(72), Tex8Xnum(40) {};
		VirTexturesGenParams(size_t t1x, size_t t2x, size_t t4x, size_t t8x) :
			Tex1Xnum(t1x), Tex2Xnum(t2x), Tex4Xnum(t4x), Tex8Xnum(t8x) 
		{};
	};
	// LLRES => texture(array) data system. (sampler texture data)
	// => MODULE(update: 20240513):
	// derive class:
	class GLEngineSmpTextureData :public PsagLow::PsagSupGraphicsLLRES {
	private:
		static PsagLow::PsagSupGraphicsTool::PsagGraphicsUniform ShaderUniform;

		static SamplerTextures TexturesSize1X; // 1/8 resolution.
		static SamplerTextures TexturesSize2X; // 1/4 resolution.
		static SamplerTextures TexturesSize4X; // 1/2 resolution.
		static SamplerTextures TexturesSize8X; // 1/1 resolution.

		// virtual texture items index.
		static std::unordered_map<std::string, VirTextureParam> TexIndexItems;

		uint32_t CheckResolutionType(const Vector2T<uint32_t>& size);
		VirTextureParam* FindTexIndexItems(ResUnique strkey);
	protected:
		bool VirTextureItemAlloc(ResUnique strkey, const ImageRawData& image);
		bool VirTextureItemAlloc(ResUnique strkey, const Vector2T<uint32_t>& size);
		bool VirTextureItemFree(ResUnique strkey);

		// bind texture(array), layer(s) => virtual texture.
		// upload uniform param, "uniform_name".
		bool VirTextureItemDraw(ResUnique strkey, PsagShader shader, const VirTextureUniformName& uniform_name);
		// virtual texture & layer_index.
		bool VirTextureItemIndex(ResUnique strkey, PsagTexture& texture, uint32_t layer_index);

		// virtual texture system: framework oper.
		void VirtualTextureDataObjectCreate(
			Vector2T<uint32_t> base_size, const VirTexturesGenParams& params = VirTexturesGenParams()
		);
		void VirtualTextureDataObjectDelete();
	};
}

namespace GraphicsShaderScript {
	extern const char* PsagShaderScriptPublicVS;
	// private fragment shader script.
	extern const char* PsagShaderScriptBloomHFS;
	extern const char* PsagShaderScriptBloomVFS;
	extern const char* PsagShaderScriptPostFS;
	extern const char* PsagShaderScriptBackFS;
	extern const char* PsagShaderScriptParticleFS;
	extern const char* PsagShaderScriptFxSequenceFS;
}

namespace GraphicsEnginePost {
#define PSAGM_GLENGINE_POST_LABEL "PSAG_GL_POSTFX"

	struct PostFxParameters {
		// bloom_radius [1-32] calc & filter avg_color.
		uint32_t GameSceneBloomRadius;
		float    GameSceneBloomFilter;
		// bloom_blend x:source, y:blur.
		Vector2T<float> GameSceneBloomBlend;

		PostFxParameters() :
			GameSceneBloomRadius(1), GameSceneBloomFilter(0.0f), GameSceneBloomBlend(Vector2T<float>(0.5f, 0.5f))
		{}
	};

	class PsagGLEnginePost :public GraphicsEngineDataset::GLEngineStcVertexData {
	protected:
		PsagLow::PsagSupGraphicsTool::PsagGraphicsUniform ShaderUniform = {};

		Vector2T<float> RenderResolution = {};
		Vector2T<float> RenderMove       = {};
		Vector2T<float> RenderScale      = Vector2T<float>(1.0f, 1.0f);

		PsagMatrix4 RenderingMatrixMvp = {};
		// scene => bloom_h + bloom_v => post_shader.
		std::string ShaderProgramItem = {};
		std::string ShaderBloomH = {}, ShaderBloomV = {};
		
		std::string GameSceneFrameBuffer = {};
		std::string FrameBufferBloomH = {}, FrameBufferBloomV = {};
		// texture_cube: 0:frame_buffer_tex, 1:bloom_h_tex, 2:bloom_v_tex
		std::string ProcessTextures = {};

		void BloomShaderProcessHV();
	public:
		PsagGLEnginePost(const Vector2T<uint32_t>& render_resolution);
		~PsagGLEnginePost();

		// game_main scene => capture => process.
		bool CaptureGameScene(const std::function<bool()>& rendering_func);

		// rendering-event.
		bool RenderingPostModule();
		PostFxParameters RenderParameters = {};
	};
}

namespace GraphicsEngineBackground {
#define PSAGM_GLENGINE_BACK_LABEL "PSAG_GL_BACKFX"

	struct BackFxParameters {
		// value[0.0,1.0]
		float BackgroundVisibility;
		// strength x:back_v, y:front_v.
		Vector2T<float> BackgroundStrength;
		Vector4T<float> BackgroundColor;

		BackFxParameters() :
			BackgroundVisibility (1.0f), 
			BackgroundColor      (Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f)),
			BackgroundStrength   (Vector2T<float>(1.0f, 1.0f))
		{}
	};

	class PsagGLEngineBackground :public GraphicsEngineDataset::GLEngineStcVertexData {
	protected:
		float RunTimeTick = 0.0f;
		PsagLow::PsagSupGraphicsTool::PsagGraphicsUniform ShaderUniform = {};

		Vector2T<float> RenderResolution = {};
		Vector2T<float> RenderMove       = {};
		Vector2T<float> RenderScale      = Vector2T<float>(1.0f, 1.0f);

		PsagMatrix4 RenderingMatrixMvp = {};
		// x:tex_idx[1,n-1], y:tex_idx[n].
		std::string ShaderProgramItem = {};

		Vector2T<float> RenderTexIndex = {};
		// texture3d(n * layers).
		std::string BackgroundTextures = {};
	public:
		PsagGLEngineBackground(
			const Vector2T<uint32_t>& render_resolution, const std::vector<ImageRawData>& imgdataset
		);
		~PsagGLEngineBackground();

		// rendering-event.
		bool RenderingBackgroundModule();
		BackFxParameters RenderParameters = {};
	};
}

namespace GraphicsEngineParticle {
#define PSAGM_GLENGINE_PARTICLE_LABEL "PSAG_GL_PARTICLE"

	enum EmittersMode {
		PrtcPoints = 1 << 1, // 点云 [扩散]
		PrtcDrift  = 1 << 2, // 飘落 [效果]
		PrtcCircle = 1 << 3, // 圆形 [扩散]
		PrtcSquare = 1 << 4, // 矩形 [扩散]
	};

	enum ColorChannelMode {
		Grayscale   = 1 << 1, // 灰度(单通道R-RGB)
		ChannelsRG  = 1 << 2, // R,G 通道
		ChannelsRB  = 1 << 3, // R,B 通道
		ChannelsGB  = 1 << 4, // G,B 通道
		ChannelsRGB = 1 << 5  // R,G,B 通道
	};

	struct ParticleAttributes {
		Vector3T<float> ParticleVector;
		Vector3T<float> ParticlePosition;
		Vector4T<float> ParticleColor;

		float ParticleLife;
		ColorChannelMode ParticleModeType;

		constexpr ParticleAttributes() :
			ParticleVector   ({}),
			ParticlePosition ({}),
			ParticleColor    ({}),
			ParticleLife     (0.0f),
			ParticleModeType (Grayscale)
		{}
	};

	struct ParticleSystemState {
		// real-time status.
		size_t DarwParticlesNumber;
		size_t DarwDatasetSize;
	};

	// 创建例子效果(使用[均匀分布]随机数模型).
	class ParticleGeneratorBase {
	public:
		// particle generation number[n > 8].
		virtual bool ConfigCreateNumber(float number) = 0;

		virtual void ConfigCreateMode(EmittersMode mode) = 0;                   // 运算模式.
		virtual void ConfigLifeDispersion(Vector2T<float> rand_limit_life) = 0; // 随机生命.

		virtual void ConfigRandomColorSystem(
			Vector2T<float> r, Vector2T<float> g, Vector2T<float> b,
			ColorChannelMode mode = Grayscale
		) = 0;
		// 粒子空间: vector.xy scale[min,max], position.xy scale[min,max].
		// "OffsetPosition" 创建偏移位置(中心偏移).
		virtual void ConfigRandomDispersion(
			Vector2T<float> rand_limit_vector,
			Vector2T<float> rand_limit_position,
			Vector3T<float> offset_position
		) = 0;

		virtual void CreateAddParticleDataset(std::vector<ParticleAttributes>& data) = 0;
	};

	class ParticleGenerator :public ParticleGeneratorBase {
	protected:
		std::vector<ParticleAttributes> ParticlesGenCache = {};

		EmittersMode ParticlesModeType = {};
		size_t       ParticlesNumber   = 8;

		bool EnableGrayscale = false;
		Vector3T<Vector2T<float>> RandomColorSystem = {};
		Vector2T<uint32_t>        RandomLimitLife   = {};

		Vector3T<float> PositionOffset = {};
		Vector2T<float> RandomLimitPosition = {}, RandomLimitVector = {};

	public:
		bool ConfigCreateNumber(float number) override;
		void ConfigCreateMode(EmittersMode mode) override;
		void ConfigLifeDispersion(Vector2T<float> rand_limit_life) override;

		// color system(random) channels[0.0,1.0].
		void ConfigRandomColorSystem(
			Vector2T<float> r, Vector2T<float> g, Vector2T<float> b,
			ColorChannelMode mode = Grayscale
		) override;
		void ConfigRandomDispersion(
			// randomly distributed parameters.
			Vector2T<float> rand_limit_vector,   // vec random[min,max] pos += vec * speed.
			Vector2T<float> rand_limit_position, // pos random[min,max].
			Vector3T<float> offset_position = Vector3T<float>()
		) override;

		void CreateAddParticleDataset(std::vector<ParticleAttributes>& Data) override;
	};

	class PsagGLEngineParticle :
		public GraphicsEngineDataset::GLEngineDyVertexData,
		public GraphicsEngineDataset::GLEngineSmpTextureData
	{
	protected:
		PsagLow::PsagSupGraphicsTool::PsagGraphicsUniform ShaderUniform = {};

		std::vector<ParticleAttributes> DataParticles = {};
		std::vector<float>              DataVertices  = {};

		std::string ShaderProgramItem = {};
		std::string DyVertexSysItem   = {};

		float       RenderTimer     = 0.0f;
		PsagMatrix4 RenderMatrix    = {};

		Vector2T<float> RenderMove  = {};
		Vector2T<float> RenderScale = Vector2T<float>(1.0f, 1.0f);

		std::string VirTextureItem = {};
		GraphicsEngineDataset::VirTextureUniformName VirTextureUniform = {};

		void CalcUpdateParticles(std::vector<ParticleAttributes>& particles, float speed, float lifesub);
		void VertexDataConvert(const std::vector<ParticleAttributes>& src, std::vector<float>& cvt);
	public:
		PsagGLEngineParticle(const Vector2T<uint32_t>& render_resolution, const ImageRawData& image = {});
		~PsagGLEngineParticle();

		void ParticleCreate(ParticleGeneratorBase* generator);

		ParticleSystemState				 GetParticleState();
		std::vector<ParticleAttributes>* GetParticleDataset();

		void UpdateParticleData();
		void RenderParticleFX();
	};
}

namespace GraphicsEnginePVFX {
#define PSAGM_GLENGINE_PVFX_LABEL "PSAG_GL_PVFX"

	// captrue => texture_view.
	class PsagGLEngineFxCaptureView :public PsagLow::PsagSupGraphicsLLRES {
	protected:
		PsagTextureView TextureViewItem = {};
		std::string FrameBufferItem = {};
	public:
		PsagGLEngineFxCaptureView(const Vector2T<uint32_t>& render_resolution);
		~PsagGLEngineFxCaptureView();

		void CaptureContextBind();
		void CaptureContextUnBind();

		PsagTexture GetCaptureTexView() {
			return TextureViewItem.Texture;
		};
	};

	struct SequencePlayer {
		float PlayerSpeedScale;

		float UaxisFrameNumber;
		float VaxisFrameNumber;
	};
	// 特效序列贴图.
	class PsagGLEngineFxSequence :
		public GraphicsEngineDataset::GLEngineSmpTextureData,
		public GraphicsEngineDataset::GLEngineStcVertexData
	{
	protected:
		PsagLow::PsagSupGraphicsTool::PsagGraphicsUniform ShaderUniform = {};
		SequencePlayer  PlayerParams   = {};
		Vector2T<float> PlayerPosition = {};
		float           PlayerTimer    = 0.0f;

		std::string ShaderProgramItem = {};
		float       RenderTimer       = 0.0f;
		PsagMatrix4 RenderMatrix      = {};

		std::string VirTextureItem = {};
		GraphicsEngineDataset::VirTextureUniformName VirTextureUniform = {};
	public:
		PsagGLEngineFxSequence(const ImageRawData& image, const SequencePlayer& params);
		~PsagGLEngineFxSequence();

		bool DrawFxSequence(const Vector4T<float>& blend_color);
	};
}

#endif