// psag_actor_module. RCSZ. [middle_level_module]

#ifndef __PSAG_ACTOR_MODULE_HPP
#define __PSAG_ACTOR_MODULE_HPP
// graphics & physics engine => actor_module.
#include "../CoreFramework/GraphicsEngine/psag_graphics_engine.h"
#include "../CoreFramework/PhysicsEngine/psag_physics_engine.h"

class __ACTOR_MODULES_TIMESTEP {
protected:
	static float ActorModulesTimeStep;
};

namespace GamePhysicsOper {
	// name, mode, params = {}
	// mode: 1:create, 2:delete params: x:gravity_vec_x, y:gravity_vec_y
	class GamePhysicalWorld :public PhysicsEngine::PhyEngineCoreDataset {
	public:
		GamePhysicalWorld(std::string name_key, int mode, Vector2T<float> params = {}) {
			if (mode == 1) OperFlag = PhysicsWorldCreate(name_key, params);
			if (mode == 2) OperFlag = PhysicsWorldDelete(name_key);
		}
		bool OperFlag = false;
	};
}

namespace GameActorCore {
	class GameActorExecutor;
}
// basic_components: 
// comp_space_trans, comp_health_trans, comp_rendering
namespace GameComponents {
	StaticStrLABEL PSAGM_ACTOR_COMP_LABEL = "PSAG_ACTOR_COMPS";
	using Actor = GameActorCore::GameActorExecutor;

	// actor information global_code & type_code.
	struct ActorPrivateINFO {
		size_t   ActorUniqueCode;
		uint32_t ActorTypeCode;
	};

	class ActorActionLogicBase {
	public:
		virtual void ActionLogicRun(Actor* actor_object, float timestep) = 0;
	};

	class ActorActionLogic :public __ACTOR_MODULES_TIMESTEP {
	protected:
		bool ActionLogicNULL = false;
		ActorActionLogicBase* ActionLogicObject = nullptr;

		// ENABLE: CPP_RTTI.
		std::string ObjectName = {};
		size_t ObjectHashCode = NULL;
	public:
		ActorActionLogic(ActorActionLogicBase* pointer);
		// warning: default: nullptr!
		ActorActionLogic() : ActionLogicNULL(true) {}
		~ActorActionLogic();

		virtual void UpdateActorActionLogic(Actor* actor_this);
	};
    
	class ActorSpaceTrans :
		public PhysicsEngine::PhyEngineCoreDataset,
		public __ACTOR_MODULES_TIMESTEP
	{
	protected:
		std::function<void(Vector2T<float>&, float&)> ActorTransFunc = {};
		// world index, body(item) index.
		std::string PhysicsWorld;
		PhyBodyKey  PhysicsBody;

		void UpdateActorTransVelocity(Vector2T<float>& position, float& rotate);
		void UpdateActorTransForce(Vector2T<float>& position, float& rotate);
	public:
		ActorSpaceTrans(const std::string& phy_world, PhyBodyKey phy_body, bool enable_force = false);

		Vector2T<float> ActorTransMoveValue = {};
		Vector2T<float> ActorStateMoveSpeed = {};

		float ActorTransRotateValue = 0.0f;
		float ActorStateRotateSpeed = 0.0f;

		virtual void UpdateActorTrans(Vector2T<float>& position, float& rotate);
		virtual void SetActorState(const Vector2T<float>& pos, float angle);
	};

	// game actor_hp: [state,speed] calc: h += (s - h) * speed.
	struct GameActorHP {
		float HealthSTATE = 0.0f;
		float HealthSPEED = 0.0f;
		float HealthMAX   = 0.0f;

		GameActorHP(float state, float speed, float max) : 
			HealthSTATE(state), HealthSPEED(speed), HealthMAX(max)
		{}
	};
	class ActorHealthTrans : public __ACTOR_MODULES_TIMESTEP {
	public:
		ActorHealthTrans() {};
		// actor hp_state system. state: set, speed out: lerp.
		std::vector<GameActorHP> ActorHealthState    = {};
		std::vector<float>       ActorHealthStateOut = {};

		virtual void UpdateActorHealthTrans();
		virtual void SetActorHealth(size_t count, float value);
	};

	struct RenderingParams {
		// position: x,y. scale: w,h. rotate: angle. layer: z.
		Vector2T<float> RenderPosition;
		Vector2T<float> RenderScale;
		float           RenderRotate;
		float           RenderLayerHeight;

