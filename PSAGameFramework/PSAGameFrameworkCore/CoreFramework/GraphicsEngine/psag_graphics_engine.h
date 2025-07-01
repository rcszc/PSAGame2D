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
		static ResUnique SystemPresetCircle;

		// dataset thread: vertex(static) resource mutex.
		static std::mutex DatasetResMutex;
	protected:
		// width: +-10.0f, height: +-10.0f
		ResUnique GetPresetRect()   { return SystemPresetRectangle; }
		ResUnique GetPresetCircle() { return SystemPresetCircle;    }

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
		std::string TexParamSampler;  // uniform_type: "sampler2DArray"
		std::string TexParamLayer;    // uniform_type: "int"
		std::string TexParamCropping; // uniform_type: "vec2"
		std::string TexParamSize;     // uniform_type: "vec2"
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
			const Vector2T<uint32_t>& base_size, bool texture_high, bool texture_edge, 
			const VirTexturesGenParams& params = VirTexturesGenParams()
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
		PsagMatrix4 UpdateEncodeMatrix(const glm::mat4& matrix, float NoiseScale);

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
			GameSceneFilterCOL  (Vector3T<float>(0.0f, 0.0f, 0.0f)),
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

		float ParticleScaleSize;
		float ParticleLife;
		bool  ParticleLifeSwitch;

		// particle color(system) channels draw_type mode.
		ColorChannelMode ParticleModeType;

		constexpr ParticleAttributes() :
			ParticleVector    ({}),
			ParticlePosition  ({}),
			ParticleColor     ({}),
			ParticleScaleSize (1.0f),
			ParticleLife      (0.0f),
			ParticleLifeSwitch(true),
			ParticleModeType  (Grayscale)
		{}
	};

	struct ParticleSystemState {
		// real-time status.
		size_t DarwParticlesNumber;
		size_t DarwDatasetBytes;
		
		float BPT_RunFramerate;
		float BPT_BackMemoryUsed; // Mib
	};

	// 粒子生成器(使用均匀分布随机数模型).
	class ParticleGeneratorBase {
	public:
		// particle generation number[n > 8].
		virtual bool ConfigCreateNumber(float number) = 0;

		virtual void ConfigLifeDispersion(const Vector2T<float>& rand_life) = 0; // 随机生命.
		virtual void ConfigSizeDispersion(const Vector2T<float>& rand_size) = 0; // 随机大小.

		virtual void ConfigRandomColorSystem(
			const Vector2T<float>& r, const Vector2T<float>& g, const Vector2T<float>& b,
			ColorChannelMode mode
		) = 0;
		virtual void CreateAddParticleDataset(std::vector<ParticleAttributes>& data) = 0;
	};

	// 源粒子数据生成器 [COPY].
	class GeneratorDataset :public ParticleGeneratorBase {
	public:
		bool ConfigCreateNumber(float number) override { 
			return true; 
		};
		void ConfigLifeDispersion(const Vector2T<float>& rand_life) override {};
		void ConfigSizeDispersion(const Vector2T<float>& rand_size) override {};

		void ConfigRandomColorSystem(
			const Vector2T<float>& r, const Vector2T<float>& g, const Vector2T<float>& b,
			ColorChannelMode mode
		) override {};

		const std::vector<ParticleAttributes>* DataPtr = nullptr;
		void CreateAddParticleDataset(std::vector<ParticleAttributes>& data) {
			// src dataset => particle system.
			data.insert(data.end(), DataPtr->begin(), DataPtr->end());
		}
	};

	// mode => color channels filter.
	Vector3T<Vector2T<float>> __COLOR_SYSTEM_TYPE(
		Vector2T<float> r, Vector2T<float> g, Vector2T<float> b,
		ColorChannelMode mode, bool* gray_switch
	);

	// 点云粒子生成器 [扩散].
	class GeneratorPointsDiffu :public ParticleGeneratorBase {
	protected:
		std::vector<ParticleAttributes> GeneratorCache = {};
		bool EnableGrayscale = false;

		Vector3T<Vector2T<float>> RandomColorSystem = {};
		Vector2T<float>           RandomLimitLife   = {};
		Vector2T<float>           RandomScaleSize   = Vector2T<float>(1.0f, 1.0f);

		Vector3T<float> OffsetPosition = {};
		Vector2T<float> RandomPosition = {};
		Vector2T<float> RandomSpeed    = {};

		size_t ParticlesNumber = 8;
	public:
		bool ConfigCreateNumber(float number) override;

		void ConfigLifeDispersion(const Vector2T<float>& rand_life) override;
		void ConfigSizeDispersion(const Vector2T<float>& rand_size) override;

		void ConfigGenPos    (const Vector2T<float>& position);
		void ConfigGenPosRand(const Vector2T<float>& position_rand);
		void ConfigGenVector (const Vector2T<float>& speed);

		void ConfigRandomColorSystem(
			const Vector2T<float>& r, const Vector2T<float>& g, const Vector2T<float>& b,
			ColorChannelMode mode
		) override;

	    // particle_system call
		// file: psag_graphics_engine_particle.cpp
		void CreateAddParticleDataset(std::vector<ParticleAttributes>& data) override;
	};

	// 形状粒子生成器 [扩散]. [圆形, 矩形]
	class GeneratorShape :public ParticleGeneratorBase {
	protected:
		std::vector<ParticleAttributes> GeneratorCache = {};
		bool EnableGrayscale = false;

		Vector3T<Vector2T<float>> RandomColorSystem = {};
		Vector2T<float>           RandomLimitLife   = {};
		Vector2T<float>           RandomScaleSize   = Vector2T<float>(1.0f, 1.0f);

		Vector3T<float> OffsetPosition = {};
		Vector2T<float> RandomPosition = {};
		Vector2T<float> RandomSpeed    = {};

		size_t   ParticlesNumber = 8;
		uint32_t ParticlesMode   = 0;
	public:
		bool ConfigCreateNumber(float number) override;

		void ConfigLifeDispersion(const Vector2T<float>& rand_life) override;
		void ConfigSizeDispersion(const Vector2T<float>& rand_size) override;

		void ConfigGenPos    (const Vector2T<float>& position);
		void ConfigGenPosRand(const Vector2T<float>& position_rand);
		void ConfigGenVector (const Vector2T<float>& speed);

		// mode_code: 1: circle, 2: square(rect).
		void SettingShape(uint32_t mode) { ParticlesMode = mode; }

		void ConfigRandomColorSystem(
			const Vector2T<float>& r, const Vector2T<float>& g, const Vector2T<float>& b,
			ColorChannelMode mode
		) override;

		// particle_system call
		// file: psag_graphics_engine_particle.cpp
		void CreateAddParticleDataset( std::vector<ParticleAttributes>& data) override;
	};

	// 飘落氛围粒子生成器 [向下].
	class GeneratorDriftDown :public ParticleGeneratorBase {
	protected:
		std::vector<ParticleAttributes> GeneratorCache = {};
		bool EnableGrayscale = false;

		Vector3T<Vector2T<float>> RandomColorSystem = {};
		Vector2T<float>           RandomLimitLife = {};
		Vector2T<float>           RandomScaleSize = Vector2T<float>(1.0f, 1.0f);

		Vector3T<float> OffsetPosition = {};
		float ParticleWidth = 0.0f, ParticleSpeed = 1.0f;

		size_t ParticlesNumber = 8;
	public:
		bool ConfigCreateNumber(float number) override;

		void ConfigLifeDispersion(const Vector2T<float>& rand_life) override;
		void ConfigSizeDispersion(const Vector2T<float>& rand_size) override;

		void ConfigGenPos(const Vector2T<float>& position);
		void ConfigGenWidthSpeed(float width, float drift_speed) {
			ParticleWidth = width;
			ParticleSpeed = drift_speed;
		}
		void ConfigRandomColorSystem(
			const Vector2T<float>& r, const Vector2T<float>& g, const Vector2T<float>& b,
			ColorChannelMode mode
		) override;

		// particle_system call
		// file: psag_graphics_engine_particle.cpp
		void CreateAddParticleDataset(std::vector<ParticleAttributes>& data) override;
	};

	class PsagGLEngineThread {
	private:
		std::vector<ParticleAttributes> ParticlesData = {};
		std::mutex ParticlesDataMutex = {};
		std::atomic<size_t> ParticlesDataSize = {};

		std::atomic<bool> ParticleTaskProcFlag = {};
		std::thread*      ParticleTaskProc     = {};

		// thread_safe particle gen_dataset queue.
		std::queue<std::vector<ParticleAttributes>> GenParticle = {};
		std::mutex GenParticleMutex = {};

		// thread_safe double buffer swap data.
		// status: true: proc thread write. false: draw thread read.
		std::atomic<bool>  VertexDataReadStatus   = false;
		std::atomic<bool>  VertexDataDBufferIndex = false;
		std::vector<float> VertexDataDBuffer[2]   = {};

		void ParticlesCalcuate(
			std::vector<ParticleAttributes>* particles, double timestep
		);
		void ParticlesConvertData(
			const std::vector<ParticleAttributes>& src, std::vector<float>* dst,
			const Vector2T<float>& center
		);
		std::chrono::system_clock::time_point FramerateTimer 
			= std::chrono::system_clock::now();
		double FramerateStep = 0.0;
		void CalcThisContextFramerateStep(double base_fps);
	protected:
		std::atomic<float> CoordCenterX = 0.0f, CoordCenterY = 0.0f;
		// generate particle entities data.
		void PPTS_ParticleGeneratorTask(ParticleGeneratorBase* ptr);
		void PPTS_SetDataReadStatus();

		// get format vertex data.
		const std::vector<float>* PPTS_GetVertexData() const;
		// 0: entities number, 1: buffer bytes 2: 1000x fps, 3: mem usage.
		std::array<size_t, 4> PPTS_GetBackRunParams();
		// warn: copy entities data.
		std::vector<ParticleAttributes> PPTS_GetCurrentEntities();
	public:
		PsagGLEngineThread();
		~PsagGLEngineThread();
	};

	// UPDATE: 2025_02_23 RCSZ. [128 ENT(S)]
