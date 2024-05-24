// psag_actor_module.
#include <limits>
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

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

	GameActorShader::GameActorShader(const std::string& SHADER_FRAG, const Vector2T<uint32_t>& RESOLUTION) {
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		
		PsagLow::PsagSupGraphicsFunc::PsagGraphicsShader ShaderProcess;
		__ACTOR_SHADER_ITEM = to_string(GenResourceID.PsagGenTimeKey());

		ShaderProcess.ShaderLoaderPushVS(ActorShaderScript::PsagShaderScriptPublicVS, StringScript);
		ShaderProcess.ShaderLoaderPushFS(SHADER_FRAG,                                 StringScript);

		if (ShaderProcess.CreateCompileShader())
			LLRES_Shaders->ResourceStorage(__ACTOR_SHADER_ITEM, &ShaderProcess);

		float RenderScale = (float)RESOLUTION.vector_x / (float)RESOLUTION.vector_y;
		// porj matrix & scale ortho.
		glm::mat4 ProjectionMatrix = glm::ortho(
			-SystemRenderingOrthoSpace * RenderScale, SystemRenderingOrthoSpace * RenderScale,
			-SystemRenderingOrthoSpace, SystemRenderingOrthoSpace, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace
		);
		// convert: glm matrix => psag matrix.
		const float* glmmatptr = glm::value_ptr(ProjectionMatrix);
		memcpy_s(__ACTOR_MATRIX_ITEM.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		__WINDOW_RESOLUTION = RESOLUTION;
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader create: %u x %u", 
			RESOLUTION.vector_x, RESOLUTION.vector_y);
	}

	GameActorShader::~GameActorShader() {
		// 引用虚拟纹理情况下, 不由"GameActorShader"回收.
		if (!ReferVirTextureFlag)
			VirTextureItemFree(__VIR_TEXTURE_ITEM);
		LLRES_Shaders->ResourceDelete(__ACTOR_SHADER_ITEM);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader delete.");
	}

	bool GameActorShader::CheckRepeatTex(VirTextureUnqiue virtex) {
		if (VirTextureExist(virtex)) {
			PushLogger(LogWarning, PSAGM_ACTOR_CORE_LABEL, "game_actor shader texture duplicate.");
			return false;
		}
		return true;
	}

	bool GameActorShader::ShaderLoadVirTexture(VirTextureUnqiue virtex) {
		if (!CheckRepeatTex(__VIR_TEXTURE_ITEM)) return false;

		if (VirTextureExist(virtex)) {
			__VIR_TEXTURE_ITEM = virtex;
			__VIR_UNIFORM_ITEM = SystemPresetUname();
			ReferVirTextureFlag = true;
			return true;
		}
		PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader invalid vir_texture.");
		return false;
	}

	bool GameActorShader::ShaderLoadImage(const ImageRawData& image) {
		if (!CheckRepeatTex(__VIR_TEXTURE_ITEM)) return false;

		if (!image.ImagePixels.empty()) {
			PSAG_SYSGEN_TIME_KEY GenResourceID;
			__VIR_TEXTURE_ITEM = to_string(GenResourceID.PsagGenTimeKey());
			// alloc virtual sampler texture.
			if (!VirTextureItemAlloc(__VIR_TEXTURE_ITEM, image)) {
				PushLogger(LogError, PSAGM_GLENGINE_POST_LABEL, "game_actor shader failed load_image.");
				return false;
			}
			__VIR_UNIFORM_ITEM = SystemPresetUname();
			return true;
		}
		return false;
	}

