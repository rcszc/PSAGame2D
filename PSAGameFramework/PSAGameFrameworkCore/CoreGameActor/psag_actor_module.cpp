// psag_actor_module.
#include <limits>
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

float           __ACTOR_MODULES_TIMESTEP::ActorModulesTimeStep   = 1.0f;
Vector2T<float> __ACTOR_MODULES_CAMERAPOS::ActorModulesCameraPos = {};

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
		// system actor default vert_shader.
		ShaderScript.vector_x = ActorShaderScript::PsagShaderScriptPublicVS;
		ShaderScript.vector_y = SHADER_FRAG;

		__RENDER_RESOLUTION = RESOLUTION;
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader create: %u x %u", RESOLUTION.vector_x, RESOLUTION.vector_y);
	}

	GameActorShader::~GameActorShader() {
		// delete virtual static vertex_dataset.
		VerStcDataItemFree(__ACTOR_VERTEX_ITEM);
		// 引用虚拟纹理情况下, 不由"GameActorShader"回收.
		if (!ReferVirTextureFlag)
			VirTextureItemFree(__VIR_TEXTURE_ITEM);
		// delete opengl shader.
		LLRES_Shaders->ResourceDelete(__ACTOR_SHADER_ITEM);
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader delete.");
	}

	bool GameActorShader::CreateShaderRes() {
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;

		ShaderProcess.ShaderLoaderPushVS(ShaderScript.vector_x, StringScript);
		ShaderProcess.ShaderLoaderPushFS(ShaderScript.vector_y, StringScript);

		if (ShaderProcess.CreateCompileShader()) {
			__ACTOR_SHADER_ITEM = GenResourceID.PsagGenTimeKey();
			LLRES_Shaders->ResourceStorage(__ACTOR_SHADER_ITEM, &ShaderProcess);
		}
		else {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed create.");
			return false;
		}

		float RenderScale = (float)__RENDER_RESOLUTION.vector_x / (float)__RENDER_RESOLUTION.vector_y;
		// porj matrix & scale ortho.
		glm::mat4 ProjectionMatrix = glm::ortho(
			-SystemRenderingOrthoSpace * RenderScale, SystemRenderingOrthoSpace * RenderScale,
			-SystemRenderingOrthoSpace, SystemRenderingOrthoSpace, -SystemRenderingOrthoSpace, SystemRenderingOrthoSpace
		);
		// convert: glm matrix => psag matrix.
		const float* glmmatptr = glm::value_ptr(ProjectionMatrix);
		memcpy_s(__ACTOR_MATRIX_ITEM.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		__ACTOR_VERTEX_ITEM = GenResourceID.PsagGenTimeKey();
		if (VerPosition != nullptr) {

			vector<float> DatasetTemp = {};
			for (size_t i = 0; i < VerPosition->size(); ++i) {
				// vertex group, std: GL_VERT_01.
				vector<float> VertexGroup = {
					// pos: vec3, color: vec4, uv: vec2, normal: vec3
					(*VerPosition)[i].vector_x, (*VerPosition)[i].vector_y, 0.0f,
					ShaderDebugColor.vector_x, ShaderDebugColor.vector_y, ShaderDebugColor.vector_z, ShaderDebugColor.vector_w,
					(*VerUvCoord)[i].vector_x, (*VerUvCoord)[i].vector_y,
					0.0f, 0.0f, 0.0f
				};
				DatasetTemp.insert(DatasetTemp.begin(), VertexGroup.begin(), VertexGroup.end());
			}
			// upload static dataset.
			VerStcDataItemAlloc(__ACTOR_VERTEX_ITEM, DatasetTemp);
		}
		else {
			vector<float> PresetTemp = {};
			PresetTemp.assign(PSAG_OGL_MAG::ShaderTemplateRect, PSAG_OGL_MAG::ShaderTemplateRect + PSAG_OGL_MAG::ShaderTemplateRectLen);
			VerStcDataItemAlloc(__ACTOR_VERTEX_ITEM, PresetTemp);
		}
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader resource create.");
		return true;
	}

	bool GameActorShader::CheckRepeatTex(VirTextureUnqiue virtex) {
		if (VirTextureExist(virtex)) {
			PushLogger(LogWarning, PSAGM_ACTOR_CORE_LABEL, "game_actor shader texture duplicate.");
			return false;
		}
		return true;
	}

	bool GameActorShader::ShaderLoadVertices(GameActorShaderVerticesDESC& VER_DESC) {
		if (VER_DESC.VertexShaderEnable) {
			if (VER_DESC.VertexShaderScript.empty()) {
				// vertex shader non-script.
				PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader vert_script empty.");
				return false;
			}
			ShaderScript.vector_x = VER_DESC.VertexShaderScript;
		}
		// position num = uv num.
		if (VER_DESC.ShaderVertexCollision.size() != VER_DESC.ShaderVertexUvCoord.size()) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader vert: pos_size != uv_num.");
			return false;
		}
		VerPosition = &VER_DESC.ShaderVertexCollision;
		VerUvCoord  = &VER_DESC.ShaderVertexUvCoord;
		// shader frag non-texture out color.
		ShaderDebugColor = VER_DESC.ShaderDebugColor;
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
			__VIR_TEXTURE_ITEM = GenResourceID.PsagGenTimeKey();
			// alloc virtual sampler texture.
			if (!VirTextureItemAlloc(__VIR_TEXTURE_ITEM, image)) {
				PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader failed load_image.");
				return false;
			}
			__VIR_UNIFORM_ITEM = SystemPresetUname();
			return true;
		}
		return false;
	}

	// ******************************** GameActorActuator ********************************

	inline void SysVector2FInter(const Vector2T<float>& src, Vector2T<float>& inter, float value) {
		inter.vector_x += (src.vector_x - inter.vector_x) * value;
		inter.vector_y += (src.vector_y - inter.vector_y) * value;
	}
	constexpr Vector2T<float> ActorMoveSpeedZERO(0.0f, 0.0f);