#define PSAG_PARTICLE_SYSTEM_LIMIT 128
	// particle system and back proc thread.
	class PsagGLEngineParticle : public PsagGLEngineThread,
		public __GRAPHICS_ENGINE_TIMESETP,
		public GraphicsEngineDataset::GLEngineDynamicVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData,
		public GraphicsEngineMatrix::PsagGLEngineMatrix
	{
	private:
		static size_t ParticleSystemCount;
		bool ParticleSystemValid = false;
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};

		ResUnique ShaderProcessFinal = {};
		ResUnique DyVertexSysItem    = {};

		float           RenderTimer = 0.0f;
		Vector2T<float> RenderMove  = {};
		Vector2T<float> RenderScale = Vector2T<float>(1.0f, 1.0f);
		float           RenderAngle = 0.0f;
		float           RenderTwist = 0.0f;

		VirTextureUnique VirTextureItem = {};
		GraphicsEngineDataset::VirTextureUniformName VirTextureUniform = {};

		ParticleSystemState SystemStateTemp = {};
		std::chrono::system_clock::time_point BackInfoSampleTimer
			= std::chrono::system_clock::now();

		void ParticleSystemSample(int64_t sample_time);
	public:
		PsagGLEngineParticle(const Vector2T<uint32_t>& render_size, const ImageRawData& image = {});
		~PsagGLEngineParticle();

		void ParticleCreate(ParticleGeneratorBase* generator);

		std::vector<ParticleAttributes> GetParticleDataCopy();
		ParticleSystemState				GetParticleState() const;

		void SetParticleTwisted    (float value) { RenderTwist = value > 0.0f ? value : 0.0f; }
		void SetParticleRotateSpeed(float value) { RenderAngle = value > 0.0f ? value : 0.0f; }
		// shader calcuate coord center.
		void SetParticleCenter(const Vector2T<float>& value) { 
			CoordCenterX = value.vector_x; CoordCenterY = value.vector_y;
		}
		void SetParticleScale(const Vector2T<float>& value) {
			RenderScale = value;
		}
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
		VirTextureUnique CaptureVirTexture = {};
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