// psag_graphics_engine_system. RCSZ. [middle_level_engine]
// update: 2024_09_26.

#ifndef __PSAG_GRAPHICS_ENGINE_H
#define __PSAG_GRAPHICS_ENGINE_H

#define ENABLE_LOWMODULE_GRAPHICS
#include "../psag_lowlevel_support.h"

// graphics_engine timestep, mainevent => GE.
class __GRAPHICS_ENGINE_TIMESETP {
protected:
	static float GraphicsEngineTimeStep;
};

#define PSAGM_VIR_TICKSTEP_GL 0.058f

// 统一数据集管理,单一共享 VAO,VBO,TEXTURE(TMU).
// THREAD-SAFE.
namespace GraphicsEngineDataset {
	StaticStrLABEL PSAGM_GLENGINE_DATA_LABEL = "PSAG_GL_DATASET";

	class GLEngineDataSTATE {
	protected:
		static std::atomic<size_t> DataBytesStaticVertex;
		static std::atomic<size_t> DataBytesDynamicVertex;
		static std::atomic<size_t> DataBytesOnlineTexture;
	};

	struct VABO_DATASET_INFO {
		size_t DatasetLength;
		size_t DatasetOffsetLength;

		VABO_DATASET_INFO() : DatasetLength(NULL), DatasetOffsetLength(NULL) {}
	};

	// LLRES => stc_vertex_data system. (static vertex data)
	// static vertex_data alloc/delete, thread_safe.
	class GLEngineStaticVertexData :public PsagLow::PsagSupGraphicsLLRES, public GLEngineDataSTATE {
	private:
		static PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER;
		// opengl vao,vbo handle(res).
		static ResUnique VertexAttribute;
		static ResUnique VertexBuffer;

		static std::unordered_map<ResUnique, VABO_DATASET_INFO> IndexItems;
		static ResUnique SystemPresetRectangle;

		// dataset thread: vertex(static) resource mutex.
		static std::mutex DatasetResMutex;
	protected:
		// width: +-10.0f, height: +-10.0f
		ResUnique GetPresetRect() { return SystemPresetRectangle; }

		// ALLOC & FREE : [T-SAFE]
		bool VerStcDataItemAlloc(ResUnique rukey, const std::vector<float>& data);
		bool VerStcDataItemFree(ResUnique rukey);
		// DRAW [T-SAFE]
		bool VerStcOperFrameDraw(ResUnique rukey);

		// static vertex system: framework oper.
		void StaticVertexDataObjectCreate();
		void StaticVertexDataObjectDelete();
	};

	// LLRES => dy_vertex_data system. (dynamic vertex data)
	// dynamic vertex_data alloc/delete/push, thread_safe.
	class GLEngineDynamicVertexData :public PsagLow::PsagSupGraphicsLLRES, public GLEngineDataSTATE {
	private:
		static PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER;
		// opengl vao,vbo handle(res).
		static ResUnique VertexAttribute;
		static ResUnique VertexBuffer;

		static std::unordered_map<ResUnique, VABO_DATASET_INFO> IndexItems;
		static std::vector<float> VertexRawDataset;

		static bool GLOBAL_UPDATE_FLAG;
		void UpdateVertexDyDataset();

		// dataset thread: vertex(dynamic) resource mutex.
		static std::mutex DatasetResMutex;
	protected:
		bool VerDyDataItemAlloc(ResUnique rukey); // [T-SAFE]
		bool VerDyDataItemFree(ResUnique rukey);  // [T-SAFE]

		bool VerDyOperFramePushData(ResUnique rukey, const std::vector<float>& data); // [T-SAFE]
		bool VerDyOperFrameDraw(ResUnique rukey); // [T-SAFE]

		// dynamic vertex system: framework oper.
		void DynamicVertexDataObjectCreate();
		void DynamicVertexDataObjectDelete();

		// framework core system_update. [T-SAFE]
		void SystemFrameUpdateNewState();
	};

	// 纹理数层组分配器 => "VirTexturesGenParams".
	namespace TextureLayerAlloc {
		class TEX_LAYER_ALLOC {
		protected:
			std::vector<bool> TexArrayStateFlag = {};
		public:
			TEX_LAYER_ALLOC(size_t tmu_size) {
				// texture array(virtual tex) init_setting "slot".
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

		ResUnique TextureArrayIndex;
		TextureLayerAlloc::TEX_LAYER_ALLOC* LayerAllotter;
	};
	// virtual texture(item) params: cropping,size.
	struct VirTextureParam {
		// 1:1/1, 2:1/2, 3:1/4, 4:1/8.
		uint32_t ResolutionType;
		ResUnique Texture;