#define PSAGM_ACTOR_INTER     0.05f
#define PSAGM_ACTOR_INTER_SUB 0.005f
	constexpr Vector2T<float> ActorMoveSpeedZERO(0.0f, 0.0f);

	GameActorActuator::GameActorActuator(uint32_t TYPE, const GameActorActuatorDESC& INIT_DESC) {
		if (TYPE == Type::ActorTypeNULL) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor type code = null.");
			return;
		}
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		ActorUniqueInfo.ActorUniqueCode = GenResourceID.PsagGenTimeKey();
		ActorUniqueInfo.ActorTypeCode   = TYPE;

		if (INIT_DESC.ActorShaderResource == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader resource = nullptr.");
			return;
		}
		ActorShaderItem = INIT_DESC.ActorShaderResource->__ACTOR_SHADER_ITEM;
		RenderingMatrix = INIT_DESC.ActorShaderResource->__ACTOR_MATRIX_ITEM;

		RenderingResolution.vector_x = (float)INIT_DESC.ActorShaderResource->__WINDOW_RESOLUTION.vector_x;
		RenderingResolution.vector_y = (float)INIT_DESC.ActorShaderResource->__WINDOW_RESOLUTION.vector_y;

		// inter speed_scale(base:1.0f)
		AnimInterSpeed.vector_x = INIT_DESC.ControlFrictionSpeedMove;
		AnimInterSpeed.vector_y = INIT_DESC.ControlFrictionSpeedRotate;
		AnimInterSpeed.vector_z = INIT_DESC.ControlFrictionSpeedScale;

		// config initial state.
		ActorStateScale[0] = INIT_DESC.InitialScale;
		ActorStateScale[1] = INIT_DESC.InitialScale;

		ActorStateMove[0] = INIT_DESC.InitialSpeed;
		ActorStateMove[1] = INIT_DESC.InitialPosition;
		ActorStateRotate  = Vector2T<float>(INIT_DESC.InitialRotate, INIT_DESC.InitialRotate);
		
		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode = ActorUniqueInfo.ActorUniqueCode;

		switch (INIT_DESC.ActorPhysicsMode) {
		case(PhyMoveActor):  { ActorPhyConfig.PhysicsModeTypeFlag = true;  break; }
		case(PhyFixedActor): { ActorPhyConfig.PhysicsModeTypeFlag = false; break; }
		}

		ActorPhyConfig.PhyBoxRotate = INIT_DESC.InitialRotate;
		ActorPhyConfig.PhyBoxCollisionSize = 
			Vector2T<float>(10.0f * INIT_DESC.InitialScale.vector_x, 10.0f * INIT_DESC.InitialScale.vector_y);
		ActorPhyConfig.PhyBoxPosition = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		ActorPhysicsItem = to_string(GenResourceID.PsagGenTimeKey());
		PhyBodyItemAlloc(ActorPhysicsItem, ActorPhyConfig);

		ActorPhysicsDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhysicsFriction = INIT_DESC.InitialPhysics.vector_y;

		// config health system.
		memcpy(ActorHealthState[0], INIT_DESC.ActorHealthSystem.InitialHealthState, PSAG_HEALTH_STATE_NUM * sizeof(float));
		memcpy(ActorHealthState[1], INIT_DESC.ActorHealthSystem.InitialHealthState, PSAG_HEALTH_STATE_NUM * sizeof(float));
		memcpy(ActorHealthState[2], INIT_DESC.ActorHealthSystem.InitialHealthSpeed, PSAG_HEALTH_STATE_NUM * sizeof(float));

		ActorHealthHandlerFunc = INIT_DESC.ActorHealthSystem.HealthHandlerFunc;

		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor item create.");
	}

	GameActorActuator::~GameActorActuator() {
		// free: physics system item.
		PhyBodyItemFree(ActorPhysicsItem);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor item delete: %u", ActorUniqueInfo.ActorUniqueCode);
	}

	float GameActorActuator::ActorGetLifeTime() {
		return (float)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - ActorTimer).count() / 1000.0f;
	}

	Vector2T<float> GameActorActuator::ActorConvertVirCoord(Vector2T<uint32_t> window_res) {
		// actor virtual coord => window coord.
		float ValueScale = (float)window_res.vector_y / (float)window_res.vector_x;
		float LossWidth  = window_res.vector_x * 0.5f;
		float LossHeight = window_res.vector_y * 0.5f;

		return Vector2T<float>(
			ActorStateMove[1].vector_x * ValueScale / SystemRenderingOrthoSpace * LossWidth + LossWidth,
			ActorStateMove[1].vector_y / SystemRenderingOrthoSpace * LossHeight + LossHeight
		);
	}

	void GameActorActuator::ActorUpdateHealth() {
		// state trans calc.
		for (size_t i = 0; i < PSAG_HEALTH_STATE_NUM; ++i)
			ActorHealthState[1][i] += (ActorHealthState[0][i] - ActorHealthState[1][i]) * PSAGM_ACTOR_INTER * ActorHealthState[2][i];

		// in func parameters.
		HealthFuncParams FuncParams = {};
		FuncParams.ThisActorUniqueCode = ActorUniqueInfo.ActorUniqueCode;
		FuncParams.ActorCollision      = ActorCollisionINFO;

		FuncParams.ActorHealthStates = ActorHealthState[0];
		FuncParams.ActorHealthLength = PSAG_HEALTH_STATE_NUM;

		FuncParams.ActorPosition = ActorStateMove[1];
		FuncParams.ActorSpeed    = ActorStateMove[0];

		// calling hp processing functions.
		ActorHealthHandlerFunc(FuncParams);
	}

	inline void SysVector2FInter(const Vector2T<float>& src, Vector2T<float>& inter, float value) {
		inter.vector_x += (src.vector_x - inter.vector_x) * value;
		inter.vector_y += (src.vector_y - inter.vector_y) * value;
	}