		// render color: uniform"FxColor". [20240825]
		Vector4T<float> RenderColorBlend = Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f);

		RenderingParams() : RenderPosition({}), RenderScale({}), RenderRotate(0.0f), RenderLayerHeight(-1.0f) {}
		RenderingParams(const Vector2T<float>& rp, const Vector2T<float>& rs, float rr, float rz, const Vector4T<float>& rc) :
			RenderPosition(rp), RenderScale(rs), RenderRotate(rr), RenderLayerHeight(rz), RenderColorBlend(rc)
		{}
	};

	class ActorRendering :
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData
	{
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER   = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform                  ShaderUniform = {};
	public:
		ResUnique ShaderIndex      = NULL;
		ResUnique VertexGroupIndex = NULL;

		Vector2T<float> RenderResolution = {};
		
		std::function<void(PsagShader)> RenderingTextureNFunc = [&](PsagShader) {};
		std::function<void(PsagShader)> RenderingTextureHFunc = [&](PsagShader) {};

		GraphicsEngineDataset::VirTextureUniformName VirTexUniform    = {};
		GraphicsEngineDataset::VirTextureUniformName VirTexUniformHDR = {};

		VirTextureUnqiue VirTexture    = NULL;
		VirTextureUnqiue VirTextureHDR = NULL;

		virtual void UpdateActorRendering(const RenderingParams& params, float time_count);
		virtual void UpdateActorRenderingTextureN(PsagShader shader);
		virtual void UpdateActorRenderingTextureH(PsagShader shader);
	};

	// actor coordinate_convert tool(s).
	class ActorCoordConvert :public GraphicsEngineMatrix::PsagGLEngineMatrix {
	public:
		Vector2T<float> ConvertSceneToWindow(Vector2T<uint32_t> window_size, Vector2T<float> position);
	};

	namespace null {
		// actor_action_logic components: [NULL-OBJ].
		class ActorActionLogicNULL :public ActorActionLogic {
		public:
			ActorActionLogicNULL() {};
			void UpdateActorActionLogic(Actor* actor_this) override {};
		};
		// actor_space_trans components: [NULL-OBJ].
		class ActorSpaceTransNULL :public ActorSpaceTrans {
		public:
			ActorSpaceTransNULL(const std::string& phy_world, PhyBodyKey phy_body) : ActorSpaceTrans(phy_world, phy_body) {}
			void UpdateActorTrans(Vector2T<float>& position, float& rotate) override {};
			void SetActorState(const Vector2T<float>& pos, float angle) override {};
		};
		// actor_health_trans components: [NULL-OBJ].
		class ActorHealthTransNULL : public ActorHealthTrans {
		public:
			ActorHealthTransNULL() {};
			void UpdateActorHealthTrans() override {};
		};
		// actor_rendering components: [NULL-OBJ].
		class ActorRenderingNULL : public ActorRendering {
		public:
			void UpdateActorRendering(const RenderingParams& params, float time_count) override {};

			void UpdateActorRenderingTextureN(PsagShader shader) override {};
			void UpdateActorRenderingTextureH(PsagShader shader) override {};
		};
	}
}

namespace GameActorScript {
	extern const char* psag_shader_public_frag_header;
	extern const char* psag_shader_public_frag_texnor;
	extern const char* psag_shader_public_frag_texhdr;

	extern const char* psag_shader_private_frag_brick_nor;
	extern const char* psag_shader_private_frag_brick_hdr;
}

namespace GameActorCore {
	StaticStrLABEL PSAGM_ACTOR_CORE_LABEL = "PSAG_ACTOR_CORE";
	namespace Type {
		// actor null_type value.
		constexpr uint32_t ActorTypeNULL = 0;

		class GameActorTypeBind {
		protected:
			std::unordered_map<std::string, uint32_t> ActorTypeINFO = {};
			uint32_t ActorTypeCount = ActorTypeNULL;
		public:
			uint32_t ActorTypeIs(const std::string& type_name);

			void ActorTypeCreate(const std::string& type_name);
			void ActorTypeDelete(const std::string& type_name);
		};
		// [GLOBAL.OBJECT] Actor类型分配器.
		extern GameActorTypeBind ActorTypeAllotter;
	}
	class GameActorExecutor;

