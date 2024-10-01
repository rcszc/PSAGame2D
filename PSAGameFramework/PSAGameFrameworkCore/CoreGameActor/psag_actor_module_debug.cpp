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

	void DebugWindowGuiActorPawn(const char* name, GameActorCore::GameActorExecutor* actor) {
#if PSAG_DEBUG_MODE
		ImGui::Begin(name);
		{
			ImGui::Text("Actor Type: %u, Unique: %u", actor->ActorGetPrivate().ActorTypeCode, actor->ActorGetPrivate().ActorUniqueCode);
			ImGui::Text("W"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_W), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("A"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_A), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("S"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_S), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("D"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_D), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);

			ImGui::Text("Actor Position: %.2f, %.2f", actor->ActorGetPosition().vector_x, actor->ActorGetPosition().vector_y);
			ImGui::Text("Actor Speed: %.2f, %.2f",    actor->ActorGetMoveSpeed().vector_x, actor->ActorGetMoveSpeed().vector_y);
			ImGui::Text("Actor Scale: %.2f, %.2f",    actor->ActorGetScale().vector_x, actor->ActorGetScale().vector_y);
			ImGui::Text("Actor RotateSpeed: %.3f",    actor->ActorGetRotateSpeed());
		}
		ImGui::End();
#endif
	}

	void DebugWindowGuiActors(const char* name, unordered_map<size_t, GameActorCore::GameActorExecutor*>* actors) {
#if PSAG_DEBUG_MODE
		ImGui::Begin(name, (bool*)NULL, ImGuiWindowFlags_NoScrollbar);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 0.92f), "Actors Number: %u", actors->size());

		int CountItemID = NULL;
		for (const auto& ActorItem : *actors) {
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f, 0.0f, 0.16f, 0.92f));
			ImGui::PushID(CountItemID);
			ImGui::BeginChild("INFO", ImVec2(ImGui::GetWindowSize().x - IMGUI_ITEM_SPAC * 2.0f, 64.0f), true);

			ImGui::TextColored(
				ImVec4(0.0f, 1.0f, 1.0f, 0.92f), "actor: type: %u unique: %u",
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
#endif
	}

	void DebugWindowGuiFPS::RenderingWindowGui() {
#if PSAG_DEBUG_MODE
		// 500ms sample min_fps.
		if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - DebugFpsTimer).count() > 3200) {
			FramerateParams[2] = 0.0f;
			FramerateParams[3] = 32768.0f;
			FramerateCount = NULL;
			DebugFpsTimer = chrono::steady_clock::now();
		}
		if (FramerateParams[0] < FramerateParams[3]) FramerateParams[3] = FramerateParams[0];
		if (FramerateParams[0] > FramerateParams[2]) FramerateParams[2] = FramerateParams[0];

		ImGui::Begin(DebugWindowName, (bool*)NULL, ImGuiWindowFlags_NoScrollbar);
		{
			FramerateParams[0] = ImGui::GetIO().Framerate;
			FramerateParams[1] = (FramerateParams[1] * FramerateCount + FramerateParams[0]) / (FramerateCount + 1.0f);

			ImGui::Text("Run:"); ImGui::SameLine(); ImGui::ProgressBar(FramerateParams[0] / FramerateLimitMax);
			ImGui::Text("Avg:"); ImGui::SameLine(); ImGui::ProgressBar(FramerateParams[1] / FramerateLimitMax);

			ImGui::Text("Framerate (RT) Run: %.2f", FramerateParams[0]);

			ImGui::Text("Framerate (3200ms) Avg: %.2f", FramerateParams[1]);
			ImGui::Text("Framerate (3200ms) Max: %.2f", FramerateParams[2]);
			ImGui::Text("Framerate (3200ms) Min: %.2f", FramerateParams[3]);
			ImGui::Text("Max - Min: %.2f", FramerateParams[2] - FramerateParams[3]);
		}
		ImGui::End();
		++FramerateCount;
#endif
	}
}