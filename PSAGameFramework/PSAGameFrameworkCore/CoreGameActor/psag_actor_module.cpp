// psag_actor_module.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

float __ACTOR_MODULES_TIMESTEP::ActorModulesTimeStep = 1.0f;
namespace GameActorCore {
	namespace Type {
		uint32_t GameActorTypeBind::ActorTypeIs(const string& type_name) {
			return (ActorTypeINFO.find(type_name) != ActorTypeINFO.end()) ? ActorTypeINFO[type_name] : ActorTypeNULL;
		}

		void GameActorTypeBind::ActorTypeCreate(const string& type_name) {
			// type_code count, duplicate key not err & warn.
			if (ActorTypeIs(type_name) == ActorTypeNULL) {
				ActorTypeCount += 1 << 1;
				ActorTypeINFO[type_name] = ActorTypeCount;
				PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "actor_type: register key: %s", type_name.c_str());
			}
		}

		void GameActorTypeBind::ActorTypeDelete(const string& type_name) {
			auto it = ActorTypeINFO.find(type_name);
			if (it != ActorTypeINFO.end()) {
				ActorTypeINFO.erase(it);
				PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "actor_type: delete key: %s", type_name.c_str());
			}
		}
		GameActorTypeBind ActorTypeAllotter = {};
	}

	// ******************************** game actor_actuator ********************************

	GameActorExecutor::GameActorExecutor(uint32_t TYPE, const GameActorExecutorDESC& INIT_DESC) {
		if (TYPE == Type::ActorTypeNULL) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor type code = null.");
			return;
		}
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		ActorUniqueInfo.ActorUniqueCode = GenResourceID.PsagGenTimeKey();
		ActorUniqueInfo.ActorTypeCode   = TYPE;

#if ENABLE_DEBUG_MODE
		FTDcapture::CaptureContext CapPoint;

		CapPoint.CaptureSettingFilter(FTD_TagLv3);
		CapPoint.CaptureSettingPointer(this);
		CapPoint.CaptureSettingTagging("actor create.");
		CapPoint.CaptureBegin();
