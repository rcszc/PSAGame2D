// psag_actor_module. RCSZ. [middle_level_module]
// => "psag_actor_global_atomic.h"
#ifndef __PSAG_ACTOR_MODULE_HPP
#define __PSAG_ACTOR_MODULE_HPP
#include "psag_actor_global_atomic.h"

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
		// health calc mode: lerp_trans.
		float HealthState = 0.0f;
		float HealthSpeed = 0.0f;
		float HealthMax   = 0.0f;

		GameActorHP(float state, float speed, float max) : 
			HealthState(state), HealthSpeed(speed), HealthMax(max)
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
		float           RenderAngle;
		float           RenderLayerHeight;

		// render color: uniform"FxColor". [20240825]
		Vector4T<float> RenderColorBlend = Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f);

		RenderingParams() : RenderPosition({}), RenderScale({}), RenderAngle(0.0f), RenderLayerHeight(-1.0f) {}
		RenderingParams(const Vector2T<float>& rp, const Vector2T<float>& rs, float rr, float rz, const Vector4T<float>& rc) :
			RenderPosition(rp), RenderScale(rs), RenderAngle(rr), RenderLayerHeight(rz), RenderColorBlend(rc)
		{}
	};

	enum VirTextureMode {
		VIR_TEXTURE_ROOT = 1 << 1, // 着色器持有纹理.
		VIR_TEXTURE_REFE = 1 << 2  // 着色器引用纹理.
	};
	// actor shader texture params.
	struct VirtualTextureInfo {
		// virtual texture engine index(key).
		VirTextureUnique VirTextureIndex;
		std::string VirTextureName;
		VirTextureMode VirTextureMode;
		GraphicsEngineDataset::VirTextureUniformName VirTextureUname;

		VirtualTextureInfo() :
			VirTextureIndex(PSAG_VIR_TEXTURE_INVALID),
			VirTextureName ({}),
			VirTextureMode (VIR_TEXTURE_ROOT),
			VirTextureUname({})
		{}
	};
	class ActorRendering :
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData
	{
	protected:
		PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderState OGLAPI_OPER = {};
		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform ShaderUniform = {};
	public:
		ResUnique ShaderIndex      = NULL;
		ResUnique VertexGroupIndex = NULL;

		Vector2T<float> RenderResolution = {};
		// shader virtual textures group.
		std::vector<VirtualTextureInfo> VirTextures = {};
		
		virtual void UpdateActorRendering(const RenderingParams& params, float time_count);
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
			ActorSpaceTransNULL(const std::string& phy_world, PhyBodyKey phy_body) : 
				ActorSpaceTrans(phy_world, phy_body) 
			{}
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
		};
	}
}

namespace GameActorScript {
	extern const char* psag_shader_public_frag_header;
	// warn: non-format shader script. 20250219.
	extern const char* psag_shader_public_frag_texture;

	extern const char* psag_shader_private_frag_dfirst;
	extern const char* psag_shader_private_frag_dlast;
}

namespace GameActorCore {
	StaticStrLABEL PSAGM_ACTOR_CORE_LABEL = "PSAG_ACTOR_CORE";
	namespace Type {
		// actor null_type value.
		constexpr uint32_t ActorTypeNULL = 0;

		class GameActorTypeBind {
		protected:
			std::unordered_map<std::string, uint32_t> ActorTypeMapping = {};
			uint32_t ActorTypeCount = ActorTypeNULL;
		public:
			uint32_t ActorTypeIs(const std::string& type_name);

			bool ActorTypeCreate(const std::string& type_name);
			bool ActorTypeDelete(const std::string& type_name);
		};
		// [GLOBAL.OBJECT] Actor类型分配器.
		extern GameActorTypeBind ActorTypeAllotter;
	}
	class GameActorExecutor;

	// game_actor render system_preset shader script.
	class GameActorPresetScript {
	public:
		std::string TmpScriptDrawImage(const std::string& tex_name) {
			if (tex_name.empty()) return "glsl gen error: tex name invalid.";
			// shader default 1-tex draw glsl. 
			return std::string(GameActorScript::psag_shader_private_frag_dfirst + 
				tex_name + GameActorScript::psag_shader_private_frag_dlast);
		};
	};

	struct GameActorShaderVerticesDESC {
		// shader pipline vertex color.
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