	using SScript = const char*;
	// game_actor render system_preset shader script.
	class GameActorPresetScript {
	public:
		SScript TmpScriptBrickImage(bool HDR = false) {
			// shader "main" code nor / hdr. 
			if (HDR) return GameActorScript::psag_shader_private_frag_brick_hdr;
			return GameActorScript::psag_shader_private_frag_brick_nor;
		};
	};

	struct GameActorShaderVerticesDESC {
		Vector4T<float> ShaderDefaultColor;

		// shader ver: coord & collision.
		std::vector<Vector2T<float>> ShaderVertexCollision;
		std::vector<Vector2T<float>> ShaderVertexUvCoord;

		// false: use system_default vert_shader.
		bool        VertexShaderEnable;
		std::string VertexShaderScript;

		GameActorShaderVerticesDESC() :
			ShaderDefaultColor   (Vector4T<float>(0.0f, 0.2f, 0.2f, 0.92f)),
			ShaderVertexCollision({}),
			ShaderVertexUvCoord  ({}),
			VertexShaderEnable   (false),
			VertexShaderScript   ({})
		{}
	};

	class GameActorShader :
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData
	{
	private:
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform U_LOADER = {};
		PsagShader S_HANDLE = OPENGL_INVALID_HANDEL;
	protected:
		// 系统预设 [NOR] Virtual Texture Uniform 参数组.
		GraphicsEngineDataset::VirTextureUniformName SystemPresetUnameN() {
			GraphicsEngineDataset::VirTextureUniformName U_NAME = {};
			// preset shader uniform name.
			U_NAME.TexParamSampler  = "VirTextureNOR";
			U_NAME.TexParamLayer    = "VirTextureNORLayer";
			U_NAME.TexParamCropping = "VirTextureNORCropping";
			U_NAME.TexParamSize     = "VirTextureNORSize";
			return U_NAME;
		}
		// 系统预设 [HDR] Virtual Texture Uniform 参数组.
		GraphicsEngineDataset::VirTextureUniformName SystemPresetUnameH() {
			GraphicsEngineDataset::VirTextureUniformName U_NAME = {};
			// preset shader uniform name.
			U_NAME.TexParamSampler  = "VirTextureHDR";
			U_NAME.TexParamLayer    = "VirTextureHDRLayer";
			U_NAME.TexParamCropping = "VirTextureHDRCropping";
			U_NAME.TexParamSize     = "VirTextureHDRSize";
			return U_NAME;
		}
		bool CheckRepeatTex(VirTextureUnqiue virtex);
		bool MappingTextureFlag = false;

		Vector4T<float> ShaderDefaultColor = {};
		// x:vert_shader, y:frag_shader.
		Vector2T<std::string> ShaderScript = {};

		std::vector<Vector2T<float>>* VerticesPosition = nullptr;
		std::vector<Vector2T<float>>* VerticesUvCoord  = nullptr;

		bool ShaderImageTextureLoad(VirTextureUnqiue* ref_texture, const ImageRawData& image);
	public:
		GameActorShader(const std::string& SHADER_FRAG, const Vector2T<uint32_t>& RESOLUTION);
		~GameActorShader();
		// create actor shader_resource.
		bool CreateShaderResource(bool default_is_circle = false);

		// load vertices(pos,uv) resource. (warn: ref)
		bool ShaderVerticesLoad(GameActorShaderVerticesDESC& VER_DESC);

		// referencing virtual texture(non-delete). base + hdr_blend. rgb color add.
		bool ShaderLoadVirTexture   (VirTextureUnqiue virtex);
		bool ShaderLoadVirTextureHDR(VirTextureUnqiue virtex);

		// create virtual texture. base + hdr_blend. rgb color add.
		bool ShaderImageLoad   (const ImageRawData& image);
		bool ShaderImageLoadHDR(const ImageRawData& image);

		// => uniform context (shader_context) => set_uniform.
		void UniformSetContext(std::function<void()> context_func);

		void UniformMatrix3x3(const char* name, const PsagMatrix3& matrix);
		void UniformMatrix4x4(const char* name, const PsagMatrix4& matrix);

		void UniformInt32(const char* name, const int32_t& value);
		void UniformFP32 (const char* name, const float& value);

		void UniformVec2(const char* name, const Vector2T<float>& value);
		void UniformVec3(const char* name, const Vector3T<float>& value);
		void UniformVec4(const char* name, const Vector4T<float>& value);

		// ================================ system params ================================

		VirTextureUnqiue __VIR_TEXTURE_ITEM     = NULL;
		VirTextureUnqiue __VIR_TEXTURE_HDR_ITEM = NULL;

		GraphicsEngineDataset::VirTextureUniformName __VIR_UNIFORM_ITEM     = {};
		GraphicsEngineDataset::VirTextureUniformName __VIR_UNIFORM_HDR_ITEM = {};
		
		ResUnique __ACTOR_SHADER_ITEM = NULL;
		ResUnique __ACTOR_VERTEX_ITEM = NULL;

		Vector2T<uint32_t> __RENDER_RESOLUTION = {};

		std::vector<Vector2T<float>>* __GET_VERTICES_RES() {
			return VerticesPosition;
		}
	};