		uint32_t        SampleLayers;   // texture layers_offset.
		Vector2T<float> SampleCropping; // x,y:[0.0f,1.0f]
		Vector2T<float> SampleSize;     // x,y:[image_size]

		VirTextureParam() : ResolutionType(NULL), Texture(NULL), SampleLayers(NULL), SampleCropping({}), SampleSize({}) {};
		VirTextureParam(uint32_t type, ResUnique rukey, uint32_t layer, const Vector2T<float>& smpcro, const Vector2T<float>& smpsz) :
			ResolutionType(type), Texture(rukey), SampleLayers(layer), SampleCropping(smpcro), SampleSize(smpsz)
		{};
	};

	struct VirTextureUniformName {
		const char* TexParamSampler;  // uniform_type: "sampler2DArray"
		const char* TexParamLayer;    // uniform_type: "int"
		const char* TexParamCropping; // uniform_type: "vec2"
		const char* TexParamSize;     // uniform_type: "vec2"
	};

	// global textures generate numbers.
	struct VirTexturesGenParams {
		// size: 1/8, 1/4, 1/2, 1/1.
		size_t Tex1Xnum, Tex2Xnum, Tex4Xnum, Tex8Xnum;

		VirTexturesGenParams() : Tex1Xnum(100), Tex2Xnum(50), Tex4Xnum(50), Tex8Xnum(25) {};
		VirTexturesGenParams(size_t t1x, size_t t2x, size_t t4x, size_t t8x) :
			Tex1Xnum(t1x), Tex2Xnum(t2x), Tex4Xnum(t4x), Tex8Xnum(t8x) 
		{};
	};
	// LLRES => texture(array) data system. (sampler texture data)
	// => MODULE(update: 20240513):
	// derive class:
	class GLEngineVirTextureData :public PsagLow::PsagSupGraphicsLLRES, public GLEngineDataSTATE {
	private:
		static PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER;
		static PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform;

		static SamplerTextures TexturesSize1X; // 1/8 resolution.
		static SamplerTextures TexturesSize2X; // 1/4 resolution.
		static SamplerTextures TexturesSize4X; // 1/2 resolution.
		static SamplerTextures TexturesSize8X; // 1/1 resolution.

		// virtual texture items index.
		static std::unordered_map<ResUnique, VirTextureParam> TexIndexItems;

		uint32_t CheckResolutionType(const Vector2T<uint32_t>& size);
		VirTextureParam* FindTexIndexItems(ResUnique rukey);
	protected:
		bool VirTextureItemAlloc(ResUnique rukey, const ImageRawData& image);
		bool VirTextureItemAllocEmpty(ResUnique rukey, const Vector2T<uint32_t>& size);
		bool VirTextureItemFree(ResUnique rukey);

		bool VirTextureExist(ResUnique rukey);
		// bind texture(array), layer(s) => virtual texture.
		// upload uniform param, "uniform_name".
		bool VirTextureItemDraw(ResUnique rukey, PsagShader shader, const VirTextureUniformName& uniform_name);
		// virtual texture & layer_index.
		bool VirTextureItemIndex(ResUnique rukey, PsagTexture& texture, uint32_t layer_index);

		VirTextureParam VirTextureItemGet(ResUnique rukey);

		// virtual texture system: framework oper.
		void VirtualTextureDataObjectCreate(
			Vector2T<uint32_t> base_size, const VirTexturesGenParams& params = VirTexturesGenParams()
		);
		void VirtualTextureDataObjectDelete();
	};
}

namespace GraphicsShaderCode {
	StaticStrLABEL PSAGM_GLENGINE_SHADER_LABEL = "PSAG_GL_GLSL";

	struct PublicDESC {
		std::string ShaderVertTemplate;
		std::string ShaderVertTemplateActor;
		std::string ShaderFragTools;
		std::string ShaderFragHeader;
	};