	using VirTexuresGroup = std::vector<GameComponents::VirtualTextureInfo>;
	// shader resource, shader => mul actors.
	class GameActorShader :
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData
	{
	private:
		// t-safe, texture(vir) unique uniform name.
		static std::unordered_set<std::string> TUUN_RegisterInfo;

		PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform U_LOADER = {};
		PsagShader S_HANDLE = OPENGL_INVALID_HANDEL;
	protected:
		Vector4T<float> ShaderDefaultColor = {};
		// x:vert_shader, y:frag_shader.
		Vector2T<std::string> ShaderScript = {};

		std::vector<Vector2T<float>>* VerticesPosition = nullptr;
		std::vector<Vector2T<float>>* VerticesUvCoord  = nullptr;

		bool ShaderImageTextureLoad(VirTextureUnique* ref_texture, const ImageRawData& image);
	public:
		GameActorShader(const std::string& shader_frag, const Vector2T<uint32_t>& size);
		~GameActorShader();
		// create actor shader_resource.
		bool CreateShaderResource(bool default_is_circle = false);

		// load vertices(pos,uv) resource. (warn: ref)
		bool ShaderVerticesLoad(GameActorShaderVerticesDESC& VER_DESC);

		// shader: referencing virtual texture(non-delete).
		bool ShaderVirTextureLADD(const std::string& u_name, VirTextureUnique virtex);
		// shader: load-add image => virtual texture.
		bool ShaderImageLADD(const std::string& u_name, const ImageRawData& image);

		// => uniform context (shader_context) => set_uniform.
		void UniformSetContext(std::function<void()> context_func);

		void UniformMatrix3x3(const char* name, const PsagMatrix3& matrix);
		void UniformMatrix4x4(const char* name, const PsagMatrix4& matrix);

		void UniformInt32(const char* name, const int32_t& value);
		void UniformFP32 (const char* name, const float& value);

		void UniformVec2(const char* name, const Vector2T<float>& value);
		void UniformVec3(const char* name, const Vector3T<float>& value);
		void UniformVec4(const char* name, const Vector4T<float>& value);

		// ================================ shader params ================================
		std::vector<Vector2T<float>>* __GET_VERTICES_RES() {
			return VerticesPosition;
		}
		VirTexuresGroup __VIR_TEXTURES_GROUP = {};

		ResUnique __ACTOR_SHADER_ITEM = NULL;
		ResUnique __ACTOR_VERTEX_ITEM = NULL;

		Vector2T<uint32_t> __RENDER_RESOLUTION = {};
	};

#define PSAG_ACTOR_HP_ERR -1.0f
	struct GameActorHealthDESC {
		std::vector<GameComponents::GameActorHP> InitialActorHealth;
	};

	struct GameCollisionPAIR {
		// pair: a -> collision <- b.
		GameComponents::ActorPrivateINFO ThisActor = {};
		GameComponents::ActorPrivateINFO ThatActor = {};

		GameCollisionPAIR(
			const GameComponents::ActorPrivateINFO& a_this, 
			const GameComponents::ActorPrivateINFO& a_that) 
			: ThisActor(a_this), ThatActor(a_that)
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
		// collision groups type:[0~E]
		ActorPhyGroup0 = 1 << 0, ActorPhyGroup1 = 1 << 1,
		ActorPhyGroup2 = 1 << 2, ActorPhyGroup3 = 1 << 3,
		ActorPhyGroup4 = 1 << 4, ActorPhyGroup5 = 1 << 5,
		ActorPhyGroup6 = 1 << 6, ActorPhyGroup7 = 1 << 7,
		ActorPhyGroup8 = 1 << 8, ActorPhyGroup9 = 1 << 9,

		ActorPhyGroup10 = 1 << 10, ActorPhyGroup11 = 1 << 11,
		ActorPhyGroup12 = 1 << 12, ActorPhyGroup13 = 1 << 13,
		ActorPhyGroup14 = 1 << 14, 
		// group15 system default: brick. 
		// ActorPhyGroup15 = 1 << 15,
		ActorPhyGroupALL = 0b1111'1111'1111'1111
	};
	ActorCollisionGroup  operator| (ActorCollisionGroup  a, ActorCollisionGroup b);
	ActorCollisionGroup& operator|=(ActorCollisionGroup& a, ActorCollisionGroup b);

	ActorCollisionGroup operator&(ActorCollisionGroup a, ActorCollisionGroup b);
	ActorCollisionGroup operator~(ActorCollisionGroup v);

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
		float           InitialAngleSpeed;
		float           InitialRenderLayer;

		Vector4T<float> VertexColor;

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
			InitialAngleSpeed (0.0f),
			InitialRenderLayer(50.0f),

			VertexColor(Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f)),

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
		// prev frame calc position.
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
			if (count >= ActorCompHealthTrans->ActorHealthStateOut.size()) {
				PSAG_LOGGER::PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, 
					"actor default health_out index err.");
				return PSAG_ACTOR_HP_ERR;
			}
