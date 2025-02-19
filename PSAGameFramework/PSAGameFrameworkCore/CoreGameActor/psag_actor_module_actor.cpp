// psag_actor_module_actor.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

float __ACTOR_MODULES_TIMESTEP::ActorModulesTimeStep = 1.0f;
namespace GameActorCore {
	// "ActorComponentFlags" operator function.
	ActorComponentFlags operator|(ActorComponentFlags a, ActorComponentFlags b) {
		return static_cast<ActorComponentFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
	}
	ActorComponentFlags operator&(ActorComponentFlags a, ActorComponentFlags b) {
		return static_cast<ActorComponentFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
	}
	ActorComponentFlags& operator|=(ActorComponentFlags& a, ActorComponentFlags b) {
		a = static_cast<ActorComponentFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		return a;
	}
	// "ActorCollisionGroup" operator function.
	ActorCollisionGroup operator|(ActorCollisionGroup a, ActorCollisionGroup b) {
		return static_cast<ActorCollisionGroup>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
	}
	ActorCollisionGroup& operator|=(ActorCollisionGroup& a, ActorCollisionGroup b) {
		a = static_cast<ActorCollisionGroup>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
		return a;
	}
	ActorCollisionGroup operator&(ActorCollisionGroup a, ActorCollisionGroup b) {
		return static_cast<ActorCollisionGroup>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
	}
	ActorCollisionGroup operator~(ActorCollisionGroup v) {
		return static_cast<ActorCollisionGroup>(~static_cast<uint16_t>(v));
	}
	// game actor type_bind object.
	namespace Type {
		uint32_t GameActorTypeBind::ActorTypeIs(const string& type_name) {
			return (ActorTypeMapping.find(type_name) != ActorTypeMapping.end()) ? ActorTypeMapping[type_name] : ActorTypeNULL;
		}
#define ACTOR_TYPE_REG(name) PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "AType: register key: %s", type_name.c_str())
#define ACTOR_TYPE_DEL(name) PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "AType: delete key: %s", type_name.c_str())

		bool GameActorTypeBind::ActorTypeCreate(const string& type_name) {
			// type_code count, duplicate key not err & warn.
			bool NameValidFlag = ActorTypeIs(type_name) == ActorTypeNULL;
			if (NameValidFlag) {
				ActorTypeCount += 1 << 1;
				ActorTypeMapping[type_name] = ActorTypeCount;
				ACTOR_TYPE_REG(type_name.c_str());
			}
			return NameValidFlag;
		}

		bool GameActorTypeBind::ActorTypeDelete(const string& type_name) {
			auto it = ActorTypeMapping.find(type_name);
			bool NameValidFlag = it != ActorTypeMapping.end();
			if (NameValidFlag) {
				ActorTypeMapping.erase(it);
				ACTOR_TYPE_DEL(type_name.c_str());
				return ActorTypeCount;
			}
			return NameValidFlag;
		}
		GameActorTypeBind ActorTypeAllotter = {};
	}

	// ******************************** game actor_actuator ********************************

	GameActorExecutor::GameActorExecutor(uint32_t TYPE, const GameActorExecutorDESC& INIT_DESC) {
		if (TYPE == Type::ActorTypeNULL) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor type code = null.");
			return;
		}
		PSAG_SYS_GENERATE_KEY GenResourceID;
		ActorUniqueInfo.ActorUniqueCode = GenResourceID.PsagGenUniqueKey();
		ActorUniqueInfo.ActorTypeCode   = TYPE;

		if (INIT_DESC.ActorShaderResource == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader_resource = nullptr.");
			return;
		}
		// shader_resolution.
		auto ResTemp = INIT_DESC.ActorShaderResource->__RENDER_RESOLUTION;
		RenderingResolution = Vector2T<float>((float)ResTemp.vector_x, (float)ResTemp.vector_y);
		// actor自身留有shader资源指针, 目前无操作. 2024_06_04.
		ActorRenderRes = INIT_DESC.ActorShaderResource;
		