	struct PrivateDESC {
		std::string ShaderFragMultFilter; // [final_shaders]
		std::string ShaderFragBloomH;     // [final_shaders]
		std::string ShaderFragBloomV;     // [final_shaders]
		std::string ShaderFragFinalPhase; // [final_shaders]
		std::string ShaderFragBackground;
		std::string ShaderFragLight;
		std::string ShaderFragParticle;
		std::string ShaderFragFxSequence;
	};
	// shaders code(glsl_script), not filepath.
	struct GraphicsShadersDESC {
		PublicDESC  PublicShaders;
		PrivateDESC PrivateShaders;
	};

	class GraphicsEngineLayerRes {
	private:
		std::mutex          ResourceMutex   = {};
		GraphicsShadersDESC ResourecShaders = {};
	public:
		// set(config) engine_layer shaders res.
		void Set(const GraphicsShadersDESC& desc);
		// get(copy) engine_layer shaders res.
		GraphicsShadersDESC Get();
	};
	// global resource object.
	extern GraphicsEngineLayerRes GLOBALRES;
}

namespace GraphicsEngineMatrix {
	// matrix_trans params => world matrix calc.
	struct MatrixTransParams {
		Vector2T<float> MatrixPosition;
		Vector2T<float> MatrixScale;
		float           MatrixRotate;

		MatrixTransParams() : 
			MatrixPosition(Vector2T<float>(0.0f, 0.0f)),
			MatrixScale   (Vector2T<float>(1.0f, 1.0f)), 
			MatrixRotate  (0.0f)
		{}
	};

	class PsagGLEngineMatrix :public PsagLow::PsagSupGraphicsLLRES {
	private:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState ProcessUniform = {};
		ResUnique UniformBuffer = {};
		glm::mat4 GetOrthoProjMatrix(float scale_size);
	protected:
		static MatrixTransParams MatrixMainCamera;
		static PsagMatrix4       MatrixDataRect;
		static PsagMatrix4       MatrixDataWindow;

		glm::mat4 UpdateCalcMatrix(const glm::mat4& in_matrix, const MatrixTransParams& params);
		PsagMatrix4 UpdateEncodeMatrix(const glm::mat4& matrix, float scale);

		// framework main_event call update.
		void CreateMatrixUniform();
		void UpdateMatrixUniform(float roatio_value);
	};
}

namespace GraphicsEngineFinal {
	StaticStrLABEL PSAGM_GLENGINE_POST_LABEL = "PSAG_GL_POST";

	struct FinalFxParameters {
		Vector3T<float> GameSceneFilterCOL; // [Lv1]: color_channels.
		float           GameSceneFilterAVG; // [Lv2]: color_avg.

		// bloom_radius: limit: (1,31).
		// bloom_blend: x: game_scene y: blur_scene.
		// rgb color_blend, contrast => blend.
		// vignette: x: radius y: strength.

		uint32_t        GameSceneBloomRadius;
		Vector2T<float> GameSceneBloomBlend;
		float           GameSceneOutContrast; 
		Vector3T<float> GameSceneOutColor;
		Vector2T<float> GameSceneOutVignette;

		Vector2T<float> LightPosition;
		Vector3T<float> LightColor;
		float           LightIntensity;
		float           LightIntensityDecay;
		int32_t         LightSampleStep;

		// light fragment rgb_avg_value collision.
		float LightCollisionValue;
		
		FinalFxParameters() :
			GameSceneFilterCOL  (Vector3T<float>()),
			GameSceneFilterAVG  (0.0f),
			GameSceneBloomRadius(1),
			GameSceneBloomBlend (Vector2T<float>(1.0f, 1.0f)),
			GameSceneOutContrast(1.0f),
			GameSceneOutColor   (Vector3T<float>(1.0f, 1.0f, 1.0f)),
			GameSceneOutVignette(Vector2T<float>(1.0f, 0.0f)),

			LightPosition      (Vector2T<float>(0.0f, 0.0f)),
			LightColor         (Vector3T<float>(1.0f, 1.0f, 1.0f)),
			LightIntensity     (0.0f),
			LightIntensityDecay(0.0f),
			LightCollisionValue(0.08554f),
		    LightSampleStep    (1)
		{}
	};

	class PsagGLEngineFinal :public GraphicsEngineDataset::GLEngineStaticVertexData {
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};
		// shader rendering size, shader_uniform.
		Vector2T<float> RenderingResolution = {};

		glm::mat4 MatrixPorj = {};
		glm::mat4 MatrixView = {};

