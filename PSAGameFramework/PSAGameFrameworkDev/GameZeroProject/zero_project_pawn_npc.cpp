// zero_project_pawn_npc.
#include "zero_project_pawn.h"

using namespace std;
using namespace PSAG_LOGGER;

void ZPGamePawnActor::CreateNpcActor(size_t number) {
	for (size_t i = 0; i < number; ++i) {
		CreateNpcActorEntity(Vector2T<float>(
			PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-420.0f, 420.0f),
			PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-420.0f, 420.0f)
		));
	}
}

class ActorLogicNPC :public PsagActor::ActorLogicBase {
protected:
	PsagManager::Tools::Timer::GameCycleTimer Timer = {};
	Vector2T<float> TargetPosition = {};
public:
	void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {
		// 5.0s update target position.
		if (Timer.CycleTimerGetFlag()) {
			// random pos(x, y).
			TargetPosition.vector_x = PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-420.0f, 420.0f);
			TargetPosition.vector_y = PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-420.0f, 420.0f);
			Timer.CycleTimerClearReset(5000.0f);
		}
		auto DIST = PsagManager::Maths::CalcFuncPointsDistance(
			actor_object->ActorGetPosition(), TargetPosition
		);
		actor_object->ActorApplyForceMove(Vector2T<float>(
			-(TargetPosition.vector_x - actor_object->ActorGetPosition().vector_x) * 0.0012f,
			-(TargetPosition.vector_y - actor_object->ActorGetPosition().vector_y) * 0.0012f
		));
		auto Refw = actor_object->ActorMappingWindowCoord();

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.42f, 0.0f, 1.0f, 0.42f));

		ImGui::SetNextWindowPos(ImVec2(Refw.vector_x, Refw.vector_y));
		ImGui::SetNextWindowSize(ImVec2(160.0f, 78.0f));
		ImGuiWindowFlags Flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
		ImGui::Begin(
			to_string(actor_object->ActorGetPrivate().ActorUniqueCode).c_str(),
			(bool*)NULL, Flags
		);
		ImGui::ProgressBar(
			actor_object->ActorGetHealth(0) / actor_object->ActorGetHealthMax(0),
			ImVec2(-(1.1754944E-38F), 20.0f)
		);

		const ImVec4 TextColor(0.0f, 1.0f, 0.72f, 0.92f);
		// debug display value.
		if (ImPsag::GetDebugGuiFlag())
			ImGui::TextColored(TextColor, "HP: %.2f", actor_object->ActorGetHealth(0));
		ImGui::End();
		ImGui::PopStyleColor();
	}
};

void ZPGamePawnActor::CreateNpcActorEntity(const Vector2T<float>& position) {
	// create npc pawn actor shader.
	PsagActor::ActorDESC NpcActorDESC;
	NpcActorDESC.ActorPhysicsWorld = "ZPGamePhysics";

	PsagActor::ActorHpDESC NpcActorHealthDESC;

	NpcActorHealthDESC.InitialActorHealth.push_back(
		PsagActor::ActorHP(NpcActorParams.StateMaxHealth, 0.55f, NpcActorParams.StateMaxHealth)
	);
	NpcActorDESC.InitialRenderLayer  = 10.0f;
	NpcActorDESC.ActorHealthSystem   = NpcActorHealthDESC;
	NpcActorDESC.ActorShaderResource = ActorShaders.Get()->FindActorShader("NpcActor.A");

	NpcActorDESC.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup1;
	NpcActorDESC.ActorCollisionFilter = PsagActor::ActorFlagsPhy::ActorPhyGroupALL;

	ActorLogicNPC* LogicObject = new ActorLogicNPC();

	NpcActorDESC.ActorComponentConifg |= PsagActor::ActorFlagsComp::ActorEnableLogic;
	NpcActorDESC.ActorLogicObject = LogicObject;

	NpcActorDESC.VectorCalcIsForce = true;

	NpcActorDESC.InitialPhysics  = Vector2T<float>(7.2f, 3.2f);
	NpcActorDESC.InitialScale    = Vector2T<float>(2.0f, 2.0f);
	NpcActorDESC.InitialPosition = position;

	NpcActorEntities.Get()->CreateGameActor(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorNPC"),
		NpcActorDESC
	);
}