		if (INIT_DESC.ActorComponentConifg & ActorEnableRender) {
			ActorCompRendering = new GameComponents::ActorRendering();

			ActorCompRendering->RenderResolution = RenderingResolution;
			ActorCompRendering->ShaderIndex      = ActorRenderRes->__ACTOR_SHADER_ITEM;
			ActorCompRendering->VertexGroupIndex = ActorRenderRes->__ACTOR_VERTEX_ITEM;
			
			// read load shader => render comp params.
			if (!ActorRenderRes->__VIR_TEXTURES_GROUP.empty()) {
				ActorCompRendering->VirTextures = ActorRenderRes->__VIR_TEXTURES_GROUP;

				PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor get textures: %u",
					ActorCompRendering->VirTextures.size());
			}
			PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor render_comp init.");
		}
		else {
			// comp(empty_object): rendering.
			ActorCompRendering = new GameComponents::null::ActorRenderingNULL();
		}
		// actor => load physics world_item.
		if (PhysicsWorldFind(INIT_DESC.ActorPhysicsWorld) == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor unable find world: %s", 
				INIT_DESC.ActorPhysicsWorld.c_str());
			return;
		}
		ActorPhysicsWorld = INIT_DESC.ActorPhysicsWorld;

		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode = ActorUniqueInfo.ActorUniqueCode;
		ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupRECT(); // default vertex_group.

		if (INIT_DESC.CollisionBoxIsCircle)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupCIRCLE(INIT_DESC.InitialScale, 20);

		// 多边形碰撞,非传感器.
		ActorPhyConfig.PhysicalShapeType   = PhysicsEngine::POLYGON_TYPE;
		ActorPhyConfig.PhysicsIsSensorFlag = false;
		
		ActorPhyConfig.PhysicsCollisionThis   = INIT_DESC.ActorCollisionThis;
		ActorPhyConfig.PhysicsCollisionFilter = INIT_DESC.ActorCollisionFilter;

		switch (INIT_DESC.ActorPhysicalMode) {
		case(ActorPhysicsMove):  { ActorPhyConfig.PhysicsModeTypeFlag = true;  break; }
		case(ActorPhysicsFixed): { ActorPhyConfig.PhysicsModeTypeFlag = false; break; }
		}

		if (INIT_DESC.ActorShaderResource->__GET_VERTICES_RES() != nullptr)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.ActorShaderResource->__GET_VERTICES_RES());

		ActorPhyConfig.PhyBoxRotate         = INIT_DESC.InitialAngle;
		ActorPhyConfig.PhysicsCollisionFlag = INIT_DESC.ActorComponentConifg & ActorEnableCollision;
		ActorPhyConfig.PhyBoxCollisionSize  = INIT_DESC.InitialScale;
		ActorPhyConfig.PhyBoxPosition       = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		// ActorPhysicsItem(PhyBodyKey) 由物理引擎分配.
		if (!PhyBodyItemAlloc(ActorPhysicsWorld, &ActorPhysicsItem, ActorPhyConfig)) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor alloc phy_body err.");
			return;
		}
		// config space_trans. ture: non-fixed.
		if (ActorPhyConfig.PhysicsModeTypeFlag && INIT_DESC.ActorComponentConifg & ActorEnableTransform) {
			ActorCompSpaceTrans = new GameComponents::ActorSpaceTrans(ActorPhysicsWorld, ActorPhysicsItem, INIT_DESC.VectorCalcIsForce);
			// init move speed_vector. rotate_vector 20250124 RCSZ.
			ActorCompSpaceTrans->ActorTransMoveValue   = INIT_DESC.InitialSpeed;
			ActorCompSpaceTrans->ActorTransRotateValue = INIT_DESC.InitialAngleSpeed;
		}
		else {
			// comp(empty_object): space_trans.
			ActorCompSpaceTrans = new 
				GameComponents::null::ActorSpaceTransNULL(ActorPhysicsWorld, ActorPhysicsItem);
		}
		// init render params.
		ActorRenderParams.RenderColorBlend  = INIT_DESC.VertexColor;
		ActorRenderParams.RenderPosition    = INIT_DESC.InitialPosition;
		ActorRenderParams.RenderScale       = INIT_DESC.InitialScale;
		ActorRenderParams.RenderAngle       = INIT_DESC.InitialAngle;
		ActorRenderParams.RenderLayerHeight = INIT_DESC.InitialRenderLayer;

		// actor space_z value_clamp.
		ActorRenderParams.RenderLayerHeight = 
			PsagClamp(ActorRenderParams.RenderLayerHeight, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace);

		// create hp comp.
		if (INIT_DESC.ActorComponentConifg & ActorEnableHealth) {
			ActorCompHealthTrans = new GameComponents::ActorHealthTrans();
			// init health_state, copy hp_value.
			ActorCompHealthTrans->ActorHealthState = INIT_DESC.ActorHealthSystem.InitialActorHealth;
			// config health system.
			for (const auto& state : ActorCompHealthTrans->ActorHealthState)
				ActorCompHealthTrans->ActorHealthStateOut.push_back(state.HealthState);
		}
		else {
			// comp(empty_object): health_trans.
			ActorCompHealthTrans = new GameComponents::null::ActorHealthTransNULL();
		}

		// create action_logic comp.
		if (INIT_DESC.ActorComponentConifg & ActorEnableLogic) {
			ActorCompActionLogic = new GameComponents::ActorActionLogic(INIT_DESC.ActorLogicObject);
		}
		else {
			// comp(empty_object): action_logic.
			ActorCompActionLogic = new GameComponents::null::ActorActionLogicNULL();
		}
		// collision event callback_function.
		ActorCollision = INIT_DESC.CollisionCallbackFunc;
		// create coord_convert comp.
		ActorCompConvert = new GameComponents::ActorCoordConvert();
		// create-success => print_log.
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor entity create: %u", ActorUniqueInfo.ActorUniqueCode);

		// ATOMIC ENTITIES COUNTER.
		++ActorSystemAtomic::GLOBAL_PARAMS_ACTORS;
	}

	GameActorExecutor::~GameActorExecutor() {
		// free: sysem_components.
		if (ActorCompSpaceTrans  != nullptr) delete ActorCompSpaceTrans;
		if (ActorCompHealthTrans != nullptr) delete ActorCompHealthTrans;
		if (ActorCompRendering   != nullptr) delete ActorCompRendering;
		if (ActorCompConvert     != nullptr) delete ActorCompConvert;
		if (ActorCompActionLogic != nullptr) delete ActorCompActionLogic;

		// free: physics system item.
		PhyBodyItemFree(ActorPhysicsWorld, ActorPhysicsItem);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor entity delete: %u", ActorUniqueInfo.ActorUniqueCode);

		// ATOMIC ENTITIES COUNTER.
		--ActorSystemAtomic::GLOBAL_PARAMS_ACTORS;
	}

	float GameActorExecutor::ActorGetLifeTime() {
		return (float)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - ActorTimer).count() / 1000.0f;
	}

	Vector2T<float> GameActorExecutor::ActorMappingWindowCoord() {
		// actor virtual coord => window coord.
		return ActorCompConvert->ConvertSceneToWindow(
			Vector2T<uint32_t>(
				(uint32_t)RenderingResolution.vector_x, 
				(uint32_t)RenderingResolution.vector_y
			),
			ActorRenderParams.RenderPosition
		);
	}

	void GameActorExecutor::ActorUpdateHealth() {
		// actor health calc_value.
		ActorCompHealthTrans->UpdateActorHealthTrans();
	}

	void GameActorExecutor::ActorUpdate() {
		ActorLastPosition = ActorRenderParams.RenderPosition;
		ActorCompSpaceTrans->UpdateActorTrans(ActorRenderParams.RenderPosition, ActorRenderParams.RenderAngle);

		GameComponents::ActorPrivateINFO CollisionItem = {};
		// update collision info.
		CollisionItem.ActorTypeCode   = Type::ActorTypeNULL;
		CollisionItem.ActorUniqueCode = PhyBodyItemGetCollisionFirst(ActorPhysicsWorld, ActorPhysicsItem);
		ActorCollisionInfo = CollisionItem;

		if (CollisionItem.ActorUniqueCode != NULL)
			ActorCollision(this);
		// run logic event function.
		ActorCompActionLogic->UpdateActorActionLogic(this);
	}

	void GameActorExecutor::ActorRendering() {
		VirTimerStepSpeed = ActorModulesTimeStep;
		// rendering actor shader_data.
		ActorCompRendering->UpdateActorRendering(ActorRenderParams, VirTimerCount);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}

	GameActorCircleSensor::GameActorCircleSensor(const GameActorCircleSensorDESC& INIT_DESC) {
		// create physics body.
		PhysicsEngine::PhysicsBodyConfig SensorPhyConfig;

		SensorPhyConfig.IndexUniqueCode = ActorSensorUniqueID;
		SensorPhyConfig.CollVertexGroup = {}; // sensor: null.

		SensorPhyConfig.PhysicalShapeType   = PhysicsEngine::CIRCLE_TYPE;
		SensorPhyConfig.PhysicsIsSensorFlag = true;
		SensorPhyConfig.PhysicsModeTypeFlag = true;

		if (INIT_DESC.InitialRadius <= 0.0f || INIT_DESC.InitialScale <= 0.0f) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor(sensor): radius <= 0.0f | scale <= 0.0f");
			return;
		}
		SensorPhyConfig.PhyBoxRotate         = 0.0f;
		SensorPhyConfig.PhysicsCollisionFlag = true;
		SensorPhyConfig.PhyBoxCollisionSize  = Vector2T<float>(INIT_DESC.InitialRadius * INIT_DESC.InitialScale, 0.0f);
		SensorPhyConfig.PhyBoxPosition       = INIT_DESC.InitialPosition;

		SensorPhyConfig.PhyBodyDensity  = 1.0f;
		SensorPhyConfig.PhyBodyFriction = 1.0f;

		// SensorPhysicsItem(PhyBodyKey) 由物理引擎分配.
		PhyBodyItemAlloc(SensorPhysicsWorld, &SensorPhysicsItem, SensorPhyConfig);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor(sensor) entity create.");
	}

	GameActorCircleSensor::~GameActorCircleSensor() {
		// free: physics system item.
		PhyBodyItemFree(SensorPhysicsWorld, SensorPhysicsItem);
	}

	void GameActorCircleSensor::SensorModifyState(const Vector2T<float>& position) {
		PhyBodyItemGet(SensorPhysicsWorld, SensorPhysicsItem).
			BodySourcePointer->SetTransform(b2Vec2(position.vector_x, position.vector_y), 0.0f);
	}

	void GameActorCircleSensor::SensorUpdate() {
		GameComponents::ActorPrivateINFO CollisionItem = {};
		// update collision info.
		CollisionItem.ActorTypeCode   = Type::ActorTypeNULL;
		CollisionItem.ActorUniqueCode = PhyBodyItemGetCollisionFirst(SensorPhysicsWorld, SensorPhysicsItem);
		SensorCollisionInfo = CollisionItem;
	}
}