		// bloom shader hv mvp != scene mvp.
		PsagMatrix4 RenderingMatrixMvp = {};
		ResUnique ShaderProcessFinal = {};

		// scene => volumetric_light.
		ResUnique ShaderVolumLight = {};

		// light => filter => blur(bloom)_h + blur(bloom)_v => final_shader.
		ResUnique ShaderFilter = {}, ShaderBlurXaxis = {}, ShaderBlurYaxis = {};
		
		ResUnique GameSceneFrameBuffer  = {};
		ResUnique GameSceneRenderBuffer = {};

		ResUnique LightFrameBuffer  = {};
		ResUnique FilterFrameBuffer = {};
		// 0:framebuffer_h, 1:framebuffer_v.
		ResUnique BloomFrameBuffers[2] = {};

		// texture_array(5-layers constant):
		// 0: vol_light fx process.
		// 1: color_filter, 2:frame_buffer_tex, 3:blur_h_texture, 4:blur_v_texture.
		ResUnique ProcessTextures = {};
		// topmost, format_z layer: -1.0f
		ResUnique RenderRect = {};

		// vertex default: move,scale.
		void ShaderVertexDefaultParams(PsagShader shader);
		void ShaderRenderingLight();
		void ShaderRenderingBloom();

		FinalFxParameters RenderParameters = {};
	public:
		PsagGLEngineFinal(const Vector2T<uint32_t>& render_resolution);
		~PsagGLEngineFinal();

		// game_main scene => capture => process.
		bool CaptureGameScene(const std::function<bool()>& rendering_func);

		// rendering-event.
		void RenderingPostModule();
		FinalFxParameters* GetRenderParameters() { return &RenderParameters; }
	};
}

namespace GraphicsEngineBackground {
	StaticStrLABEL PSAGM_GLENGINE_BACK_LABEL = "PSAG_GL_BACK";

	struct BackFxParameters {
		// strength x:back_v, y:front_v.
		Vector2T<float> BackgroundStrength;
		Vector4T<float> BackgroundColor;

		Vector2T<float> BackgroundMove;
		Vector2T<float> BackgroundScale;

		// value: 0.0f - 1.0f (多级能见度强度).
		float BackgroundVisibility;

		BackFxParameters() :
			BackgroundColor     (Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f)),
			BackgroundStrength  (Vector2T<float>(1.0f, 1.0f)),
			BackgroundMove      (Vector2T<float>(0.0f, 0.0f)),
			BackgroundScale     (Vector2T<float>(1.0f, 1.0f)),
			BackgroundVisibility(1.0f)
		{}
	};

	class PsagGLEngineBackgroundBase {
	public:
		virtual ~PsagGLEngineBackgroundBase() = default;
		virtual BackFxParameters* GetRenderParameters() = 0;
		virtual void RenderingBackgroundModule() = 0;
	};

	class PsagGLEngineBackground :
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public PsagGLEngineBackgroundBase
	{
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};
		// shader rendering size, shader_uniform.
		Vector2T<float> RenderingResolution = {};
		PsagMatrix4     RenderingMatrixMvp  = {};

		ResUnique ShaderProcessFinal = {};

		float TextureTopLayer = 0.0f;
		// texture_array(n * layers), x:tex_idx[1,n-1], y:tex_idx[n].
		ResUnique BackgroundTextures = {};
		// lowest_layer, fmt: 1.0f
		ResUnique BackgroundRect = {};

		BackFxParameters RenderParameters = {};
	public:
		PsagGLEngineBackground(
			const Vector2T<uint32_t>& render_resolution, const std::vector<ImageRawData>& imgdataset
		);
		~PsagGLEngineBackground() override;
		BackFxParameters* GetRenderParameters() override { return &RenderParameters; }

		// rendering-event.
		void RenderingBackgroundModule() override;
	};

	class PsagGLEngineBackgroundNULL :public PsagGLEngineBackgroundBase {
	protected:
		BackFxParameters RenderParameters = {};
	public:
		~PsagGLEngineBackgroundNULL() override {};
		BackFxParameters* GetRenderParameters() override { return &RenderParameters; }

		void RenderingBackgroundModule() override {};
	};
}

namespace GraphicsEngineParticle {
	StaticStrLABEL PSAGM_GLENGINE_PARTICLE_LABEL = "PSAG_GL_PARTICLE";