#endif
		if (INIT_DESC.ActorShaderResource == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader_resource = nullptr.");
			return;
		}
		// shader_resolution.
		auto ResTemp = INIT_DESC.ActorShaderResource->__RENDER_RESOLUTION;
		RenderingResolution = Vector2T<float>((float)ResTemp.vector_x, (float)ResTemp.vector_y);
		// actor自身留有shader资源指针, 目前无操作. 2024_06_04.
		ActorRenderRes = INIT_DESC.ActorShaderResource;

		if (INIT_DESC.EnableRendering) {
			ActorCompRendering = new GameComponents::ActorRendering();

			ActorCompRendering->RenderResolution = RenderingResolution;
			ActorCompRendering->ShaderIndex      = ActorRenderRes->__ACTOR_SHADER_ITEM;
			ActorCompRendering->VertexGroupIndex = ActorRenderRes->__ACTOR_VERTEX_ITEM;
			ActorCompRendering->RenderMatrix     = &MatrixDataWindow;
			
			// load rendering texture.
			if (VirTextureExist(ActorRenderRes->__VIR_TEXTURE_ITEM)) {
				// rendering texture func.
				ActorCompRendering->RenderingTextureFunc = 
					[this](PsagShader shader) { ActorCompRendering->UpdateActorRenderingTexture(shader); };

				// virtual texture_unqiue, uniform.
				ActorCompRendering->VirTexItem    = ActorRenderRes->__VIR_TEXTURE_ITEM;
				ActorCompRendering->VirTexUniform = ActorRenderRes->__VIR_UNIFORM_ITEM;
			}
		}
		else {
			// comp(empty_object): rendering.
			ActorCompRendering = new GameComponents::null::ActorRenderingNULL();
		}
		// actor => load physics world_item.
		if (PhysicsWorldFind(INIT_DESC.ActorPhysicsWorld) == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor unable find world: %s", INIT_DESC.ActorPhysicsWorld.c_str());
			return;
		}
		ActorPhysicsWorld = INIT_DESC.ActorPhysicsWorld;

		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode = ActorUniqueInfo.ActorUniqueCode;
		ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupSqua(); // default vertex_group.

		switch (INIT_DESC.ActorPhysicalMode) {
		case(ActorPhysicsMove):  { ActorPhyConfig.PhysicsModeTypeFlag = true;  break; }
		case(ActorPhysicsFixed): { ActorPhyConfig.PhysicsModeTypeFlag = false; break; }
		}

		if (INIT_DESC.ActorShaderResource->__GET_VERTICES_RES() != nullptr)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.ActorShaderResource->__GET_VERTICES_RES());

		ActorPhyConfig.PhyBoxRotate         = INIT_DESC.InitialAngle;
		ActorPhyConfig.PhysicsCollisionFlag = INIT_DESC.EnableCollision;
		ActorPhyConfig.PhyBoxCollisionSize  = INIT_DESC.InitialScale;
		ActorPhyConfig.PhyBoxPosition       = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		// ActorPhysicsItem(PhyBodyKey) 由物理引擎分配.
		PhyBodyItemAlloc(ActorPhysicsWorld, &ActorPhysicsItem, ActorPhyConfig);
		
		// config space_trans. ture: non-fixed.
		if (ActorPhyConfig.PhysicsModeTypeFlag && INIT_DESC.EnableTrans) {
			ActorCompSpaceTrans = new GameComponents::ActorSpaceTrans(ActorPhysicsWorld, ActorPhysicsItem, INIT_DESC.ForceClacEnable);
			// init move(speed_vec), rotate, scale.
			ActorCompSpaceTrans->ActorTransMoveValue   = INIT_DESC.InitialSpeed;
			ActorCompSpaceTrans->ActorTransRotateValue = INIT_DESC.InitialRotate;
		}
		else {
			// comp(empty_object): space_trans.
			ActorCompSpaceTrans = new 
				GameComponents::null::ActorSpaceTransNULL(ActorPhysicsWorld, ActorPhysicsItem);
		}

		ActorTransPosition    = INIT_DESC.InitialPosition;
		ActorTransScale       = INIT_DESC.InitialScale;
		ActorTransRotateValue = INIT_DESC.InitialRotate;
		ActorTransLayer       = INIT_DESC.InitialRenderLayer;
		// actor space_z value_clamp.
		ActorTransLayer = PsagClamp(ActorTransLayer, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace);

		// create hp comp.
		if (INIT_DESC.EnableHealth) {
			ActorCompHealthTrans = new GameComponents::ActorHealthTrans();
			// init health_state, copy hp_value.
			ActorCompHealthTrans->ActorHealthState = INIT_DESC.ActorHealthSystem.InitialActorHealth;
			// config health system.
			for (const auto& state : ActorCompHealthTrans->ActorHealthState)
				ActorCompHealthTrans->ActorHealthStateOut.push_back(state.HealthSTATE);
		}
		else {
			// comp(empty_object): health_trans.
			ActorCompHealthTrans = new GameComponents::ActorHealthTrans();
		}

		// create action_logic comp.
		if (INIT_DESC.EnableLogic) {
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
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor entity create.");
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
	}

	float GameActorExecutor::ActorGetLifeTime() {
		return (float)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - ActorTimer).count() / 1000.0f;
	}

	Vector2T<float> GameActorExecutor::ActorConvertVirCoord(Vector2T<uint32_t> window_res) {
		// actor virtual coord => window coord.
		return ActorCompConvert->ConvertSceneToWindow(window_res, ActorTransPosition);
	}

	void GameActorExecutor::ActorUpdateHealth() {
		// actor health calc_value.
		ActorCompHealthTrans->UpdateActorHealthTrans();
	}

	void GameActorExecutor::ActorUpdate() {
		ActorCompSpaceTrans->UpdateActorTrans(ActorTransPosition, ActorTransRotateValue);

		GameComponents::ActorPrivateINFO CollisionItem = {};
		// update collision info.
		CollisionItem.ActorTypeCode   = Type::ActorTypeNULL;
		CollisionItem.ActorUniqueCode = PhyBodyItemGetCollisionFirst(ActorPhysicsWorld, ActorPhysicsItem);
		ActorCollisionInfo = CollisionItem;

		if (CollisionItem.ActorUniqueCode != NULL)
			ActorCollision(this);
#if ENABLE_DEBUG_MODE
		if (CollisionItem.ActorUniqueCode != NULL) {
			FTDcapture::CaptureContext CapPoint;

			CapPoint.CaptureSettingFilter(FTD_TagLv4);
			CapPoint.CaptureSettingPointer(this);
			CapPoint.CaptureSettingTagging("actor collision_flag.");
			CapPoint.CaptureBegin();
		}
#endif
		// run logic_event.
		ActorCompActionLogic->UpdateActorActionLogic(this);
	}

	void GameActorExecutor::ActorRendering() {
		// rendering actor shader_data.
		ActorCompRendering->UpdateActorRendering(
			GameComponents::RenderingParams(ActorTransPosition, ActorTransScale, ActorTransRotateValue, ActorTransLayer),
			VirTimerCount
		);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}