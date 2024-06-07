// psag_graphics_engine_system. RCSZ. [middle_level_engine]

#ifndef __PSAG_GRAPHICS_ENGINE_H
#define __PSAG_GRAPHICS_ENGINE_H

#define ENABLE_LOWMODULE_GRAPHICS
#include "../psag_lowlevel_support.h"

#define PSAGM_VIR_TICKSTEP_GL 0.058f

// 统一数据集管理,单一共享 VAO,VBO,TEXTURE(TMU).
namespace GraphicsEngineDataset {
	StaticStrLABEL PSAGM_GLENGINE_DATA_LABEL = "PSAG_GL_DATASET";

	struct VABO_DATASET_INFO {
		size_t DatasetLength;
		size_t DatasetOffsetLength;

		VABO_DATASET_INFO() : DatasetLength(NULL), DatasetOffsetLength(NULL) {}
	};

	// LLRES => stc_vertex_data system. (static vertex data)
	// => MODULE(update: 20240506):
	// derive class:
	// "GraphicsEnginePost::PsagGLEnginePost"
	// "GraphicsEngineBackground::PsagGLEngineBackground"
	class GLEngineStcVertexData :public PsagLow::PsagSupGraphicsLLRES {
	private:
		static PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender;
		// opengl vao,vbo handle(res).
		static ResUnique VertexAttribute;
		static ResUnique VertexBuffer;

		static std::unordered_map<ResUnique, VABO_DATASET_INFO> IndexItems;
		static ResUnique SystemPresetRectangle;
	protected:
		// width: +-10.0f, height: +-10.0f
		ResUnique GetPresetRect() { return SystemPresetRectangle; }

		bool VerStcDataItemAlloc(ResUnique rukey, const std::vector<float>& data);
		bool VerStcDataItemFree(ResUnique rukey);

		bool VerStcOperFrameDraw(ResUnique rukey);

		// static vertex system: framework oper.
		void StaticVertexDataObjectCreate();
		void StaticVertexDataObjectDelete();
	};

	// LLRES => dy_vertex_data system. (dynamic vertex data)
	// => MODULE(update: 20240505):
	// derive class:
	// "GraphicsEngineParticle::PsagGLEngineParticle"
	class GLEngineDyVertexData :public PsagLow::PsagSupGraphicsLLRES {
	private:
		static PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender;
		// opengl vao,vbo handle(res).
		static ResUnique VertexAttribute;
		static ResUnique VertexBuffer;

		static std::unordered_map<ResUnique, VABO_DATASET_INFO> IndexItems;
		static std::vector<float> VertexRawDataset;

		static bool GLOBAL_UPDATE_FLAG;
		void UpdateVertexDyDataset();
	protected:
		bool VerDyDataItemAlloc(ResUnique rukey);
		bool VerDyDataItemFree(ResUnique rukey);

		bool VerDyOperFramePushData(ResUnique rukey, const std::vector<float>& data);
		bool VerDyOperFrameDraw(ResUnique rukey);

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

		ResUnique TextureArrayIndex;
		TextureLayerAlloc::TEX_LAYER_ALLOC* LayerAllotter;
	};
	// virtual texture(item) params: cropping,size.
	struct VirTextureParam {
		// 1:1/1, 2:1/2, 3:1/4, 4:1/8.
		uint32_t ResolutionType;
		ResUnique Texture;

		uint32_t        SampleLayers;   // simpler-z offset
		Vector2T<float> SampleCropping; // x,y:[0.0f,1.0f]
		Vector2T<float> SampleSize;     // x,y:[image_size]

		VirTextureParam() : ResolutionType(NULL), Texture(NULL), SampleLayers(NULL), SampleCropping({}), SampleSize({}) {};
		VirTextureParam(uint32_t type, ResUnique rukey, uint32_t layer, const Vector2T<float>& smpcro, const Vector2T<float>& smpsz) :
			ResolutionType(type), Texture(rukey), SampleLayers(layer), SampleCropping(smpcro), SampleSize(smpsz)
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
		static PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender;
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
		std::string ShaderFragTools;
		std::string ShaderFragHeader;
	};
	struct PrivateDESC {
		std::string shaderFragColorFilter; // [post_sh_group]
		std::string ShaderFragBloomH;      // [post_sh_group]
		std::string ShaderFragBloomV;      // [post_sh_group]
		std::string ShaderFragFinalPhase;  // [post_sh_group]
		std::string ShaderFragBackground;
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

class __GRAPHICS_ENGINE_TIMESETP {
protected:
	static float GraphicsEngineTimeStep;
};

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

