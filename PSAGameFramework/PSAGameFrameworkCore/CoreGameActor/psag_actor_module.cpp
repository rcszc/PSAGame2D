// psag_actor_module.
#include <limits>
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

	GameActorShader::GameActorShader(const std::string& SHADER_FRAG, const Vector2T<uint32_t>& RESOLUTION) {
		// system actor default vert_shader.
		ShaderScript.vector_x = GameActorScript::PsagShaderPublicVS;
		ShaderScript.vector_y = SHADER_FRAG;

		__RENDER_RESOLUTION = RESOLUTION;
		PushLogger(LogInfo, PSAGM_ACTOR_CORE_LABEL, "game_actor shader create: %u x %u", RESOLUTION.vector_x, RESOLUTION.vector_y);
	}

	GameActorShader::~GameActorShader() {
		// delete virtual static vertex_dataset.
		VerStcDataItemFree(__ACTOR_VERTEX_ITEM);
		// 引用虚拟纹理情况下, 不由"GameActorShader"回收.
		if (!ReferVirTextureFlag && __VIR_TEXTURE_ITEM != NULL)
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
		// find => shader_handle_temp => uniform.
		OpenGLShaderTemp = LLRES_Shaders->ResourceFind(__ACTOR_SHADER_ITEM);

		__ACTOR_VERTEX_ITEM = GenResourceID.PsagGenTimeKey();
		if (VerPosition != nullptr) {
			// vertex coord => shader vertex_group.
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
			__ACTOR_VERTEX_ITEM = GetPresetRect();
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

	// **************** upload shader uniform ****************
	
	void GameActorShader::UniformMatrix3x3(const char* name, const PsagMatrix3& matrix) {
		ShaderUniformLoader.UniformMatrix3x3(OpenGLShaderTemp, name, matrix);
	}
	void GameActorShader::UniformMatrix4x4(const char* name, const PsagMatrix4& matrix) {
		ShaderUniformLoader.UniformMatrix4x4(OpenGLShaderTemp, name, matrix);
	}
	void GameActorShader::UniformInt32(const char* name, const int32_t& value) {
		ShaderUniformLoader.UniformInteger(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformFP32(const char* name, const float& value) {
		ShaderUniformLoader.UniformFloat(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformVec2(const char* name, const Vector2T<float>& value) {
		ShaderUniformLoader.UniformVec2(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformVec3(const char* name, const Vector3T<float>& value) {
		ShaderUniformLoader.UniformVec3(OpenGLShaderTemp, name, value);
	}
	void GameActorShader::UniformVec4(const char* name, const Vector4T<float>& value) {
		ShaderUniformLoader.UniformVec4(OpenGLShaderTemp, name, value);
	}

	// ******************************** GameActorActuator ********************************

	namespace system {
#define PSAGM_ACTOR_INTER 0.05f

		ActorSpaceTrans::ActorSpaceTrans(const string& phy_world, PhyBodyKey phy_body, bool enable_force) :
			PhysicsWorld(phy_world), PhysicsBody(phy_body)
		{
			enable_force == true ?
				ActorTransFunc = [this](Vector2T<float>& position, float& rotate) { UpdateActorTransForce   (position, rotate); } : 
				ActorTransFunc = [this](Vector2T<float>& position, float& rotate) { UpdateActorTransVelocity(position, rotate); };
		}

		void ActorSpaceTrans::UpdateActorTransVelocity(Vector2T<float>& position, float& rotate) {
			auto PhysicsState = PhyBodyItemGet(PhysicsWorld, PhysicsBody);

			b2Vec2 BodyLinear  = b2Vec2(-ActorPawnMoveValue.vector_x * 3.2f, -ActorPawnMoveValue.vector_y * 3.2f);
			float  BodyAngular = -ActorPawnRotateValue * 0.7f;

			// apply force & torque (move,rotate).
			PhysicsState.BodySourcePointer->SetLinearVelocity(BodyLinear);
			PhysicsState.BodySourcePointer->SetAngularVelocity(BodyAngular);

			memcpy(ActorStateMoveSpeed.data(), &PhysicsState.BodySourcePointer->GetLinearVelocity().x, 2 * sizeof(float));
			ActorStateRotateSpeed = PhysicsState.BodySourcePointer->GetAngularVelocity();

			// rendering actor state.
			position = PhysicsState.BodyPositionMove;
			rotate   = PhysicsState.BodyRotateAngle;
		}

		void ActorSpaceTrans::UpdateActorTransForce(Vector2T<float>& position, float& rotate) {
			auto PhysicsState = PhyBodyItemGet(PhysicsWorld, PhysicsBody);

			b2Vec2 BodyForce  = b2Vec2(
				-ActorPawnMoveValue.vector_x * PhysicsState.BodySourcePointer->GetMass(), 
				-ActorPawnMoveValue.vector_y * PhysicsState.BodySourcePointer->GetMass()
			);
			b2Vec2 BodyCenter = PhysicsState.BodySourcePointer->GetWorldCenter();
			float  BodyTorque = -ActorPawnRotateValue * PhysicsState.BodySourcePointer->GetMass();

			// apply force & torque (move,rotate).
			PhysicsState.BodySourcePointer->ApplyForce(BodyForce, BodyCenter, true);
			PhysicsState.BodySourcePointer->ApplyTorque(BodyTorque, true);

			memcpy(ActorStateMoveSpeed.data(), &PhysicsState.BodySourcePointer->GetLinearVelocity().x, 2 * sizeof(float));
			ActorStateRotateSpeed = PhysicsState.BodySourcePointer->GetAngularVelocity();

			// rendering actor state.
			position = PhysicsState.BodyPositionMove;
			rotate   = PhysicsState.BodyRotateAngle;
		}

		void ActorSpaceTrans::UpdateActorTrans(Vector2T<float>& position, float& rotate) {
			ActorTransFunc(position, rotate);
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

		void ActorRendering::UpdateActorRendering(const RenderingParams& params, float time_count) {
			auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderIndex);
			ShaderRender.RenderBindShader(ShaderTemp);

			// framework preset uniform.
			ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix",        *RenderMatrix);
			ShaderUniform.UniformVec2     (ShaderTemp, "RenderResolution", RenderResolution);
			ShaderUniform.UniformFloat    (ShaderTemp, "RenderTime",       time_count);

			ShaderUniform.UniformVec2 (ShaderTemp, "ActorPos",  params.RenderPosition);
			ShaderUniform.UniformFloat(ShaderTemp, "ActorRot",  params.RenderRotate);
			ShaderUniform.UniformVec2 (ShaderTemp, "ActorSize", params.RenderScale);
			
			RenderingTextureFunc();
			VerStcOperFrameDraw(VertexGroupIndex);
			ShaderRender.RenderUnbindShader();
		}

		void ActorRendering::UpdateActorRenderingTexture() {
			auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderIndex);
			// draw virtual texture.
			VirTextureItemDraw(VirTexItem, ShaderTemp, VirTexUniform);
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
			ActorCompRendering->RenderMatrix     = &MatrixDataWindow;
			
			// load rendering texture.
			if (VirTextureExist(ActorResource->__VIR_TEXTURE_ITEM)) {
				// rendering_tex_func, vir_tex_unqiue, unifrom.
				ActorCompRendering->RenderingTextureFunc = [this]() { ActorCompRendering->UpdateActorRenderingTexture(); };
				ActorCompRendering->VirTexItem           = ActorResource->__VIR_TEXTURE_ITEM;
				ActorCompRendering->VirTexUniform        = ActorResource->__VIR_UNIFORM_ITEM;
			}
		}
		else {
			ActorCompRendering = 
				new system::null::ActorRenderingNULL();
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

		// 'key'由物理引擎分配.
		PhyBodyItemAlloc(ActorPhysicsWorld, &ActorPhysicsItem, ActorPhyConfig);
		
		// config space_trans. ture: non-fixed.
		if (ActorPhyConfig.PhysicsModeTypeFlag && INIT_DESC.EnablePawn) {
			ActorCompSpaceTrans = new system::ActorSpaceTrans(ActorPhysicsWorld, ActorPhysicsItem, INIT_DESC.ForceClacEnable);
			// init move(speed_vec), rotate, scale.
			ActorCompSpaceTrans->ActorPawnMoveValue   = INIT_DESC.InitialSpeed;
			ActorCompSpaceTrans->ActorPawnRotateValue = INIT_DESC.InitialRotate;
		}
		else {
			ActorCompSpaceTrans = 
				new system::null::ActorSpaceTransNULL(ActorPhysicsWorld, ActorPhysicsItem);
		}
		ActorStatePosition   = INIT_DESC.InitialPosition;
		ActorPawnRotateValue = INIT_DESC.InitialRotate;
		ActorPawnScale       = INIT_DESC.InitialScale;

		// create hp comp.
		if (INIT_DESC.EnableHealth) {
			ActorCompHealthTrans = new system::ActorHealthTrans(INIT_DESC.ActorHealthSystem.HealthHandlerFunc);

			// config health system.
			size_t Bytes = PSAG_HEALTH_STATE_NUM * sizeof(float);
			memcpy(ActorCompHealthTrans->ActorHealthState[0], INIT_DESC.ActorHealthSystem.InitialHealthState, Bytes);
			memcpy(ActorCompHealthTrans->ActorHealthState[1], INIT_DESC.ActorHealthSystem.InitialHealthState, Bytes);
			memcpy(ActorCompHealthTrans->ActorHealthState[2], INIT_DESC.ActorHealthSystem.InitialHealthSpeed, Bytes);
		}
		else {
			ActorCompHealthTrans = 
				new system::ActorHealthTrans(INIT_DESC.ActorHealthSystem.HealthHandlerFunc);
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
			ActorStatePosition.vector_x + MatrixWorldCamera.MatrixPosition.vector_x / 10.0f,
			ActorStatePosition.vector_y - MatrixWorldCamera.MatrixPosition.vector_y / 10.0f
		);
		return Vector2T<float>(
			ActorMapping.vector_x * ValueScale / (SystemRenderingOrthoSpace * MatrixWorldCamera.MatrixScale.vector_x) * LossWidth + LossWidth,
			ActorMapping.vector_y / (SystemRenderingOrthoSpace * MatrixWorldCamera.MatrixScale.vector_y) * LossHeight + LossHeight
		);
	}

	void GameActorActuator::ActorUpdateHealth() {
		// in func parameters.
		HealthFuncParams FuncParams = {};
		FuncParams.ThisActorUniqueCode = ActorUniqueInfo.ActorUniqueCode;
		FuncParams.ActorCollision      = ActorCollisionInfo;

		FuncParams.ActorHealthStates = ActorCompHealthTrans->ActorHealthState[0];
		FuncParams.ActorHealthLength = PSAG_HEALTH_STATE_NUM;

		FuncParams.ActorPosition = ActorStatePosition;
		FuncParams.ActorSpeed = 
			ActorCompSpaceTrans == nullptr ? Vector2T<float>(0.0f, 0.0f) : ActorCompSpaceTrans->ActorPawnMoveValue;

		ActorCompHealthTrans->UpdateActorHealthTrans(FuncParams);
	}

	void GameActorActuator::ActorUpdate() {
		ActorCompSpaceTrans->UpdateActorTrans(ActorStatePosition, ActorPawnRotateValue);

		ActorPrivateINFO CollisionItem = {};
		// update collision info.
		CollisionItem.ActorTypeCode   = Type::ActorTypeNULL;
		CollisionItem.ActorUniqueCode = PhyBodyItemGetCollision(ActorPhysicsWorld, ActorPhysicsItem);
		ActorCollisionInfo = CollisionItem;
	}

	void GameActorActuator::ActorRendering() {
		// rendering actor shader_data.
		ActorCompRendering->UpdateActorRendering(
			system::RenderingParams(ActorStatePosition, ActorPawnScale, ActorPawnRotateValue), 
			VirTimerCount
		);
		VirTimerCount += PSAGM_VIR_TICKSTEP_GL * VirTimerStepSpeed;
	}
}