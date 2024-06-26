// psag_actor_module_brick.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameBrickCore {
	GameBrickActuator::~GameBrickActuator() {
		// free: physics system item.
		PhyBodyItemFree(BrickPhysicsWorld, BcickPhysicsItem);
		PushLogger(LogInfo, PSAGM_BRICK_CORE_LABEL, "game_brick item delete: %u", BrickUniqueID);
	}

	GameBrickActuator::GameBrickActuator(const GameBrickActuatorDESC& INIT_DESC) {
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		BrickUniqueID = GenResourceID.PsagGenTimeKey();
		
		if (INIT_DESC.BrickShaderResource == nullptr) {
			PushLogger(LogError, PSAGM_BRICK_CORE_LABEL, "game_brick shader_resource = nullptr.");
			return;
		}
		// shader_resolution.
		auto ResTemp = INIT_DESC.BrickShaderResource->__RENDER_RESOLUTION;
		RenderingResolution = Vector2T<float>((float)ResTemp.vector_x, (float)ResTemp.vector_y);

		BrickResource = INIT_DESC.BrickShaderResource;

		if (INIT_DESC.EnableRendering) {
			BirckCompRendering = new GameActorCore::system::ActorRendering();

			BirckCompRendering->RenderResolution = RenderingResolution;
			BirckCompRendering->ShaderIndex      = BrickResource->__ACTOR_SHADER_ITEM;
			BirckCompRendering->VertexGroupIndex = BrickResource->__ACTOR_VERTEX_ITEM;
			BirckCompRendering->RenderMatrix     = &MatrixDataWindow;

			// load rendering texture.
			if (VirTextureExist(BrickResource->__VIR_TEXTURE_ITEM)) {
				// rendering_tex_func, vir_tex_unqiue, unifrom.
				BirckCompRendering->RenderingTextureFunc = [this]() { BirckCompRendering->UpdateActorRenderingTexture(); };
				BirckCompRendering->VirTexItem           = BrickResource->__VIR_TEXTURE_ITEM;
				BirckCompRendering->VirTexUniform        = BrickResource->__VIR_UNIFORM_ITEM;
			}
		}
		else {
			BirckCompRendering =
				new GameActorCore::system::null::ActorRenderingNULL();
		}
		// birck => load physics world_item.
		if (PhysicsWorldFind(INIT_DESC.BrickPhysicsWorld) == nullptr) {
			PushLogger(LogError, PSAGM_BRICK_CORE_LABEL, "game_brick unable find world: %s", INIT_DESC.BrickPhysicsWorld.c_str());
			return;
		}
		BrickPhysicsWorld = INIT_DESC.BrickPhysicsWorld;

		BrickStaticPosition = INIT_DESC.InitialPosition;
		BrickStaticScale    = INIT_DESC.InitialScale;
		BrickStaticRotate   = INIT_DESC.InitialRotate;

		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode     = BrickUniqueID;
		ActorPhyConfig.CollVertexGroup     = PhysicsEngine::PresetVertexGroupSqua(); // default vertex_group.
		ActorPhyConfig.PhysicsModeTypeFlag = false;

		if (INIT_DESC.BrickShaderResource->__GET_VERTICES_RES() != nullptr)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.BrickShaderResource->__GET_VERTICES_RES());

		ActorPhyConfig.PhyBoxRotate        = INIT_DESC.InitialRotate;
		ActorPhyConfig.PhyBoxCollisionSize = INIT_DESC.InitialScale;
		ActorPhyConfig.PhyBoxPosition      = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		// 'key'由物理引擎分配.
		PhyBodyItemAlloc(BrickPhysicsWorld, &BcickPhysicsItem, ActorPhyConfig);

		PushLogger(LogInfo, PSAGM_BRICK_CORE_LABEL, "game_brick item create.");
	}

	void GameBrickActuator::BrickRendering() {
		if (BirckCompRendering == nullptr)
			return;
		// rendering brick shader_data.
		BirckCompRendering->UpdateActorRendering(
			GameActorCore::system::RenderingParams(BrickStaticPosition, BrickStaticScale, BrickStaticRotate), 
			VirTimerCount
		);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}