// psag_actor_module_envmt.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameEnvmtCore {

	GameEnvmtExecutor::GameEnvmtExecutor(const GameEnvmtExecutorDESC& INIT_DESC) {
		PSAG_SYS_GENERATE_KEY GenResourceID;
		EnvmtEntryUniqueID = GenResourceID.PsagGenUniqueKey();

		if (INIT_DESC.EnableRendering) {
			if (INIT_DESC.EnvmtShaderResource == nullptr) {
				PushLogger(LogError, PSAGM_ENVMT_CORE_LABEL, "game_envmt shader_resource = nullptr.");
				return;
			}
			// shader_sys => get resolution.
			auto ResTemp = INIT_DESC.EnvmtShaderResource->__RENDER_RESOLUTION;
			RenderingResolution = Vector2T<float>((float)ResTemp.vector_x, (float)ResTemp.vector_y);

			EnvmtRenderRes = INIT_DESC.EnvmtShaderResource;
			EnvmtCompRendering = new GameComponents::ActorRendering();

			EnvmtCompRendering->RenderResolution = RenderingResolution;
			EnvmtCompRendering->ShaderIndex      = EnvmtRenderRes->__ACTOR_SHADER_ITEM;
			EnvmtCompRendering->VertexGroupIndex = EnvmtRenderRes->__ACTOR_VERTEX_ITEM;
			
			// read load shader => render comp params.
			if (!EnvmtRenderRes->__VIR_TEXTURES_GROUP.empty()) {
				EnvmtCompRendering->VirTextures = EnvmtRenderRes->__VIR_TEXTURES_GROUP;

				PushLogger(LogInfo, PSAGM_ENVMT_CORE_LABEL, "game_envmt get textures: %u",
					EnvmtCompRendering->VirTextures.size());
			}
			PushLogger(LogInfo, PSAGM_ENVMT_CORE_LABEL, "game_envmt render_comp init.");
		}
		else {
			// comp(empty_object): rendering.
			EnvmtCompRendering = new GameComponents::null::ActorRenderingNULL();
		}
		// birck => load physics world_item.
		if (PhysicsWorldFind(INIT_DESC.EnvmtPhysicsWorld) == nullptr) {
			PushLogger(LogError, PSAGM_ENVMT_CORE_LABEL, "game_envmt unable find world: %s", INIT_DESC.EnvmtPhysicsWorld.c_str());
			return;
		}
		EnvmtPhysicsWorld = INIT_DESC.EnvmtPhysicsWorld;

		EnvmtRenderParams.RenderColorBlend = INIT_DESC.VertexColor;

		EnvmtRenderParams.RenderPosition    = INIT_DESC.InitialPosition;
		EnvmtRenderParams.RenderScale       = INIT_DESC.InitialScale;
		EnvmtRenderParams.RenderAngle       = INIT_DESC.InitialAngle;
		EnvmtRenderParams.RenderLayerHeight = INIT_DESC.InitialRenderLayer;
		// actor space_z value_clamp.
		EnvmtRenderParams.RenderLayerHeight = 
			PsagClamp(EnvmtRenderParams.RenderLayerHeight, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace);

		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode = EnvmtEntryUniqueID;
		ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupRECT(); // default vertex_group.

		if (INIT_DESC.CollisionBoxIsCircle)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupCIRCLE(INIT_DESC.InitialScale, 20);

		ActorPhyConfig.PhysicsModeTypeFlag = false;
		// 多边形碰撞, 非传感器.
		ActorPhyConfig.PhysicalShapeType   = PhysicsEngine::POLYGON_TYPE;
		ActorPhyConfig.PhysicsIsSensorFlag = false;

		ActorPhyConfig.PhysicsCollisionThis   = GameActorCore::ActorCollisionGroup(1 << 15);
		ActorPhyConfig.PhysicsCollisionFilter = GameActorCore::ActorCollisionGroup::ActorPhyGroupALL;

		if (INIT_DESC.EnvmtShaderResource != nullptr &&
			INIT_DESC.EnvmtShaderResource->__GET_VERTICES_RES() != nullptr
		) {
			// render vert => collision vert.
			ActorPhyConfig.CollVertexGroup =
				PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.EnvmtShaderResource->__GET_VERTICES_RES());
		}
		ActorPhyConfig.PhyBoxRotate         = INIT_DESC.InitialAngle;
		ActorPhyConfig.PhysicsCollisionFlag = INIT_DESC.EnableCollision;
		ActorPhyConfig.PhyBoxCollisionSize  = INIT_DESC.InitialScale;
		ActorPhyConfig.PhyBoxPosition       = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		// BcickPhysicsItem(PhyBodyKey) 由物理引擎分配.
		PhyBodyItemAlloc(EnvmtPhysicsWorld, &EnvmtPhysicsItem, ActorPhyConfig);
		PushLogger(LogInfo, PSAGM_ENVMT_CORE_LABEL, "game_envmt item create: %u", EnvmtEntryUniqueID);

		// ATOMIC ENTITIES COUNTER.
		++ActorSystemAtomic::GLOBAL_PARAMS_EVNS;
	}

	GameEnvmtExecutor::~GameEnvmtExecutor() {
		// free: sysem_components.
		if (EnvmtCompRendering != nullptr) delete EnvmtCompRendering;

		// free: physics system item.
		PhyBodyItemFree(EnvmtPhysicsWorld, EnvmtPhysicsItem);
		PushLogger(LogInfo, PSAGM_ENVMT_CORE_LABEL, "game_envmt item delete: %u", EnvmtEntryUniqueID);

		// ATOMIC ENTITIES COUNTER.
		--ActorSystemAtomic::GLOBAL_PARAMS_EVNS;
	}

	void GameEnvmtExecutor::EnvmtRendering() {
		VirTimerStepSpeed = ActorModulesTimeStep;
		// rendering brick shader_data.
		EnvmtCompRendering->UpdateActorRendering(EnvmtRenderParams, VirTimerCount);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}