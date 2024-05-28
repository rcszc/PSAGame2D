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

namespace ActorShaderScript {
	extern const char* PsagShaderScriptPublicVS;
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
	// actor information global_code & type_code.
	struct ActorPrivateINFO {
		size_t   ActorUniqueCode;
		uint32_t ActorTypeCode;
	};

	// name, mode, params = {}
	// mode: 1:create, 2:delete params: x:gravity_vec_x, y:gravity_vec_y
	class GameActorPhysicalWorld :public PhysicsEngine::PhyEngineCoreDataset {
	public:
		GameActorPhysicalWorld(std::string name_key, int mode, Vector2T<float> params = {}) {
			if (mode == 1) OperFlag = PhysicsWorldCreate(name_key, params);
			if (mode == 2) OperFlag = PhysicsWorldDelete(name_key);
		}
		bool OperFlag = false;
	};

	/* Actor Fragment ShaderDemo:
		#version 460 core

		in vec4 FxColor;
		in vec2 FxCoord;

		uniform vec2  RenderResolution;
		uniform float ActorTime;

		uniform vec2  ActorMove;
		uniform float ActorRotate;
		uniform vec2  ActorScale;

		uniform sampler2DArray ActorTexture;
		uniform int            ActorTextureLayer;
		uniform vec2           ActorTextureCropping;
		uniform vec2           ActorTextureSize;

		out vec4 FragColor;

		void main()
		{
			FragColor = vec4(1.0);
		}
	*/
	class GameActorShader :public GraphicsEngineDataset::GLEngineSmpTextureData {
	protected:
		GraphicsEngineDataset::VirTextureUniformName SystemPresetUname() {
			GraphicsEngineDataset::VirTextureUniformName U_NAME = {};
			// preset shader uniform name.
			U_NAME.TexParamSampler  = "ActorTexture";
			U_NAME.TexParamLayer    = "ActorTextureLayer";
			U_NAME.TexParamCropping = "ActorTextureCropping";
			U_NAME.TexParamSize     = "ActorTextureSize";
			return U_NAME;
		}
		bool CheckRepeatTex(VirTextureUnqiue virtex);
		bool ReferVirTextureFlag = false;
	public:
		GameActorShader(const std::string& SHADER_FRAG, const Vector2T<uint32_t>& RESOLUTION);
		~GameActorShader();

		// referencing virtual texture(non-free).
		bool ShaderLoadVirTexture(VirTextureUnqiue virtex);
		// create virtual texture.
		bool ShaderLoadImage(const ImageRawData& image);

		ResUnique                                    __VIR_TEXTURE_ITEM = {};
		GraphicsEngineDataset::VirTextureUniformName __VIR_UNIFORM_ITEM = {};
		
		ResUnique   __ACTOR_SHADER_ITEM = {};
		PsagMatrix4 __ACTOR_MATRIX_ITEM = {};

		Vector2T<uint32_t> __WINDOW_RESOLUTION = {};
	};

	// hp handler function params.
	struct HealthFuncParams {
		size_t ThisActorUniqueCode;
		ActorPrivateINFO ActorCollision;

		float* ActorHealthStates;
		size_t ActorHealthLength;

		Vector2T<float> ActorPosition;
		Vector2T<float> ActorSpeed;

		HealthFuncParams() { 
			std::memset(this, NULL, sizeof(HealthFuncParams));
		}
	};
#define PSAG_HEALTH_STATE_NUM 16
	struct GameActorHealthDESC {
		float InitialHealthState[PSAG_HEALTH_STATE_NUM];
		float InitialHealthSpeed[PSAG_HEALTH_STATE_NUM];

		// process function demo:
		// demo: void MyHPprocess(const HealthFuncParams& params);
		std::function<void(const HealthFuncParams& params)> HealthHandlerFunc;

		GameActorHealthDESC() {
			std::memset(InitialHealthState, 0, PSAG_HEALTH_STATE_NUM * sizeof(float));
			std::memset(InitialHealthSpeed, 1, PSAG_HEALTH_STATE_NUM * sizeof(float));

			HealthHandlerFunc = [](const HealthFuncParams&) {};
		}
	};

	enum ActorPhyMode {
		PhyMoveActor  = 1 << 1,
		PhyFixedActor = 1 << 2
	};
	// init(config) descriptor.
	struct GameActorActuatorDESC {
		GameActorShader* ActorShaderResource;
		std::string      ActorInPhyWorld;
		ActorPhyMode     ActorPhysicalMode;

		float ControlFricMove;
		float ControlFricRotate;
		float ControlFricScale;

		// params: x:phy_density, y:phy_friction.
		Vector2T<float> InitialPhysics;
		Vector2T<float> InitialPosition;
		Vector2T<float> InitialScale;
		Vector2T<float> InitialSpeed;
		float           InitialRotate;

		GameActorHealthDESC ActorHealthSystem;
		
		GameActorActuatorDESC() :
			ActorShaderResource(nullptr),
			ActorInPhyWorld    ("SYSTEM_PHY_WORLD"),
			ActorPhysicalMode  (PhyMoveActor),

			ControlFricMove  (1.0f),
			ControlFricRotate(1.0f),
			ControlFricScale (1.0f),

			InitialPhysics (Vector2T<float>(1.0f, 0.7f)),
			InitialPosition(Vector2T<float>(0.0f, 0.0f)),
			InitialScale   (Vector2T<float>(1.0f, 1.0f)),
			InitialSpeed   (Vector2T<float>(0.0f, 0.0f)),
			InitialRotate  (0.0f),

			ActorHealthSystem({})
		{}
	};

