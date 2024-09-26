// psag_actor_module_brick.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameBrickCore {

	GameBrickExecutor::GameBrickExecutor(const GameBrickExecutorDESC& INIT_DESC) {
		PSAG_SYS_GENERATE_KEY GenResourceID;
		BrickUniqueID = GenResourceID.PsagGenUniqueKey();

#if ENABLE_DEBUG_MODE
		FTDcapture::CaptureContext CapPoint;

		CapPoint.CaptureSettingFilter(FTD_TagLv2);
		CapPoint.CaptureSettingPointer(this);
		CapPoint.CaptureSettingTagging("brick create.");
		CapPoint.CaptureBegin();
#endif
		if (INIT_DESC.BrickShaderResource == nullptr) {
			PushLogger(LogError, PSAGM_BRICK_CORE_LABEL, "game_brick shader_resource = nullptr.");
			return;
		}
		// shader_resolution.
		auto ResTemp = INIT_DESC.BrickShaderResource->__RENDER_RESOLUTION;
		RenderingResolution = Vector2T<float>((float)ResTemp.vector_x, (float)ResTemp.vector_y);

		BrickRenderRes = INIT_DESC.BrickShaderResource;

		if (INIT_DESC.EnableRendering) {
			BirckCompRendering = new GameComponents::ActorRendering();

			BirckCompRendering->RenderResolution = RenderingResolution;
			BirckCompRendering->ShaderIndex      = BrickRenderRes->__ACTOR_SHADER_ITEM;
			BirckCompRendering->VertexGroupIndex = BrickRenderRes->__ACTOR_VERTEX_ITEM;
			
			// load rendering texture.
			if (VirTextureExist(BrickRenderRes->__VIR_TEXTURE_ITEM)) {
				// rendering texture func.
				BirckCompRendering->RenderingTextureFunc = 
					[this](PsagShader shader) { BirckCompRendering->UpdateActorRenderingTexture(shader); };

				// virtual texture_unqiue, unifrom.
				BirckCompRendering->VirTexItem    = BrickRenderRes->__VIR_TEXTURE_ITEM;
				BirckCompRendering->VirTexUniform = BrickRenderRes->__VIR_UNIFORM_ITEM;
			}
		}
		else {
			// comp(empty_object): rendering.
			BirckCompRendering = new GameComponents::null::ActorRenderingNULL();
		}
		// birck => load physics world_item.
		if (PhysicsWorldFind(INIT_DESC.BrickPhysicsWorld) == nullptr) {
			PushLogger(LogError, PSAGM_BRICK_CORE_LABEL, "game_brick unable find world: %s", INIT_DESC.BrickPhysicsWorld.c_str());
			return;
		}
		BrickPhysicsWorld = INIT_DESC.BrickPhysicsWorld;

		BrickRenderParams.RenderColorBlend = INIT_DESC.InitialVertexColor;

		BrickRenderParams.RenderPosition    = INIT_DESC.InitialPosition;
		BrickRenderParams.RenderScale       = INIT_DESC.InitialScale;
		BrickRenderParams.RenderRotate      = INIT_DESC.InitialRotate;
		BrickRenderParams.RenderLayerHeight = INIT_DESC.InitialRenderLayer;
		// actor space_z value_clamp.
		BrickRenderParams.RenderLayerHeight = 
			PsagClamp(BrickRenderParams.RenderLayerHeight, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace);

		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode     = BrickUniqueID;
		ActorPhyConfig.CollVertexGroup     = PhysicsEngine::PresetVertexGroupRECT(); // default vertex_group.
		ActorPhyConfig.PhysicsModeTypeFlag = false;

		// 多边形碰撞,非传感器.
		ActorPhyConfig.PhyShapeType        = PhysicsEngine::POLYGON_TYPE;
		ActorPhyConfig.PhysicsIsSensorFlag = false;

		if (INIT_DESC.BrickShaderResource->__GET_VERTICES_RES() != nullptr)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.BrickShaderResource->__GET_VERTICES_RES());

		ActorPhyConfig.PhyBoxRotate         = INIT_DESC.InitialRotate;
		ActorPhyConfig.PhysicsCollisionFlag = INIT_DESC.EnableCollision;
		ActorPhyConfig.PhyBoxCollisionSize  = INIT_DESC.InitialScale;
		ActorPhyConfig.PhyBoxPosition       = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		// BcickPhysicsItem(PhyBodyKey) 由物理引擎分配.
		PhyBodyItemAlloc(BrickPhysicsWorld, &BcickPhysicsItem, ActorPhyConfig);
		PushLogger(LogInfo, PSAGM_BRICK_CORE_LABEL, "game_brick item create: %u", BrickUniqueID);
	}

	GameBrickExecutor::~GameBrickExecutor() {
		// free: sysem_components.
		if (BirckCompRendering != nullptr) delete BirckCompRendering;

		// free: physics system item.
		PhyBodyItemFree(BrickPhysicsWorld, BcickPhysicsItem);
		PushLogger(LogInfo, PSAGM_BRICK_CORE_LABEL, "game_brick item delete: %u", BrickUniqueID);
	}

	void GameBrickExecutor::BrickRendering() {
		// rendering brick shader_data.
		BirckCompRendering->UpdateActorRendering(BrickRenderParams, VirTimerCount);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}