#define PSAGM_ACTOR_INTER 0.05f
#define PSAGM_ACTOR_FRICT 0.005f

#define PSAGM_FP32_LOSS std::numeric_limits<float>::epsilon()
	namespace system {

		void ActorSpaceTrans::UpdateActorSpaceTrans(Vector2T<float>& position, Vector2T<float>& scale, float& rotate) {
			// speed_sub: move, inter: scale.
			SysVector2FInter(ActorMoveSpeedZERO, ActorStateMoveVec, PSAGM_ACTOR_FRICT * CalcInterSpeed.vector_x * ActorModulesTimeStep);
			SysVector2FInter(ActorStateScale, scale, PSAGM_ACTOR_INTER * CalcInterSpeed.vector_z * ActorModulesTimeStep);
			// rotate speed sub(vir_friction).
			ActorStateRotate += (0.0f - ActorStateRotate) * PSAGM_ACTOR_FRICT * CalcInterSpeed.vector_y * ActorModulesTimeStep;

			auto PhysicsState = PhyBodyItemGet(PhysicsWorld, PhysicsBody);

			if (abs(ActorStateMoveVec.vector_x) > PSAGM_FP32_LOSS || abs(ActorStateMoveVec.vector_y) > PSAGM_FP32_LOSS)
				PhysicsState.BodySourcePointer->SetLinearVelocity(b2Vec2(ActorStateMoveVec.vector_x, ActorStateMoveVec.vector_y));

			if (abs(ActorStateRotate) > PSAGM_FP32_LOSS) {
				float CurrentAngularVelocity = PhysicsState.BodySourcePointer->GetAngularVelocity();
				float Torque = (ActorStateRotate - CurrentAngularVelocity) * PhysicsState.BodySourcePointer->GetInertia();
				PhysicsState.BodySourcePointer->ApplyTorque(Torque, true);
			}

			// rendering actor state.
			position = PhysicsState.BodyPositionMove;
			rotate   = PhysicsState.BodyRotateAngle;
		}

		void ActorHealthTrans::UpdateActorHealthTrans(const HealthFuncParams& params) {
			// state inter_calc.
			for (size_t i = 0; i < PSAG_HEALTH_STATE_NUM; ++i) {
				ActorHealthState[1][i] += (ActorHealthState[0][i] - ActorHealthState[1][i])
					* PSAGM_ACTOR_INTER * ActorHealthState[2][i] * ActorModulesTimeStep;
			}
			// calling hp processing functions.
			ActorHealthHandlerFunc(params);
		}

		void ActorRendering::UpdateActorRendering(
			const Vector2T<float>& position, const Vector2T<float>& scale, float rotate, float time_count
		) {
			auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderIndex);

			ShaderRender.RenderBindShader(ShaderTemp);
			VerStcOperFrameDraw(VertexGroupIndex);

			// framework preset uniform.
			ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix",        RenderMatrix);
			ShaderUniform.UniformVec2     (ShaderTemp, "RenderResolution", RenderResolution);

			ShaderUniform.UniformFloat(ShaderTemp, "ActorTime", time_count);
			ShaderUniform.UniformVec2 (ShaderTemp, "ActorPos",  position);
			ShaderUniform.UniformFloat(ShaderTemp, "ActorRot",  rotate);
			ShaderUniform.UniformVec2 (ShaderTemp, "ActorSize", scale);

			ShaderRender.RenderUnbindShader();
		}
	}

	GameActorActuator::GameActorActuator(uint32_t TYPE, const GameActorActuatorDESC& INIT_DESC) {
		if (TYPE == Type::ActorTypeNULL) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor type code = null.");
			return;
		}
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		ActorUniqueInfo.ActorUniqueCode = GenResourceID.PsagGenTimeKey();
		ActorUniqueInfo.ActorTypeCode   = TYPE;

		if (INIT_DESC.ActorShaderResource == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor shader_resource = nullptr.");
			return;
		}
		// shader_resolution.
		auto ResTemp = INIT_DESC.ActorShaderResource->__RENDER_RESOLUTION;
		RenderingResolution = Vector2T<float>((float)ResTemp.vector_x, (float)ResTemp.vector_y);
		// actor自身留有shader资源指针, 目前无操作. 2024_06_04.
		ActorResource = INIT_DESC.ActorShaderResource;

		if (INIT_DESC.EnableRendering) {
			ActorCompRendering = new system::ActorRendering();

			ActorCompRendering->RenderResolution = RenderingResolution;
			ActorCompRendering->ShaderIndex      = ActorResource->__ACTOR_SHADER_ITEM;
			ActorCompRendering->VertexGroupIndex = ActorResource->__ACTOR_VERTEX_ITEM;
			ActorCompRendering->RenderMatrix     = ActorResource->__ACTOR_MATRIX_ITEM;
		}

		if (PhysicsWorldFind(INIT_DESC.ActorPhysicsWorld) == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_CORE_LABEL, "game_actor unable find world: %s", INIT_DESC.ActorPhysicsWorld.c_str());
			return;
		}
		ActorPhysicsWorld = INIT_DESC.ActorPhysicsWorld;

		// inter speed_scale(base:1.0f)
		AnimInterSpeed.vector_x = INIT_DESC.ControlFricMove;
		AnimInterSpeed.vector_y = INIT_DESC.ControlFricRotate;
		AnimInterSpeed.vector_z = INIT_DESC.ControlFricScale;
		
		// create physics body.
		PhysicsEngine::PhysicsBodyConfig ActorPhyConfig;
		ActorPhyConfig.IndexUniqueCode = ActorUniqueInfo.ActorUniqueCode;
		ActorPhyConfig.CollVertexGroup = PhysicsEngine::PresetVertexGroupSqua(); // default vertex_group.

		switch (INIT_DESC.ActorPhysicalMode) {
		case(PhyMoveActor):  { ActorPhyConfig.PhysicsModeTypeFlag = true;  break; }
		case(PhyFixedActor): { ActorPhyConfig.PhysicsModeTypeFlag = false; break; }
		}

		if (INIT_DESC.ActorShaderResource->__GET_VERTICES_RES() != nullptr)
			ActorPhyConfig.CollVertexGroup = PhysicsEngine::VertexPosToBox2dVec(*INIT_DESC.ActorShaderResource->__GET_VERTICES_RES());

		ActorPhyConfig.PhyBoxRotate        = INIT_DESC.InitialRotate;
		ActorPhyConfig.PhyBoxCollisionSize = INIT_DESC.InitialScale;
		ActorPhyConfig.PhyBoxPosition      = INIT_DESC.InitialPosition;

		ActorPhyConfig.PhyBodyDensity  = INIT_DESC.InitialPhysics.vector_x;
		ActorPhyConfig.PhyBodyFriction = INIT_DESC.InitialPhysics.vector_y;

		ActorPhysicsItem = GenResourceID.PsagGenTimeKey();
		PhyBodyItemAlloc(ActorPhysicsWorld, ActorPhysicsItem, ActorPhyConfig);
		
		// config space_trans. ture: non-fixed.
		if (ActorPhyConfig.PhysicsModeTypeFlag == true) {
			ActorCompSpaceTrans = new system::ActorSpaceTrans(ActorPhysicsWorld, ActorPhysicsItem, AnimInterSpeed);
			// init move(speed_vec), rotate, scale.
			ActorCompSpaceTrans->ActorStateMoveVec = INIT_DESC.InitialSpeed;
			ActorCompSpaceTrans->ActorStateRotate  = INIT_DESC.InitialRotate;
			ActorCompSpaceTrans->ActorStateScale   = INIT_DESC.InitialScale;
		}
		ActorStatePosition = INIT_DESC.InitialPosition;
		ActorStateRotate   = INIT_DESC.InitialRotate;
		ActorStateScale    = INIT_DESC.InitialScale;

		// create hp comp.
		if (INIT_DESC.EnableHealth) {
			ActorCompHealthTrans = new system::ActorHealthTrans(INIT_DESC.ActorHealthSystem.HealthHandlerFunc);

			// config health system.
			size_t Bytes = PSAG_HEALTH_STATE_NUM * sizeof(float);
			memcpy(ActorCompHealthTrans->ActorHealthState[0], INIT_DESC.ActorHealthSystem.InitialHealthState, Bytes);
			memcpy(ActorCompHealthTrans->ActorHealthState[1], INIT_DESC.ActorHealthSystem.InitialHealthState, Bytes);
			memcpy(ActorCompHealthTrans->ActorHealthState[2], INIT_DESC.ActorHealthSystem.InitialHealthSpeed, Bytes);
		}
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor item create.");
	}

	GameActorActuator::~GameActorActuator() {
		// free: sysem_components.
		if (ActorCompSpaceTrans != nullptr)  delete ActorCompSpaceTrans;
		if (ActorCompHealthTrans != nullptr) delete ActorCompHealthTrans;
		if (ActorCompRendering != nullptr)   delete ActorCompRendering;

		// free: physics system item.
		PhyBodyItemFree(ActorPhysicsWorld, ActorPhysicsItem);
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
		
		// actor_position - camera_position.
		Vector2T<float> ActorMapping(
			ActorStatePosition.vector_x + ActorModulesCameraPos.vector_x / ValueScale,
			ActorStatePosition.vector_y + ActorModulesCameraPos.vector_y
		);
		return Vector2T<float>(
			ActorMapping.vector_x * ValueScale / SystemRenderingOrthoSpace * LossWidth + LossWidth,
			ActorMapping.vector_y / SystemRenderingOrthoSpace * LossHeight + LossHeight
		);
	}

	void GameActorActuator::ActorUpdateHealth() {
		if (ActorCompHealthTrans == nullptr)
			return;

		// in func parameters.
		HealthFuncParams FuncParams = {};
		FuncParams.ThisActorUniqueCode = ActorUniqueInfo.ActorUniqueCode;
		FuncParams.ActorCollision      = ActorCollisionInfo;

		FuncParams.ActorHealthStates = ActorCompHealthTrans->ActorHealthState[0];
		FuncParams.ActorHealthLength = PSAG_HEALTH_STATE_NUM;

		FuncParams.ActorPosition = ActorStatePosition;
		FuncParams.ActorSpeed = 
			ActorCompSpaceTrans == nullptr ? Vector2T<float>(0.0f, 0.0f) : ActorCompSpaceTrans->ActorStateMoveVec;

		ActorCompHealthTrans->UpdateActorHealthTrans(FuncParams);
	}

	void GameActorActuator::ActorUpdate() {
		if (ActorCompSpaceTrans != nullptr)
			ActorCompSpaceTrans->UpdateActorSpaceTrans(ActorStatePosition, ActorStateScale, ActorStateRotate);

		ActorPrivateINFO CollisionItem = {};
		// update collision info.
		CollisionItem.ActorTypeCode   = Type::ActorTypeNULL;
		CollisionItem.ActorUniqueCode = PhyBodyItemGetCollision(ActorPhysicsWorld, ActorPhysicsItem);
		ActorCollisionInfo = CollisionItem;
	}

	void GameActorActuator::ActorRendering() {
		if (ActorCompRendering == nullptr)
			return;
		// rendering actor shader_data.
		ActorCompRendering->UpdateActorRendering(ActorStatePosition, ActorStateScale, ActorStateRotate, VirTimerCount);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}