	struct GameActorHealthDESC {
		std::vector<GameComponents::GameActorHP> InitialActorHealth;
	};

	struct GameCollisionPAIR {
		GameComponents::ActorPrivateINFO ThisActor = {};
		GameComponents::ActorPrivateINFO ThatActor = {};

		GameCollisionPAIR(const GameComponents::ActorPrivateINFO& a_this, const GameComponents::ActorPrivateINFO& a_that) :
			ThisActor(a_this), ThatActor(a_that)
		{}
	};

	enum ActorComponentFlags : uint32_t {
		ActorEnableLogic     = 1 << 1,
		ActorEnableTransform = 1 << 2,
		ActorEnableHealth    = 1 << 3,
		ActorEnableRender    = 1 << 4,
		ActorEnableCollision = 1 << 5
	};
	ActorComponentFlags  operator| (ActorComponentFlags  a, ActorComponentFlags b);
	ActorComponentFlags  operator& (ActorComponentFlags  a, ActorComponentFlags b);
	ActorComponentFlags& operator|=(ActorComponentFlags& a, ActorComponentFlags b);

	// box2d collision filter 16bit => 1 - 32768. [20241110]
	enum ActorCollisionGroup : uint16_t {
		ActorPhyGroup0 = 1 << 0, ActorPhyGroup1 = 1 << 1,
		ActorPhyGroup2 = 1 << 2, ActorPhyGroup3 = 1 << 3,
		ActorPhyGroup4 = 1 << 4, ActorPhyGroup5 = 1 << 5,
		ActorPhyGroup6 = 1 << 6, ActorPhyGroup7 = 1 << 7,
		ActorPhyGroup8 = 1 << 8, ActorPhyGroup9 = 1 << 9,

		ActorPhyGroup10 = 1 << 10, ActorPhyGroup11 = 1 << 11,
		ActorPhyGroup12 = 1 << 12, ActorPhyGroup13 = 1 << 13,
		ActorPhyGroup14 = 1 << 14, ActorPhyGroup15 = 1 << 15,

		ActorPhyGroupALL = 0b1111'1111'1111'1111
	};
	ActorCollisionGroup  operator| (ActorCollisionGroup  a, ActorCollisionGroup b);
	ActorCollisionGroup& operator|=(ActorCollisionGroup& a, ActorCollisionGroup b);

	enum ActorPhyMode {
		ActorPhysicsFixed = 1 << 1,
		ActorPhysicsMove  = 1 << 2
	};
	// init(config) descriptor.
	struct GameActorExecutorDESC {
		GameActorShader* ActorShaderResource;
		std::string      ActorPhysicsWorld;
		ActorPhyMode     ActorPhysicalMode;

		bool VectorCalcIsForce;
		// collision box preset circle(polygon: 20).
		bool CollisionBoxIsCircle;

		// params: x:phy_density, y:phy_friction.
		Vector2T<float> InitialPhysics;
		Vector2T<float> InitialPosition;
		Vector2T<float> InitialScale;
		Vector2T<float> InitialSpeed;
		float           InitialAngle;
		float           InitialRenderLayer;

		Vector4T<float> InitialVertexColor;

		std::function<void(GameActorExecutor*)> CollisionCallbackFunc =
			[](GameActorExecutor*) {};
		// phy_engine bit_filter, max group = 16.
		ActorCollisionGroup ActorCollisionThis;
		ActorCollisionGroup ActorCollisionFilter;

		// cancel: bool comp_enable flags.
		// update: enum bit flags. RCSZ 2024_11_10.
		ActorComponentFlags ActorComponentConifg = 
			ActorEnableTransform | ActorEnableHealth | ActorEnableRender | ActorEnableCollision;