#define PSAGM_FP32_LOSS std::numeric_limits<float>::epsilon()
	void GameActorActuator::ActorUpdate() {
		// speed_sub: move, inter: scale.
		SysVector2FInter(ActorMoveSpeedZERO, ActorStateMove[0], PSAGM_ACTOR_INTER_SUB * AnimInterSpeed.vector_x);
		SysVector2FInter(ActorStateScale[0], ActorStateScale[1], PSAGM_ACTOR_INTER * AnimInterSpeed.vector_z);
		// rotate speed sub(vir_friction).
		ActorStateRotate.vector_x += (0.0f - ActorStateRotate.vector_x) * PSAGM_ACTOR_INTER_SUB * AnimInterSpeed.vector_x;

		auto PhysicsState = PhyBodyItemGet(ActorPhysicsItem);

		b2Vec2 DesiredVelocity = b2Vec2(ActorStateMove[0].vector_x, ActorStateMove[0].vector_y);
		b2Vec2 CurrentVelocity = PhysicsState.BodySourcePointer->GetLinearVelocity();
		b2Vec2 Force = (DesiredVelocity - CurrentVelocity);

		Force.x *= PhysicsState.BodySourcePointer->GetMass();
		Force.y *= PhysicsState.BodySourcePointer->GetMass();
		PhysicsState.BodySourcePointer->ApplyForceToCenter(Force, true);

		if (abs(ActorStateRotate.vector_x) > PSAGM_FP32_LOSS) {
			float CurrentAngularVelocity = PhysicsState.BodySourcePointer->GetAngularVelocity();
			float Torque = (ActorStateRotate.vector_x - CurrentAngularVelocity) * PhysicsState.BodySourcePointer->GetInertia();
			PhysicsState.BodySourcePointer->ApplyTorque(Torque, true);
		}

		// rendering actor state.
		ActorStateMove[1] = PhysicsState.BodyPositionMove;
		ActorStateRotate.vector_y = PhysicsState.BodyRotateAngle;

		ActorPrivateINFO CollisionItem = {};
		// update collision info.
		CollisionItem.ActorTypeCode   = Type::ActorTypeNULL;
		CollisionItem.ActorUniqueCode = PhyBodyItemGetCollision(ActorPhysicsItem);
		ActorCollisionINFO = CollisionItem;
	}

	void GameActorActuator::ActorRendering() {
		auto ShaderTemp = LLRES_Shaders->ResourceFind(ActorShaderItem);

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextBind(ShaderTemp);
		StaticVertexFrameDraw();

		// framework preset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix", RenderingMatrix);
		ShaderUniform.UniformVec2     (ShaderTemp, "RenderResolution", RenderingResolution);

		ShaderUniform.UniformFloat(ShaderTemp, "ActorTime", VirTimerCount);
		ShaderUniform.UniformVec2 (ShaderTemp, "ActorMove", ActorStateMove[1]);
		// rotate angle =convet=> radian.
		ShaderUniform.UniformFloat(ShaderTemp, "ActorRotate", ActorStateRotate.vector_y);
		ShaderUniform.UniformVec2 (ShaderTemp, "ActorScale",  ActorStateScale[1]);

		PsagLow::PsagSupGraphicsFunc::PsagGraphicsFuncShaderContextUnbind();
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}