	namespace ParticlesGenMode {
		enum EmittersMode {
			PrtcPoints = 1 << 1, // 点云 [扩散]
			PrtcCircle = 1 << 2, // 圆形 [扩散]
			PrtcSquare = 1 << 3, // 矩形 [扩散]
			PrtcPoly   = 1 << 4, // 汇聚 [聚合]
			PrtcDrift  = 1 << 5  // 飘落 [定向]
		};

		enum ColorChannelMode {
			Grayscale   = 1 << 1, // 灰度(单通道R-RGB)
			ChannelsRG  = 1 << 2, // R,G 通道
			ChannelsRB  = 1 << 3, // R,B 通道
			ChannelsGB  = 1 << 4, // G,B 通道
			ChannelsRGB = 1 << 5  // R,G,B 通道
		};
	}

	struct ParticleAttributes {
		Vector3T<float> ParticleVector;
		Vector3T<float> ParticlePosition;
		Vector4T<float> ParticleColor;

		float ParticleScaleSize;
		float ParticleLife;
		bool  ParticleLifeSwitch;

		// particle color(system) channels draw_type mode.
		ParticlesGenMode::ColorChannelMode ParticleModeType;

		constexpr ParticleAttributes() :
			ParticleVector    ({}),
			ParticlePosition  ({}),
			ParticleColor     ({}),
			ParticleScaleSize (1.0f),
			ParticleLife      (0.0f),
			ParticleLifeSwitch(true),
			ParticleModeType  (ParticlesGenMode::Grayscale)
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

		virtual void ConfigCreateMode(ParticlesGenMode::EmittersMode mode) = 0; // 运算模式.
		virtual void ConfigLifeDispersion(Vector2T<float> rand_limit_life) = 0; // 随机生命.
		virtual void ConfigSizeDispersion(Vector2T<float> rand_limit_size) = 0; // 随机大小.

		virtual void ConfigRandomColorSystem(
			Vector2T<float> r, Vector2T<float> g, Vector2T<float> b,
			ParticlesGenMode::ColorChannelMode mode
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

		ParticlesGenMode::EmittersMode ParticlesModeType = {};
		size_t ParticlesNumber = 8;

		bool EnableGrayscale = false;
		Vector3T<Vector2T<float>> RandomColorSystem = {};
		Vector2T<uint32_t>        RandomLimitLife   = {};
		Vector2T<float>           RandomScaleSize   = Vector2T<float>(1.0f, 1.0f);

		Vector3T<float> PositionOffset = {};
		Vector2T<float> RandomLimitPosition = {}, RandomLimitVector = {};

	public:
		bool ConfigCreateNumber(float number) override;
		void ConfigCreateMode(ParticlesGenMode::EmittersMode mode) override;
		void ConfigLifeDispersion(Vector2T<float> rand_limit_life) override;
		void ConfigSizeDispersion(Vector2T<float> rand_limit_size) override;

		// color system(random) channels[0.0,1.0].
		void ConfigRandomColorSystem(
			Vector2T<float> r, Vector2T<float> g, Vector2T<float> b,
			ParticlesGenMode::ColorChannelMode mode = ParticlesGenMode::Grayscale
		) override;
		void ConfigRandomDispersion(
			// randomly distributed parameters.
			Vector2T<float> rand_limit_vector,   // vec random[min,max] pos += vec * speed.
			Vector2T<float> rand_limit_position, // pos random[min,max].
			Vector3T<float> offset_position = Vector3T<float>()
		) override;

		void CreateAddParticleDataset(std::vector<ParticleAttributes>& Data) override;
	};

	enum ParticleCalcMode {
		CALC_DEFAULT  = 1 << 1, // 默认计算模式.
		CALC_PARALLEL = 1 << 2, // 并行计算模式.
		CALC_NO_CALC  = 1 << 3  // 无计算模式.
	};

	class PsagGLEngineParticle :
		public GraphicsEngineDataset::GLEngineDynamicVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData,
		public GraphicsEngineMatrix::PsagGLEngineMatrix,
		public __GRAPHICS_ENGINE_TIMESETP
	{
	private:
		// clac_mode: CALC_PARALLEL => create thread_pool.
		PSAG_THREAD_POOL::PsagThreadTasks* ThreadsParallel = nullptr;
		size_t DataBlockSize = NULL;

		void CalcUpdateParticlesNULL(std::vector<ParticleAttributes>& particles, float speed, float lifesub) {};
		void CalcUpdateParticlesPARA(std::vector<ParticleAttributes>& particles, float speed, float lifesub);
		void CalcUpdateParticles    (std::vector<ParticleAttributes>& particles, float speed, float lifesub);

		std::function<void(std::vector<ParticleAttributes>&, float, float)> UPDATE_CALC_FUNC = {};
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};