		GameActorHealthDESC ActorHealthSystem;

		GameComponents::ActorActionLogicBase* ActorLogicObject;
		
		GameActorExecutorDESC() :
			ActorShaderResource(nullptr),
			ActorPhysicsWorld  ("SYSTEM_PHY_WORLD"),
			ActorPhysicalMode  (ActorPhysicsMove),

			VectorCalcIsForce   (false),
			CollisionBoxIsCircle(false),

			InitialPhysics    (Vector2T<float>(1.0f, 0.7f)),
			InitialPosition   (Vector2T<float>(0.0f, 0.0f)),
			InitialScale      (Vector2T<float>(1.0f, 1.0f)),
			InitialSpeed      (Vector2T<float>(0.0f, 0.0f)),
			InitialAngle      (0.0f),
			InitialRenderLayer(50.0f),

			InitialVertexColor(Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f)),

			ActorCollisionThis  ((ActorCollisionGroup)0x0001),
			ActorCollisionFilter((ActorCollisionGroup)0xFFFF),
			ActorHealthSystem   ({}),
			ActorLogicObject    (nullptr)
		{}
	};

	class GameActorExecutor :
		public GraphicsEngineDataset::GLEngineVirTextureData,
		public PhysicsEngine::PhyEngineCoreDataset,
		public __ACTOR_MODULES_TIMESTEP
	{
	protected:
		std::chrono::steady_clock::time_point   ActorTimer      = std::chrono::steady_clock::now();
		GameComponents::ActorPrivateINFO        ActorUniqueInfo = {};
		GameActorShader*                        ActorRenderRes  = nullptr;
		std::function<void(GameActorExecutor*)> ActorCollision  = [](GameActorExecutor*) {};
		
		std::string ActorPhysicsWorld = {};
		PhyBodyKey  ActorPhysicsItem  = {};

		// physical collision: type:null, unique:first.
		GameComponents::ActorPrivateINFO ActorCollisionInfo = {};

		float VirTimerStepSpeed = 1.0f;
		float VirTimerCount     = 0.0f;

		// shader rendering size, shader_uniform param.
		Vector2T<float> RenderingResolution = {};
		
		GameComponents::RenderingParams ActorRenderParams = {};

		Vector2T<float> ActorLastPosition = {};

		// actor basic components.
		GameComponents::ActorSpaceTrans*   ActorCompSpaceTrans  = nullptr;
		GameComponents::ActorHealthTrans*  ActorCompHealthTrans = nullptr;
		GameComponents::ActorRendering*    ActorCompRendering   = nullptr;
		GameComponents::ActorCoordConvert* ActorCompConvert     = nullptr;
		GameComponents::ActorActionLogic*  ActorCompActionLogic = nullptr;
	public:
		GameActorExecutor(uint32_t TYPE, const GameActorExecutorDESC& INIT_DESC);
		~GameActorExecutor();

		GameComponents::ActorPrivateINFO ActorGetPrivate()   { return ActorUniqueInfo;    }
		GameComponents::ActorPrivateINFO ActorGetCollision() { return ActorCollisionInfo; }

		float ActorGetHealth(size_t count) {
#if PSAG_DEBUG_MODE
			if (count < ActorCompHealthTrans->ActorHealthStateOut.size())
				return ActorCompHealthTrans == nullptr ? 0.0f : ActorCompHealthTrans->ActorHealthStateOut[count];
			return 0.0f;
#else
			return ActorCompHealthTrans->ActorHealthStateOut[count];
#endif
		}

		void ActorModifyHealth(size_t count, float value) {
			ActorCompHealthTrans->SetActorHealth(count, value);
		}

		// timer: accuracy: ms, unit: s.
		float ActorGetLifeTime();
		// actor get_state: position & scale & rotate.
		Vector2T<float> ActorGetPosition() { return ActorRenderParams.RenderPosition; };
		Vector2T<float> ActorGetScale()    { return ActorRenderParams.RenderScale; }
		float           ActorGetRotate()   { return ActorRenderParams.RenderRotate; };

		Vector2T<float> ActorGetMoveSpeed()   { return ActorCompSpaceTrans->ActorStateMoveSpeed; }
		float           ActorGetRotateSpeed() { return ActorCompSpaceTrans->ActorStateRotateSpeed; }

		float ActorGetSpeed() {
			auto VecSpeed = ActorGetMoveSpeed();
			return std::sqrt(std::pow(VecSpeed.vector_x, 2.0f) + std::pow(VecSpeed.vector_y, 2.0f));
		}

		float* ActorLayerValuePtr() { return &ActorRenderParams.RenderLayerHeight; }

		void ActorApplyForceRotate(float vec)                { ActorCompSpaceTrans->ActorTransRotateValue = vec; };
		void ActorApplyForceMove(const Vector2T<float>& vec) { ActorCompSpaceTrans->ActorTransMoveValue = vec; };
		// not scale actor collision_box.
		void ActorModifyScale(const Vector2T<float>& vec) { ActorRenderParams.RenderScale = vec; };
		// set actor position & angle.
		void ActorModifyState(const Vector2T<float>& position, float angle) {
			ActorCompSpaceTrans->SetActorState(position, angle);
		}
		void ActorModifyColorBlend(const Vector4T<float>& color) {
			ActorRenderParams.RenderColorBlend = color;
		}

		// actor virtual(scene) coord =convert=> window coord(pixel).
		Vector2T<float> ActorMappingWindowCoord();

		void ActorUpdateHealth();
		void ActorUpdate();
		void ActorRendering();
	};

	struct GameActorCircleSensorDESC {
		std::string SensorPhysicsWorld;

		Vector2T<float> InitialPosition;
		float InitialRadius;
		float InitialScale;

		GameActorCircleSensorDESC() :
			SensorPhysicsWorld("SYSTEM_PHY_WORLD"),

			InitialPosition(Vector2T<float>(0.0f, 0.0f)),
			InitialRadius(10.0f),
			InitialScale (1.0f)
		{}
	};

	// 物理传感器(圆形), 用于范围探测.
	class GameActorCircleSensor :public PhysicsEngine::PhyEngineCoreDataset {
	protected:
		size_t ActorSensorUniqueID = {};
		GameComponents::ActorPrivateINFO SensorCollisionInfo = {};
		
		std::string SensorPhysicsWorld = {};
		PhyBodyKey  SensorPhysicsItem  = {};
	public:
		GameActorCircleSensor(const GameActorCircleSensorDESC& INIT_DESC);
		~GameActorCircleSensor();

		void SensorModifyState(const Vector2T<float>& position);
		// sensor and actor collision, type(null).
		GameComponents::ActorPrivateINFO ActorGetCollision() { 
			return SensorCollisionInfo;
		}
		void SensorUpdate();
	};
}