	// 游戏 Actor (实体)执行器.
	class GameActorActuator :
		public GraphicsEngineDataset::GLEngineStcVertexData,
		public GraphicsEngineDataset::GLEngineSmpTextureData,
		public PhysicsEngine::PhyEngineCoreDataset,
		public __ACTOR_MODULES_TIMESTEP
	{
	protected:
		std::chrono::steady_clock::time_point ActorTimer = std::chrono::steady_clock::now();

		ActorPrivateINFO ActorUniqueInfo = {};
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsUniform ShaderUniform = {};
		std::string ActorInPhyWorld = {};

		float VirTimerStepSpeed = 1.0f;
		float VirTimerCount     = 0.0f;

		ResUnique ActorShaderItem  = {};
		ResUnique ActorTextureItem = {};

		// shader rendering size, shader_uniform.
		Vector2T<float>                              RenderingResolution = {};
		PsagMatrix4                                  RenderingMatrix     = {};
		GraphicsEngineDataset::VirTextureUniformName RenderingUniform    = {};

		ResUnique ActorPhysicsItem = {};
		float ActorPhysicsDensity = 1.0, ActorPhysicsFriction = 0.7f;

		// x:move, y:rotate, z:scale.
		Vector3T<float> AnimInterSpeed = {};

		// move,scale 0:set_speed, 1:render_value.
		// rotate x:set_speed, y:render_value.
		Vector2T<float> ActorStateMove[2]  = {};
		Vector2T<float> ActorStateRotate   = {};
		Vector2T<float> ActorStateScale[2] = { Vector2T<float>(1.0f, 1.0f), Vector2T<float>(1.0f, 1.0f) };

		ActorPrivateINFO ActorCollisionINFO = {};

		// actor hp_state system. 0:set_state, 1:actor_state, 2:trans_speed.
		float ActorHealthState[3][PSAG_HEALTH_STATE_NUM] = {};
		std::function<void(const HealthFuncParams&)> ActorHealthHandlerFunc = {};
	public:
		GameActorActuator(uint32_t TYPE, const GameActorActuatorDESC& INIT_DESC);
		~GameActorActuator();

		ActorPrivateINFO ActorGetPrivate()   { return ActorUniqueInfo;    }
		ActorPrivateINFO ActorGetCollision() { return ActorCollisionINFO; }

		float ActorGetHealth(size_t count) {
			if (count < PSAG_HEALTH_STATE_NUM)
				return ActorHealthState[1][count];
			return 0.0f;
		}

		// timer: accuracy: ms, unit: s.
		float ActorGetLifeTime();
		// actor get_state: rotate & position.
		float           ActorGetRotate()   { return ActorStateRotate.vector_y; };
		Vector2T<float> ActorGetPosition() { return ActorStateMove[1]; };

		float*           ActorMappingRotateSpeed() { return &ActorStateRotate.vector_x; };
		Vector2T<float>* ActorMappingMoveSpeed()   { return &ActorStateMove[0]; };
		// not scale actor collision_box.
		Vector2T<float>* ActorMappingScaleModify() { return &ActorStateScale[0]; };

		// actor virtual(scene) coord =convert=> window coord(pixel).
		Vector2T<float> ActorConvertVirCoord(Vector2T<uint32_t> window_res);

		void ActorUpdateHealth();
		void ActorUpdate();
		void ActorRendering();
	};
}

namespace GameActorManager {
	StaticStrLABEL PSAGM_ACTOR_MAG_LABEL = "PSAG_ACTOR_MAG";

	class GameActorActuatorManager {
	protected:
		std::unordered_map<size_t, GameActorCore::GameActorActuator*> GameActorDataset = {};
		std::vector<size_t> GameActorFreeList = {};
	public:
		GameActorActuatorManager() {};
		~GameActorActuatorManager();

		// get_source data(hash_map), pointer.
		std::unordered_map<size_t, GameActorCore::GameActorActuator*>* GetSourceData() { 
			return &GameActorDataset; 
		}

		size_t CreateGameActor(uint32_t actor_code, const GameActorCore::GameActorActuatorDESC& actor_desc);
		bool   DeleteGameActor(size_t unique_code);
		GameActorCore::GameActorActuator* FindGameActor(size_t unique_code);

		void UpdateManagerData();

		// update all actor_object "ActorUpdateHealth" & "ActorUpdate".
		// rendering all actor_object "ActorRendering".
		void RunAllGameActor();
	};
}

// 用于构建静态场景,比静态Actor更加高效.
namespace GameBrickCore {
	StaticStrLABEL PSAGM_BRICK_CORE_LABEL = "PSAG_BRICK_CORE";

	struct GameBrickActuatorDESC {
		// brick background(debug) color.
		Vector4T<float> BrickDebugColor;
		ImageRawData    BrickImageTexture;

		std::vector<Vector2T<float>> BrickVertexCollision;
		std::vector<Vector2T<float>> BrickVertexUvCoord;
	};

	class GameBrickActuator :
		public GraphicsEngineDataset::GLEngineStcVertexData,
		public GraphicsEngineDataset::GLEngineSmpTextureData,
		public PhysicsEngine::PhyEngineCoreDataset 
	{
	protected:

	public:

	};
}

namespace GameDebugGuiWindow {
	// debug window [panel], actor. 
	void DebugWindowGuiActor(const std::string& name, GameActorCore::GameActorActuator* actor);
	// debug window [panel], actor_manager(actors). 
	void DebugWindowGuiActors(std::unordered_map<size_t, GameActorCore::GameActorActuator*>* actors);
}

#endif