	class PsagGLEngineMatrix {
	private:
		glm::mat4 GetOrthoProjMatrix(float scale_size);
	protected:
		static MatrixTransParams MatrixWorldCamera;
		static PsagMatrix4       MatrixDataRect;
		static PsagMatrix4       MatrixDataWindow;

		glm::mat4 UpdateCalcMatrix(const glm::mat4& in_matrix, const MatrixTransParams& params);
		PsagMatrix4 UpdateEncodeMatrix(const glm::mat4& matrix, float scale);
	};
}

namespace GraphicsEnginePost {
	StaticStrLABEL PSAGM_GLENGINE_POST_LABEL = "PSAG_GL_POST";

	struct PostFxParameters {
		Vector3T<float> GameSceneFilterCOL; // lv1-filter.
		float           GameSceneFilterAVG; // lv2-filter.
		// bloom_radius [1-32] calc & filter avg_color.
		uint32_t GameSceneBloomRadius;
		// bloom_blend x:source, y:blur.
		Vector2T<float> GameSceneBloomBlend;
		
		PostFxParameters() :
			GameSceneFilterCOL  (Vector3T<float>()),
			GameSceneFilterAVG  (0.0f),
			GameSceneBloomRadius(1), 
			GameSceneBloomBlend (Vector2T<float>(1.0f, 1.0f))
		{}
	};

	class PsagGLEnginePost :public GraphicsEngineDataset::GLEngineStcVertexData {
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};
		// shader rendering size, shader_uniform.
		Vector2T<float> RenderingResolution = {};

		glm::mat4 MatrixPorj = {};
		glm::mat4 MatrixView = {};

		// bloom shader hv mvp != scene mvp.
		PsagMatrix4 RenderingMatrixMvp = {};
		// scene => filter => bloom_h + bloom_v => post_shader.
		ResUnique ShaderProgramItem = {};
		ResUnique ShaderFilter = {}, ShaderBloomH = {}, ShaderBloomV = {};
		
		ResUnique GameSceneFrameBuffer = {};
		ResUnique FilterFrameBuffer    = {};
		// 0:framebuffer_h, 1:framebuffer_v.
		ResUnique BloomFrameBuffers[2] = {};

		// texture_cube: 0: color_filter, 1:frame_buffer_tex, 2:bloom_h_tex, 3:bloom_v_tex
		ResUnique ProcessTextures = {};

		// vertex default: move,scale.
		void ShaderVertexDefaultParams(PsagShader shader);
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
	StaticStrLABEL PSAGM_GLENGINE_BACK_LABEL = "PSAG_GL_BACK";

	struct BackFxParameters {
		// value[0.0,1.0]
		float BackgroundVisibility;
		// strength x:back_v, y:front_v.
		Vector2T<float> BackgroundStrength;
		Vector4T<float> BackgroundColor;

		Vector2T<float> BackgroundMove;
		Vector2T<float> BackgroundScale;

		BackFxParameters() :
			BackgroundVisibility (1.0f), 
			BackgroundColor      (Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f)),
			BackgroundStrength   (Vector2T<float>(1.0f, 1.0f)),
			BackgroundMove       (Vector2T<float>(0.0f, 0.0f)),
			BackgroundScale      (Vector2T<float>(1.0f, 1.0f))
		{}
	};

	class PsagGLEngineBackground :public GraphicsEngineDataset::GLEngineStcVertexData {
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};
		// shader rendering size, shader_uniform.
		Vector2T<float> RenderingResolution = {};
		PsagMatrix4     RenderingMatrixMvp  = {};

		// x:tex_idx[1,n-1], y:tex_idx[n].
		ResUnique ShaderProgramItem = {};