// 用于构建静态场景, 比静态'Actor'更加轻量, 但是使用了一些共同组件.
namespace GameBrickCore {
	StaticStrLABEL PSAGM_BRICK_CORE_LABEL = "PSAG_BRICK_CORE";

	struct GameBrickExecutorDESC {
		GameActorCore::GameActorShader* BrickShaderResource;
		std::string                     BrickPhysicsWorld;

		// collision box preset circle(polygon: 20).
		bool CollisionBoxIsCircle;

		// params: x:phy_density, y:phy_friction.
		Vector2T<float> InitialPhysics;
		Vector2T<float> InitialPosition;
		Vector2T<float> InitialScale;
		float           InitialAngle;
		float           InitialRenderLayer;

		Vector4T<float> InitialVertexColor;

		// system core comp switch_flags.
		bool EnableRendering = true;
		bool EnableCollision = true;

		GameBrickExecutorDESC() :
			BrickShaderResource(nullptr),
			BrickPhysicsWorld  ({}),

			CollisionBoxIsCircle(false),

			InitialPhysics    (Vector2T<float>(1.0f, 0.7f)),
			InitialPosition   (Vector2T<float>(0.0f, 0.0f)),
			InitialScale      (Vector2T<float>(1.0f, 1.0f)),
			InitialAngle      (0.0f),
			InitialRenderLayer(1.0f),
			InitialVertexColor(Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f))
		{}
	};

	class GameBrickExecutor :
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData,
		public PhysicsEngine::PhyEngineCoreDataset,
		public __ACTOR_MODULES_TIMESTEP
	{
	protected:
		size_t                          BrickUniqueID  = NULL;
		GameActorCore::GameActorShader* BrickRenderRes = nullptr;

		// physical collision: type:null, unique:first.
		GameComponents::ActorPrivateINFO BrickCollisionInfo = {};

		std::string BrickPhysicsWorld = {};
		PhyBodyKey  BcickPhysicsItem  = {};

		float VirTimerStepSpeed = 1.0f;
		float VirTimerCount     = 0.0f;

		// shader rendering size, shader_uniform param.
		Vector2T<float> RenderingResolution = {};

		GameComponents::RenderingParams BrickRenderParams = {};
		GameComponents::ActorRendering* BirckCompRendering = nullptr;
	public:
		~GameBrickExecutor();
		GameBrickExecutor(const GameBrickExecutorDESC& INIT_DESC);

		size_t          BrickGetUniqueID() { return BrickUniqueID; }
		Vector2T<float> BrickGetPosition() { return BrickRenderParams.RenderPosition; }
		Vector2T<float> BrickGetScale()    { return BrickRenderParams.RenderScale; }
		float           BrickGetRotate()   { return BrickRenderParams.RenderRotate; }

		void BrickRendering();
	};
}

