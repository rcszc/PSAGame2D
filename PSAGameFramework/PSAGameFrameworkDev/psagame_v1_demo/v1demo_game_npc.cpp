// v1demo_game_npc.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

class ActorLogicNPC :public PsagActor::ActorLogicBase {
protected:
	static float TimerOffset;
	float ThisNPCTimetrOffset = 0.0f;

	PsagActor::ActorsManager* ActorsManagerPointer = nullptr;
	PsagActor::Actor*         PawnActorPointer     = nullptr;

	PsagManager::Tools::Timer::GameCycleTimer FireTimer = {};
	function<void(const Vector2T<float>&, float)> BulletFireFunc = {};
public:
	ActorLogicNPC(
		PsagActor::Actor* ptr, PsagActor::ActorsManager* mag, function<void(const Vector2T<float>&, float)> bullet_func) :
		PawnActorPointer(ptr), ActorsManagerPointer(mag), BulletFireFunc(bullet_func)
	{
		ThisNPCTimetrOffset = TimerOffset += 50.0f;
	};

	void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {
		if (PawnActorPointer == nullptr)
			return;
		// NPC与目标点的距离.
		Vector2T<float> ActorPosition = actor_object->ActorGetPosition();
		Vector2T<float> ActorTarget = 
			Vector2T<float>(
				PawnActorPointer->ActorGetPosition().vector_x,
				PawnActorPointer->ActorGetPosition().vector_y
			);
		float TargetDist = PsagManager::Maths::CalcFuncPointsDistance(ActorPosition, ActorTarget);

		float ForceDirection = 0.7f;
		if (TargetDist < 190.0f) ForceDirection = -2.4f;

		// npc actor 朝向 pawn actor.
		float NPC_ANGLE = PsagManager::Maths::CalcFuncPointsAngle(actor_object->ActorGetPosition(), PawnActorPointer->ActorGetPosition());
		actor_object->ActorModifyState(actor_object->ActorGetPosition(), NPC_ANGLE);

		if (FireTimer.CycleTimerFlagGet()) {
			BulletFireFunc(actor_object->ActorGetPosition(), NPC_ANGLE);
			FireTimer.CycleTimerClearReset(300.0f + ThisNPCTimetrOffset);
		}

		// move npc_actor.
		actor_object->ActorApplyForceMove(
			Vector2T<float>(
			(ActorPosition.vector_x - ActorTarget.vector_x) * 0.001f * ForceDirection,
			(ActorPosition.vector_y - ActorTarget.vector_y) * 0.001f * ForceDirection
		));
		HealthRendering(actor_object);

		// NPC血量 => 销毁.
		if (actor_object->ActorGetHealth(0) <= 1.0f) {
			ActorsManagerPointer->DeleteGameActor(actor_object->ActorGetPrivate().ActorUniqueCode);
		}
	}

	void HealthRendering(PsagActor::Actor* actor_object) {
		// 修改 ImGui 控件颜色.
		ImGui::PushStyleColor(ImGuiCol_WindowBg,       ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.72f, 0.72f, 0.72f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_TitleBg,        ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram,  ImVec4(1.0f, 0.25f, 0.0f, 0.38f));

		ImGuiWindowFlags FlagsTemp = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

		auto ACWIN = actor_object->ActorMappingWindowCoord();
		ImGui::SetNextWindowPos(ImVec2(ACWIN.vector_x + 20.0f, ACWIN.vector_y));
		ImGui::SetNextWindowSize(ImVec2(160.0f, 80.0f));

		ImGui::Begin(to_string(actor_object->ActorGetPrivate().ActorUniqueCode).c_str(), (bool*)0, FlagsTemp);
		ImGui::Text("HP: %.1f", actor_object->ActorGetHealth(0));
		ImGui::ProgressBar(actor_object->ActorGetHealth(0) / NpcActorHPmax);
		ImGui::End();

		ImGui::PopStyleColor(8);
	}
};
float ActorLogicNPC::TimerOffset = 0.0f;

void PsaGameV1Demo::GameCreateNPC(const Vector2T<float>& position) {
	PsagActor::ActorDESC ConfigNpcActor;
	PsagActor::ActorHpDESC PawnActorHealthDESC;

	// 创建NPC逻辑, 指针由逻辑执行组件自动销毁.
	ActorLogicNPC* NPCLogicObject = new ActorLogicNPC(
		DemoActors->FindGameActor(PawnActorUnqiue),
		DemoActors,
		[&](const Vector2T<float>& position, float angle) { GameCreateBulletNPC(position, angle); }
	);

	PawnActorHealthDESC.InitialActorHealth.push_back(PsagActor::ActorHP(NpcActorHPmax, 0.5f, NpcActorHPmax));

	ConfigNpcActor.ActorPhysicsWorld   = "DemoPhysics";
	ConfigNpcActor.ActorShaderResource = DemoShaders->FindActorShader("NpcActor");
	ConfigNpcActor.ActorHealthSystem   = PawnActorHealthDESC;
	//ConfigNpcActor.ActorLogicObject    = NPCLogicObject;

	ConfigNpcActor.ForceClacEnable = true;
	//ConfigNpcActor.EnableLogic     = true;

	ConfigNpcActor.InitialPhysics  = Vector2T<float>(32.0f, 10.0f);
	ConfigNpcActor.InitialPosition = position;

	ConfigNpcActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{
		// 与子弹发生碰撞.
		if (DemoActors->FindGameActor(
			ThisActor->ActorGetPrivate().ActorUniqueCode
		)->ActorGetPrivate().ActorTypeCode == PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorBullet")) {
			// NPC扣血 + Clamp.
			float SetHP = ThisActor->ActorGetHealth(0) <= 0.0f ?
				0.0f :
				ThisActor->ActorGetHealth(0) - PsagManager::Tools::RAND::GenerateRandomFunc(220.0f, 240.0f);
			ThisActor->ActorModifyHealth(0, SetHP);
		}
	};

	DemoActors->CreateGameActor(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorNpc"), ConfigNpcActor);
}