#endif
			return ActorCompHealthTrans->ActorHealthStateOut[count];
		}
		float ActorGetHealthMax(size_t count) {
#if PSAG_DEBUG_MODE
			if (count >= ActorCompHealthTrans->ActorHealthStateOut.size()) {
				PSAG_LOGGER::PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, 
					"actor default health_max index err.");
				return PSAG_ACTOR_HP_ERR;
			}
#endif
			return ActorCompHealthTrans->ActorHealthState[count].HealthMax;
		}
		void ActorModifyHealth(size_t count, float value) {
			ActorCompHealthTrans->SetActorHealth(count, value);
		}

		// timer: accuracy: ms, unit: s.
		float ActorGetLifeTime();
		// actor get_state: position & scale & rotate.
		Vector2T<float> ActorGetPosition() const { return ActorRenderParams.RenderPosition; };
		Vector2T<float> ActorGetScale()    const { return ActorRenderParams.RenderScale; }
		float           ActorGetAngle()    const { return ActorRenderParams.RenderAngle; };

		Vector2T<float> ActorGetMoveSpeed()   const { return ActorCompSpaceTrans->ActorStateMoveSpeed; }
		float           ActorGetRotateSpeed() const { return ActorCompSpaceTrans->ActorStateRotateSpeed; }

		float ActorGetSpeed() {
			auto VecSpeed = ActorGetMoveSpeed();
			return std::sqrt(std::pow(VecSpeed.vector_x, 2.0f) + std::pow(VecSpeed.vector_y, 2.0f));
		}

		// warn: ref render comp layer_height ptr.
		float* ActorLayerValuePtr() { return &ActorRenderParams.RenderLayerHeight; }

		void ActorApplyForceRotate(float vec) const { ActorCompSpaceTrans->ActorTransRotateValue = vec; };
		void ActorApplyForceMove(const Vector2T<float>& vec) const { ActorCompSpaceTrans->ActorTransMoveValue = vec; };
		
		// "ActorModifyScale": not scale actor collision_box.
		// "ActorModifyState": set actor position & angle.
		// "ActorModifyColorBlend": set actor shader vtx color.

		void ActorModifyScale(const Vector2T<float>& vec) { 
			ActorRenderParams.RenderScale = vec; 
		};
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
			InitialRadius  (10.0f),
			InitialScale   (1.0f)
		{}
	};

	// 物理传感器(圆形), 用于范围探测. [ALPHA]
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

// 用于构建静态场景, 比'Actor'更加轻量, 但是使用了一些共同组件.
namespace GameEnvmtCore {
	StaticStrLABEL PSAGM_ENVMT_CORE_LABEL = "PSAG_ENVMT_CORE";

	struct GameEnvmtExecutorDESC {
		GameActorCore::GameActorShader* EnvmtShaderResource;
		std::string EnvmtPhysicsWorld;

		// collision box preset circle(polygon: 20).
		bool CollisionBoxIsCircle;

		// params: x:phy_density, y:phy_friction.
		Vector2T<float> InitialPhysics;
		Vector2T<float> InitialPosition;
		Vector2T<float> InitialScale;
		float           InitialAngle;
		float           InitialRenderLayer;

		Vector4T<float> VertexColor;

		// system core comp switch_flags.
		bool EnableRendering = true;
		bool EnableCollision = true;

		GameEnvmtExecutorDESC() :
			EnvmtShaderResource(nullptr),
			EnvmtPhysicsWorld  ({}),

			CollisionBoxIsCircle(false),

			InitialPhysics    (Vector2T<float>(1.0f, 0.7f)),
			InitialPosition   (Vector2T<float>(0.0f, 0.0f)),
			InitialScale      (Vector2T<float>(1.0f, 1.0f)),
			InitialAngle      (0.0f),
			InitialRenderLayer(1.0f),
			VertexColor(Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f))
		{}
	};

	// world static environment.
	class GameEnvmtExecutor :
		public GraphicsEngineDataset::GLEngineStaticVertexData,
		public GraphicsEngineDataset::GLEngineVirTextureData,
		public PhysicsEngine::PhyEngineCoreDataset,
		public __ACTOR_MODULES_TIMESTEP
	{
	private: 
		size_t EnvmtEntryUniqueID = NULL;
	protected:
		GameActorCore::GameActorShader* EnvmtRenderRes = nullptr;
		// physical collision: type:null, unique:first.
		GameComponents::ActorPrivateINFO EnvmtCollisionInfo = {};

		std::string EnvmtPhysicsWorld = {};
		PhyBodyKey  EnvmtPhysicsItem  = {};

		float VirTimerStepSpeed = 1.0f;
		float VirTimerCount     = 0.0f;

		// shader rendering size, shader_uniform param.
		Vector2T<float> RenderingResolution = {};

		GameComponents::RenderingParams EnvmtRenderParams  = {};
		GameComponents::ActorRendering* EnvmtCompRendering = nullptr;
	public:
		~GameEnvmtExecutor();
		GameEnvmtExecutor(const GameEnvmtExecutorDESC& INIT_DESC);

		size_t          EnvmtGetUniqueID() const { return EnvmtEntryUniqueID; }
		Vector2T<float> EnvmtGetPosition() const { return EnvmtRenderParams.RenderPosition; }
		Vector2T<float> EnvmtGetScale()    const { return EnvmtRenderParams.RenderScale; }
		float           EnvmtGetRotate()   const { return EnvmtRenderParams.RenderAngle; }

		void EnvmtRendering();
	};
}