		float TextureTopLayer = {};
		// texture_array(n * layers).
		ResUnique BackgroundTextures = {};
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
	StaticStrLABEL PSAGM_GLENGINE_PARTICLE_LABEL = "PSAG_GL_PARTICLE";

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

		float ParticleScaleSize;
		float ParticleLife;
		ColorChannelMode ParticleModeType;

		constexpr ParticleAttributes() :
			ParticleVector   ({}),
			ParticlePosition ({}),
			ParticleColor    ({}),
			ParticleScaleSize(1.0f),
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
		virtual void ConfigSizeDispersion(Vector2T<float> rand_limit_size) = 0; // 随机大小.

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
		Vector2T<float>           RandomScaleSize   = Vector2T<float>(1.0f, 1.0f);

		Vector3T<float> PositionOffset = {};
		Vector2T<float> RandomLimitPosition = {}, RandomLimitVector = {};

	public:
		bool ConfigCreateNumber(float number) override;
		void ConfigCreateMode(EmittersMode mode) override;
		void ConfigLifeDispersion(Vector2T<float> rand_limit_life) override;
		void ConfigSizeDispersion(Vector2T<float> rand_limit_size) override;

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
		public GraphicsEngineDataset::GLEngineSmpTextureData,
		public GraphicsEngineMatrix::PsagGLEngineMatrix,
		public __GRAPHICS_ENGINE_TIMESETP
	{
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};

		std::vector<ParticleAttributes> DataParticles = {};
		std::vector<float>              DataVertices  = {};

		ResUnique ShaderProgramItem = {};
		ResUnique DyVertexSysItem   = {};

		float           RenderTimer   = 0.0f;
		Vector2T<float> RenderMove    = {};
		Vector2T<float> RenderScale   = Vector2T<float>(1.0f, 1.0f);
		float           RenderTwist = 0.0f;

		ResUnique VirTextureItem = {};
		GraphicsEngineDataset::VirTextureUniformName VirTextureUniform = {};

		void CalcUpdateParticles(std::vector<ParticleAttributes>& particles, float speed, float lifesub);
		void VertexDataConvert(const std::vector<ParticleAttributes>& src, std::vector<float>& cvt);
	public:
		PsagGLEngineParticle(const Vector2T<uint32_t>& render_resolution, const ImageRawData& image = {});
		~PsagGLEngineParticle();

		void ParticleCreate(ParticleGeneratorBase* generator);

		ParticleSystemState				 GetParticleState();
		std::vector<ParticleAttributes>* GetParticleDataset();

		void SetParticleTwisted(float value) {
			RenderTwist = value;
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
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender = {};
		PsagTextureView TextureViewItem = {};
		ResUnique FrameBufferItem = {};
	public:
		PsagGLEngineFxCaptureView(const Vector2T<uint32_t>& render_resolution);
		~PsagGLEngineFxCaptureView();

		void CaptureContextBind();
		void CaptureContextUnBind();

		PsagTexture GetCaptureTexView();
	};

	struct SequencePlayer {
		float PlayerSpeedScale;

		float UaxisFrameNumber;
		float VaxisFrameNumber;
	};
	// 特效序列贴图.
	class PsagGLEngineFxSequence :
		public GraphicsEngineDataset::GLEngineSmpTextureData,
		public GraphicsEngineDataset::GLEngineStcVertexData,
		public __GRAPHICS_ENGINE_TIMESETP
	{
	protected:
		static float SystemTimeStep;

		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper ShaderRender = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};

		SequencePlayer  PlayerParams   = {};
		Vector2T<float> PlayerPosition = {};
		float           PlayerTimer    = 0.0f;

		ResUnique   ShaderProgramItem = {};
		float       RenderTimer       = 0.0f;
		PsagMatrix4 RenderMatrix      = {};

		ResUnique VirTextureItem = {};
		GraphicsEngineDataset::VirTextureUniformName VirTextureUniform = {};
	public:
		PsagGLEngineFxSequence(const ImageRawData& image, const SequencePlayer& params);
		~PsagGLEngineFxSequence();

		bool DrawFxSequence(const Vector4T<float>& blend_color);
		uint32_t PlayerCyclesCount = NULL;
	};
}

#endif