namespace GameCoreManager {
	StaticStrLABEL PSAGM_CORE_MAG_LABEL = "PSAG_GAMECORE_MAG";

	class GameActorShaderManager {
	protected:
		std::unordered_map<std::string, GameActorCore::GameActorShader*> GameShaderDataset = {};
	public:
		GameActorShaderManager() {};
		~GameActorShaderManager();

		// call shader"CreateShaderResource" => storage.
		bool CreateActorShader(
			const char* shader_name, GameActorCore::GameActorShader* shader, bool default_circle = false
		);
		bool DeleteActorShader(const char* shader_name);
		GameActorCore::GameActorShader* FindActorShader(const char* shader_name);
	};

	class GameActorExecutorManager {
	protected:
		std::unordered_map<size_t, GameActorCore::GameActorExecutor*> GameActorDataset = {};
		std::vector<size_t> GameActorFreeList = {};
	public:
		GameActorExecutorManager() {};
		~GameActorExecutorManager();

		// get_source data(hash_map), pointer.
		std::unordered_map<size_t, GameActorCore::GameActorExecutor*>* GetSourceData() {
			return &GameActorDataset;
		}

		size_t CreateGameActor(uint32_t actor_code, const GameActorCore::GameActorExecutorDESC& actor_desc);
		bool   DeleteGameActor(size_t unique_code);
		GameActorCore::GameActorExecutor* FindGameActor(size_t unique_code);

		void UpdateManagerData();

		// update all actor_object "ActorUpdateHealth" & "ActorUpdate".
		// rendering all actor_object "ActorRendering".
		void RunAllGameActor();
	};

	class GameBrickExecutorManager {
	protected:
		std::unordered_map<size_t, GameBrickCore::GameBrickExecutor*> GameBrickDataset = {};
		std::vector<size_t> GameBrickFreeList = {};
	public:
		GameBrickExecutorManager() {};
		~GameBrickExecutorManager();

		// get_source data(hash_map), pointer.
		std::unordered_map<size_t, GameBrickCore::GameBrickExecutor*>* GetSourceData() {
			return &GameBrickDataset;
		}

		size_t CreateGameBrick(const GameBrickCore::GameBrickExecutorDESC& brick_desc);
		bool   DeleteGameBrick(size_t unique_code);
		
		// rendering all brick_object "BrickRendering".
		void RunAllGameBrick();
	};
}

namespace GameDebugGuiWindow {
	// debug window [panel], actor. 
	void DebugWindowGuiActorPawn(const char* name, GameActorCore::GameActorExecutor* actor);
	// debug window [panel], actor_manager(actors). 
	void DebugWindowGuiActors(const char* name, std::unordered_map<size_t, GameActorCore::GameActorExecutor*>* actors);

	// debug window [panel], framerate_info.
	class DebugWindowGuiFPS {
	protected:
		const char* DebugWindowName = {};
		std::chrono::steady_clock::time_point DebugFpsTimer = std::chrono::steady_clock::now();

		// params: run, avg, max, min(500ms).
		float FramerateParams[4] = {};
		int   FramerateCount     = NULL;
		float FramerateLimitMax  = 0.0f;
	public:
		DebugWindowGuiFPS(const char* name, float max_fps) : 
			DebugWindowName(name), FramerateLimitMax(max_fps)
		{};
		void RenderingWindowGui();
	};
}

#endif