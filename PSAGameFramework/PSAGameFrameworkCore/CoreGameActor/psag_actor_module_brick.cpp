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
				BirckCompRendering->RenderingTextureNFunc = 
					[this](PsagShader shader) { BirckCompRendering->UpdateActorRenderingTextureN(shader); };

				// virtual texture_unqiue, unifrom.
				BirckCompRendering->VirTexture    = BrickRenderRes->__VIR_TEXTURE_ITEM;
				BirckCompRendering->VirTexUniform = BrickRenderRes->__VIR_UNIFORM_ITEM;

				bool HDRTEX_FLAG = false;
				// load hdr_blend texture.
				if (BrickRenderRes->__VIR_TEXTURE_HDR_ITEM != NULL && BirckCompRendering->VirTexture != NULL) {
					// load render_tex function.
					BirckCompRendering->RenderingTextureHFunc =
						[this](PsagShader shader) { BirckCompRendering->UpdateActorRenderingTextureH(shader); };

					BirckCompRendering->VirTextureHDR    = BrickRenderRes->__VIR_TEXTURE_HDR_ITEM;
					BirckCompRendering->VirTexUniformHDR = BrickRenderRes->__VIR_UNIFORM_HDR_ITEM;
					HDRTEX_FLAG = true;
				}
				PushLogger(LogInfo, PSAGM_BRICK_CORE_LABEL, "game_brick texture loading_completed, hdr: %d", 
					(uint32_t)HDRTEX_FLAG);
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
		BrickRenderParams.RenderRotate      = INIT_DESC.InitialAngle;
		BrickRenderParams.RenderLayerHeight = INIT_DESC.InitialRenderLayer;
		// actor space_z value_clamp.
		BrickRenderParams.RenderLayerHeight = 
			PsagClamp(BrickRenderParams.RenderLayerHeight, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace);

		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode     = BrickUniqueID;
		ActorPhyConfig.CollVertexGroup     = PhysicsEngine::PresetVertexGroupRECT(); // default vertex_group.

		if (INIT_DESC.CollisionBoxIsCircle)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupCIRCLE(INIT_DESC.InitialScale, 20);

		ActorPhyConfig.PhysicsModeTypeFlag = false;
		// 多边形碰撞,非传感器.
		ActorPhyConfig.PhysicalShapeType   = PhysicsEngine::POLYGON_TYPE;
		ActorPhyConfig.PhysicsIsSensorFlag = false;

		ActorPhyConfig.PhysicsCollisionThis   = GameActorCore::ActorCollisionGroup::ActorPhyGroup15;
		ActorPhyConfig.PhysicsCollisionFilter = GameActorCore::ActorCollisionGroup::ActorPhyGroup0;

		if (INIT_DESC.BrickShaderResource->__GET_VERTICES_RES() != nullptr)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.BrickShaderResource->__GET_VERTICES_RES());

		ActorPhyConfig.PhyBoxRotate         = INIT_DESC.InitialAngle;
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
		VirTimerStepSpeed = ActorModulesTimeStep;
		// rendering brick shader_data.
		BirckCompRendering->UpdateActorRendering(BrickRenderParams, VirTimerCount);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}