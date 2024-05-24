// psag_actor_module_debug.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameDebugGuiWindow {
	inline void LndicatorLED(bool state, const ImVec2& size, const ImVec4& hcol, const ImVec4& lcol) {
		if (state) ImGui::ColorButton("", hcol, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, size);
		else       ImGui::ColorButton("", lcol, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, size);
	}

	constexpr ImVec4 L_COLOR = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
	constexpr ImVec4 H_COLOR = ImVec4(0.0f, 1.0f, 0.92f, 1.0f);

	void DebugWindowGuiActor(const string& name, GameActorCore::GameActorActuator* actor) {
		ImGui::Begin(name.c_str());
		{
			ImGui::Text("Actor Name: %u, Code: %u", actor->ActorGetPrivate().ActorTypeCode, actor->ActorGetPrivate().ActorUniqueCode);
			ImGui::Text("W"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_W), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("A"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_A), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("S"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_S), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("D"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_D), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);

			float ControlWidth = ImGui::GetWindowWidth() - IMGUI_ITEM_SPAC * 2.0f;
			ImGui::SetNextItemWidth(ControlWidth);
			ImGui::SliderFloat2("##SCALE", actor->ActorMappingScaleModify()->data(), 0.2f, 5.0f);
			ImGui::SetNextItemWidth(ControlWidth);
			ImGui::SliderFloat2("##MOVE", actor->ActorMappingMoveSpeed()->data(), -10.0f, 10.0f);

			if (ImGui::Button("RE")) *actor->ActorMappingRotateSpeed() = 0.0f;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ControlWidth);
			ImGui::SliderFloat("##ROTATE", actor->ActorMappingRotateSpeed(), -5.0f, 5.0f);
		}
		ImGui::End();
	}

	void DebugWindowGuiActors(unordered_map<size_t, GameActorCore::GameActorActuator*> actors) {
		ImGui::Begin("DebugActors INFO", (bool*)0, ImGuiWindowFlags_NoScrollbar);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 0.92f), "Actors Number: %u", actors.size());

		int CountItemID = NULL;
		for (const auto& ActorItem : actors) {
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f, 0.0f, 0.16f, 0.92f));
			ImGui::PushID(CountItemID);
			ImGui::BeginChild("INFO", ImVec2(ImGui::GetWindowSize().x - IMGUI_ITEM_SPAC * 2.0f, 64.0f), true);

			ImGui::TextColored(
				ImVec4(0.0f, 1.0f, 1.0f, 0.92f), "actor: name: %u code: %u",
				ActorItem.second->ActorGetPrivate().ActorTypeCode,
				ActorItem.second->ActorGetPrivate().ActorUniqueCode
			);
			ImGui::TextColored(
				ImVec4(0.0f, 1.0f, 0.72f, 0.92f), "actor: pos: %.2f,%.2f life: %.2f s",
				ActorItem.second->ActorGetPosition().vector_x,
				ActorItem.second->ActorGetPosition().vector_y,
				ActorItem.second->ActorGetLifeTime()
			);

			++CountItemID;
			ImGui::EndChild(); ImGui::PopID();
			ImGui::PopStyleColor();
		}
		ImGui::End();
	}
}