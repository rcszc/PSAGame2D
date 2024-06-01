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
		ActorPhyConfig.IndexUniqueCode = BrickUniqueID;
		ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupSqua();

		if (INIT_DESC.BrickShaderResource->__GET_VERTICES_RES() != nullptr)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.BrickShaderResource->__GET_VERTICES_RES());

		ActorPhyConfig.PhyBoxRotate        = INIT_DESC.InitialRotate;
		ActorPhyConfig.PhyBoxCollisionSize = INIT_DESC.InitialScale;
		ActorPhyConfig.PhyBoxPosition      = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		BcickPhysicsItem = GenResourceID.PsagGenTimeKey();
		PhyBodyItemAlloc(BrickPhysicsWorld, BcickPhysicsItem, ActorPhyConfig);

		PushLogger(LogInfo, PSAGM_BRICK_CORE_LABEL, "game_brick item create.");
	}

	void GameBrickActuator::BrickRendering() {
		auto ShaderTemp = LLRES_Shaders->ResourceFind(BrickResource->__ACTOR_SHADER_ITEM);

		ShaderRender.RenderBindShader(ShaderTemp);
		VerStcOperFrameDraw(BrickResource->__ACTOR_VERTEX_ITEM);

		// framework preset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix", BrickResource->__ACTOR_MATRIX_ITEM);
		ShaderUniform.UniformVec2     (ShaderTemp, "RenderResolution", RenderingResolution);

		ShaderUniform.UniformFloat(ShaderTemp, "ActorTime", VirTimerCount);
		ShaderUniform.UniformVec2 (ShaderTemp, "ActorMove", BrickStaticPosition);
		// rotate angle =convet=> radian.
		ShaderUniform.UniformFloat(ShaderTemp, "ActorRotate", BrickStaticRotate);
		ShaderUniform.UniformVec2 (ShaderTemp, "ActorScale",  BrickStaticScale);

		ShaderRender.RenderUnbindShader();
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}