		std::vector<ParticleAttributes> DataParticles = {};
		std::vector<float>              DataVertices  = {};

		ResUnique ShaderProcessFinal = {};
		ResUnique DyVertexSysItem   = {};

		float           RenderTimer  = 0.0f;
		Vector2T<float> RenderMove   = {};
		Vector2T<float> RenderScale  = Vector2T<float>(1.0f, 1.0f);
		float           RenderRotate = 0.0f;
		float           RenderTwist  = 0.0f;

		VirTextureUnqiue VirTextureItem = {};
		GraphicsEngineDataset::VirTextureUniformName VirTextureUniform = {};

		void VertexDataConvert(
			const std::vector<ParticleAttributes>& src, std::vector<float>& dst, 
			const Vector2T<float>& center
		);
	public:
		PsagGLEngineParticle(const Vector2T<uint32_t>& render_resolution, const ImageRawData& image = {});
		~PsagGLEngineParticle();

		void ParticleCreate(ParticleGeneratorBase* generator);
		void ParticleClacMode(ParticleCalcMode mode, size_t block_size = 2000, uint32_t threads = 8);

		std::vector<ParticleAttributes>* GetParticleDataset();
		ParticleSystemState				 GetParticleState();

		void SetParticleTwisted    (float value) { RenderTwist  = value > 0.0f ? value : 0.0f; }
		void SetParticleRotateSpeed(float value) { RenderRotate = value > 0.0f ? value : 0.0f; }

		Vector2T<float> ParticlesCoordCenter = {};

		void UpdateParticleData();
		void RenderParticleFX();
	};
}

namespace GraphicsEnginePVFX {
	StaticStrLABEL PSAGM_GLENGINE_PVFX_LABEL = "PSAG_GL_PVFX";

	// image data => image(texture)_view.
	class PsagGLEngineFxImageView :public PsagLow::PsagSupGraphicsLLRES {
	protected:
		PsagTextureView TextureViewItem = {};
	public:
		PsagGLEngineFxImageView(const ImageRawData& image_data);
		~PsagGLEngineFxImageView();

		PsagTexture GetTextureView();
	};

	// captrue => texture_view.
	class PsagGLEngineFxCaptureView :public PsagLow::PsagSupGraphicsLLRES {
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER = {};
		PsagTextureView TextureViewItem = {};
		ResUnique FrameBufferItem = {};

		std::function<void()> BindFrameBufferFunc = [&]() {};
	public:
		PsagGLEngineFxCaptureView(
			const Vector2T<uint32_t>& render_resolution, bool clear_oper = true
		);
		~PsagGLEngineFxCaptureView();

		void CaptureContextBegin();
		void CaptureContextEnd();

		PsagTexture GetCaptureTexView();
	};

	struct SequencePlayer {
		float PlayerSpeedScale;

		float UaxisFrameNumber;
		float VaxisFrameNumber;
	};

	// fx frame_sequence, player.
	class PsagGLEngineFxSequence :
		public GraphicsEngineDataset::GLEngineVirTextureData,
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public __GRAPHICS_ENGINE_TIMESETP
	{
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};
		ResUnique ShaderSequenceRun = {};

		SequencePlayer  PlayerParams   = {};
		Vector2T<float> PlayerPosition = {};
		float           PlayerTimer    = 0.0f;

		PsagMatrix4 RenderMatrix = {};
		float       RenderTimer  = 0.0f;

		ResUnique VirTextureItem = {};
		GraphicsEngineDataset::VirTextureUniformName VirTextureUniform = {};

		// rendering => capture => virtual_texture.
		VirTextureUnqiue CaptureVirTexture = {};
		ResUnique CaptureFrameBuffer = {};
	public:
		PsagGLEngineFxSequence(const ImageRawData& image, const SequencePlayer& params);
		~PsagGLEngineFxSequence();

		bool DrawFxSequence(const Vector4T<float>& blend_color);
		size_t PlayerCyclesCount = NULL;

		// get fx_sequence virtual_texture index.
		ResUnique GetFxSequenceTexture();
	};
}

#endif