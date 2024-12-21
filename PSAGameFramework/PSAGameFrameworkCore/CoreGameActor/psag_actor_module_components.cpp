// psag_actor_module_components.
#include <typeinfo>
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameComponents {
#define PSAGM_ACTOR_INTER 0.05f

	ActorActionLogic::ActorActionLogic(ActorActionLogicBase* pointer) {
		if (pointer == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_COMP_LABEL, "cmop_action object_ptr = nullptr.");
			return;
		}
		const type_info& ObjectInfoRTTI = typeid(*pointer);
		ObjectName     = ObjectInfoRTTI.name();
		ObjectHashCode = ObjectInfoRTTI.hash_code();

		ActionLogicObject = pointer;
	}

	void ActorActionLogic::UpdateActorActionLogic(Actor* actor_this) {
		// action_logic run, params: actor this_pointer, framework time_step.
		ActionLogicObject->ActionLogicRun(actor_this, ActorModulesTimeStep);
	}

	ActorActionLogic::~ActorActionLogic() {
		if (ActionLogicNULL) return;
		if (ActionLogicObject == nullptr) {
			PushLogger(LogWarning, PSAGM_ACTOR_COMP_LABEL, "cmop_action failed delete, nullptr.");
			return;
		}
		delete ActionLogicObject;
	}

	ActorSpaceTrans::ActorSpaceTrans(const string& phy_world, PhyBodyKey phy_body, bool enable_force) :
		PhysicsWorld(phy_world), PhysicsBody(phy_body)
	{
		enable_force == true ?
			ActorTransFunc = [this](Vector2T<float>& position, float& rotate) { UpdateActorTransForce(position, rotate); } :
			ActorTransFunc = [this](Vector2T<float>& position, float& rotate) { UpdateActorTransVelocity(position, rotate); };
	}

	void ActorSpaceTrans::UpdateActorTransVelocity(Vector2T<float>& position, float& rotate) {
		auto PhysicsState = PhyBodyItemGet(PhysicsWorld, PhysicsBody);

		b2Vec2 BodyLinear  = b2Vec2(-ActorTransMoveValue.vector_x * 3.2f, -ActorTransMoveValue.vector_y * 3.2f);
		float  BodyAngular = -ActorTransRotateValue * 0.7f;

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

		b2Vec2 BodyForce = b2Vec2(
			-ActorTransMoveValue.vector_x * PhysicsState.BodySourcePointer->GetMass(),
			-ActorTransMoveValue.vector_y * PhysicsState.BodySourcePointer->GetMass()
		);
		b2Vec2 BodyCenter = PhysicsState.BodySourcePointer->GetWorldCenter();
		float  BodyTorque = -ActorTransRotateValue * PhysicsState.BodySourcePointer->GetMass();

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

	void ActorSpaceTrans::SetActorState(const Vector2T<float>& pos, float angle) {
		auto PhysicsState = PhyBodyItemGet(PhysicsWorld, PhysicsBody);
		PhysicsState.BodySourcePointer->SetTransform(b2Vec2(pos.vector_x, pos.vector_y), angle);
	}

	void ActorHealthTrans::UpdateActorHealthTrans() {
		// state lerp_calc. out += (stat - out) * speed * s.
		for (size_t i = 0; i < ActorHealthState.size(); ++i) {
			ActorHealthState[i].HealthSTATE = 
				PsagClamp(ActorHealthState[i].HealthSTATE, 0.0f, ActorHealthState[i].HealthMAX);
			// calc health_lerp.
			ActorHealthStateOut[i] += (ActorHealthState[i].HealthSTATE - ActorHealthStateOut[i]) * PSAGM_ACTOR_INTER 
				* ActorHealthState[i].HealthSPEED * ActorModulesTimeStep;
		}
	}

	void ActorHealthTrans::SetActorHealth(size_t count, float value) {
#if PSAG_DEBUG_MODE
		if (count >= ActorHealthState.size()) {
			PushLogger(LogError, PSAGM_ACTOR_COMP_LABEL, "cmop_health count >= size.");
			return;
		}
		ActorHealthState[count].HealthSTATE = value;
#else
		ActorHealthState[count].HealthSTATE = value;
#endif
	}

	void ActorRendering::UpdateActorRendering(const RenderingParams& params, float time_count) {
		auto ShaderTemp = GraphicShaders->ResourceFind(ShaderIndex);
		OGLAPI_OPER.RenderBindShader(ShaderTemp);

		// framework preset uniform.
		ShaderUniform.UniformVec2 (ShaderTemp, "RenderResolution", RenderResolution);
		ShaderUniform.UniformFloat(ShaderTemp, "RenderTime",       time_count);

		ShaderUniform.UniformVec2 (ShaderTemp, "ActorPos",    params.RenderPosition);
		ShaderUniform.UniformFloat(ShaderTemp, "ActorRotate", PSAG_M_DEGRAD(params.RenderRotate));
		ShaderUniform.UniformVec2 (ShaderTemp, "ActorSize",   params.RenderScale);
		ShaderUniform.UniformFloat(ShaderTemp, "ActorZ",      params.RenderLayerHeight);
		ShaderUniform.UniformVec4 (ShaderTemp, "ActorColor",  params.RenderColorBlend);

		RenderingTextureNFunc(ShaderTemp);
		RenderingTextureHFunc(ShaderTemp);

		VerStcOperFrameDraw(VertexGroupIndex);
		OGLAPI_OPER.RenderUnbindShader();
	}

	void ActorRendering::UpdateActorRenderingTextureN(PsagShader shader) {
		// draw virtual normal texture.
		VirTextureItemDraw(VirTexture, shader, VirTexUniform);
	}
	void ActorRendering::UpdateActorRenderingTextureH(PsagShader shader) {
		// draw virtual height texture.
		VirTextureItemDraw(VirTextureHDR, shader, VirTexUniformHDR);
	}

	Vector2T<float> ActorCoordConvert::ConvertSceneToWindow(Vector2T<uint32_t> window_size, Vector2T<float> position) {
		// scene virtual coord = > window coord.
		float ValueScale = (float)window_size.vector_y / (float)window_size.vector_x;
		float LossWidth  = window_size.vector_x * 0.5f;
		float LossHeight = window_size.vector_y * 0.5f;

		// position - camera_position.
		Vector2T<float> ActorMapping(
			position.vector_x - MatrixMainCamera.MatrixPosition.vector_x,
			position.vector_y + MatrixMainCamera.MatrixPosition.vector_y
		);
		return Vector2T<float>(
			ActorMapping.vector_x * ValueScale / (SystemRenderingOrthoSpace * MatrixMainCamera.MatrixScale.vector_x) * LossWidth + LossWidth,
			ActorMapping.vector_y / (SystemRenderingOrthoSpace * MatrixMainCamera.MatrixScale.vector_y) * LossHeight + LossHeight
		);
	}
}