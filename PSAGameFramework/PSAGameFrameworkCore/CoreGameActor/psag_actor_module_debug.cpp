// psag_actor_module_debug.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameDebugGuiWindow {
	ImGuiColorEditFlags COLBUT = ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip;
	inline void LndicatorLED(bool state, const ImVec2& size, const ImVec4& hcol, const ImVec4& lcol) {
		if (state) ImGui::ColorButton("##COL", hcol, COLBUT, size);
		else       ImGui::ColorButton("##COL", lcol, COLBUT, size);
	}

	constexpr ImVec4 L_COLOR = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	constexpr ImVec4 H_COLOR = ImVec4(0.0f, 1.0f, 0.9f, 1.0f);

	void DebugWindowGuiActor(const char* name, GameActorCore::GameActorActuator* actor) {
		ImGui::Begin(name);
		{
			ImGui::Text("Actor Type: %u, Code: %u", actor->ActorGetPrivate().ActorTypeCode, actor->ActorGetPrivate().ActorUniqueCode);
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
			ImGui::SliderFloat2("##SCALE", actor->ActorGetScale().data(), 0.2f, 5.0f, "%.2f", ImGuiSliderFlags_NoInput);
			ImGui::SetNextItemWidth(ControlWidth);
			ImGui::SliderFloat2("##MOVE", actor->ActorGetMoveSpeed().data(), -10.0f, 10.0f, "%.2f", ImGuiSliderFlags_NoInput);
			ImGui::SetNextItemWidth(ControlWidth);
			ImGui::ProgressBar(actor->ActorGetRotateSpeed() * 0.1f + 0.5f);
		}
		ImGui::End();
	}

	void DebugWindowGuiActors(unordered_map<size_t, GameActorCore::GameActorActuator*>* actors) {
		ImGui::Begin("DebugActors", (bool*)NULL, ImGuiWindowFlags_NoScrollbar);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 0.92f), "Actors Number: %u", actors->size());

		int CountItemID = NULL;
		for (const auto& ActorItem : *actors) {
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

	void DebugWindowGuiFPS(const char* name, float framerate_params[3]) {
		ImGui::Begin(name, (bool*)NULL, ImGuiWindowFlags_NoScrollbar);
		{
			framerate_params[0] = ImGui::GetIO().Framerate;
			float FrameCount = (float)ImGui::GetFrameCount();
			framerate_params[1] = (framerate_params[1] * FrameCount + framerate_params[0]) / (FrameCount + 1.0f);

			if (framerate_params[0] > framerate_params[2])
				framerate_params[2] = framerate_params[0];

			ImGui::Text("GameFramerate Run: %.2f", framerate_params[0]);
			ImGui::Text("GameFramerate Avg: %.2f", framerate_params[1]);
			ImGui::Text("GameFramerate Max: %.2f", framerate_params[2]);
		}
		ImGui::End();
	}
}