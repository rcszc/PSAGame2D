// demo_origin_game_npc.
#include "demo_origin_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void NpcActorLogic::ActionLogicRun(PsagActor::Actor* actor_object, float timestep) {
	if (PawnActorPointer == nullptr)
		return;
	/*
	if (FireTimer.CycleTimerGetFlag()) {
		BulletFireFunc(actor_object->ActorGetPosition(), NPC_ANGLE);
		FireTimer.CycleTimerClearReset(300.0f + ThisNPCTimetrOffset);
	}
	*/

	Vector2T<float> Force = PsagManager::Maths::SurroundingOrbit(
		actor_object->ActorGetPosition(), PawnActorPointer->ActorGetPosition(), 100.0f
	);
	// move & rotate npc_actor.
	actor_object->ActorApplyForceRotate((Force.vector_x + Force.vector_y) * 10.0f);
	actor_object->ActorApplyForceMove(Force);
	//HealthRendering(actor_object);
}

void NpcActorLogic::HealthRendering(PsagActor::Actor* actor_object) {
	// ÐÞ¸Ä ImGui ¿Ø¼þÑÕÉ«.
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
	//ImGui::ProgressBar(actor_object->ActorGetHealth(0) / NpcActorHPmax);
	ImGui::End();

	ImGui::PopStyleColor(8);
}