namespace GameCoreManager {
	StaticStrLABEL PSAGM_CORE_MAG_LABEL = "PSAG_GAMECORE_MAG";

	class GameActorShaderManager {
	protected:
		std::unordered_map<std::string, GameActorCore::GameActorShader*> GameShaderDataset = {};
	public:
		GameActorShaderManager() {
			// ATOMIC ENTITIES COUNTER.
			++ActorSystemAtomic::GLOBAL_PARAMS_M_SHADERS;
		};
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
		GameActorExecutorManager() {
			// ATOMIC ENTITIES COUNTER.
			++ActorSystemAtomic::GLOBAL_PARAMS_M_ACTORS;
		};
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

	class GameEnvmtExecutorManager {
	protected:
		std::unordered_map<size_t, GameEnvmtCore::GameEnvmtExecutor*> GameEnvmtDataset = {};
		std::vector<size_t> GameEnvmtFreeList = {};
	public:
		GameEnvmtExecutorManager() {
			// ATOMIC ENTITIES COUNTER.
			++ActorSystemAtomic::GLOBAL_PARAMS_M_EVNS;
		};
		~GameEnvmtExecutorManager();

		// get_source data(hash_map), pointer.
		std::unordered_map<size_t, GameEnvmtCore::GameEnvmtExecutor*>* GetSourceData() {
			return &GameEnvmtDataset;
		}

		size_t CreateGameEnvmt(const GameEnvmtCore::GameEnvmtExecutorDESC& brick_desc);
		bool   DeleteGameEnvmt(size_t unique_code);
		
		// rendering all entities_object.
		void RunAllGameEnvmt();
	};
}

namespace GameDebugGuiWindow {
	using ActorEntryPtr = GameActorCore::GameActorExecutor*;
	// debug window [panel], actor. <OLD>
	PSAG_OLD_FUNC void DebugWindowGuiActorPawn(const char* name, ActorEntryPtr actor);
	// debug window [panel], actor_manager(actors). <OLD>
	PSAG_OLD_FUNC void DebugWindowGuiActors(
		const char* name, std::unordered_map<size_t, ActorEntryPtr>* actors
	);
	// framereate plot values cache length.
	constexpr size_t FrameCacheLen = 512;

	// framework debug params render panel.
	// v0.1.2 update: 2025_02_17 RCSZ.
	class DebugGamePANEL {
	protected:
		// info view window 0:fps, 1:ppactor.
		const float DebugWindowHeight[3] = { 184.0f, 192.0f, 112.0f };
		std::string DebugWindowName = {}, DebugWindowFrameName = {};

		std::chrono::steady_clock::time_point 
			DebugUpdateTimer = std::chrono::steady_clock::now(),
			DebugCacheTimer  = std::chrono::steady_clock::now();
		// player pawn actor ref(ptr).
		GameActorCore::GameActorExecutor* AEREF = nullptr;

		// params: run, avg, max, min(2.5s).
		float FramerateParams[4] = {};
		float FramerateLimitMax  = 0.0f;
		// sync frame counter.
		int FrameCounter = NULL;
		
		float FramerateCache[FrameCacheLen] = {};

		void GameInfoViewFPS    (float width);
		void GameInfoViewPPActor(float width);
		void GameInfoViewGlobal (float width);
	public:
		DebugGamePANEL(const std::string& name, float max_fps) :
			DebugWindowName(name), DebugWindowFrameName(name + "##FVIEW"), 
			FramerateLimitMax(max_fps)
		{};
		void SettingPPActorRef(GameActorCore::GameActorExecutor* actor);

		void RenderingWindowGui();
	};
}

#endif