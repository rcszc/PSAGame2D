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
			ImGui::Text("Actor Speed: %.2f, %.2f", actor->ActorGetMoveSpeed().vector_x, actor->ActorGetMoveSpeed().vector_y);
			ImGui::Text("Actor Scale: %.2f, %.2f", actor->ActorGetScale().vector_x, actor->ActorGetScale().vector_y);
			ImGui::Text("Actor Angle: %.3f", actor->ActorGetAngle());
			ImGui::Text("Actor AngleVec: %.3f", actor->ActorGetRotateSpeed());
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

	void DebugGamePanel::GameInfoViewFPS(float width) {
		// 2500(ms) sample min_fps.
		if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - DebugFpsTimer).count() > 2500) {
			FramerateParams[2] = 0.0f;
			FramerateParams[3] = 32768.0f;
			FrameCounter = NULL;
			DebugFpsTimer = chrono::steady_clock::now();
		}
		if (FramerateParams[0] < FramerateParams[3]) FramerateParams[3] = FramerateParams[0];
		if (FramerateParams[0] > FramerateParams[2]) FramerateParams[2] = FramerateParams[0];
		
		FramerateParams[0] = ImGui::GetIO().Framerate;
		FramerateParams[1] = (FramerateParams[1] * FrameCounter + FramerateParams[0]) / (FrameCounter + 1.0f);

		const ImVec2 WindowSize(width, DebugWindowHeight[0]);
		// draw params_view child_window.
		ImGui::BeginChild("##DEBUG_VIEW_FPS", WindowSize);
		ImGui::SetCursorPosY(IMGUI_ITEM_SPAC);
		ImGui::Indent(IMGUI_ITEM_SPAC);
		{
			const ImVec2 PBSIZE(240.0f, 20.0f);
			ImGui::Text("Run:"); ImGui::SameLine(); ImGui::ProgressBar(FramerateParams[0] / FramerateLimitMax, PBSIZE);
			ImGui::Text("Avg:"); ImGui::SameLine(); ImGui::ProgressBar(FramerateParams[1] / FramerateLimitMax, PBSIZE);

			ImGui::Text("Framerate (RT) Run: %.2f", FramerateParams[0]);

			ImGui::Text("Framerate (3200ms) Avg: %.2f", FramerateParams[1]);
			ImGui::Text("Framerate (3200ms) Max: %.2f", FramerateParams[2]);
			ImGui::Text("Framerate (3200ms) Min: %.2f", FramerateParams[3]);
			ImGui::Text("Max - Min: %.2f", FramerateParams[2] - FramerateParams[3]);
		}
		ImGui::Unindent(IMGUI_ITEM_SPAC);
		ImGui::EndChild();
		// frame sync counter.
		++FrameCounter;
	}

	void DebugGamePanel::GameInfoViewPPActor(float width) {
		const ImVec2 WindowSize(width, DebugWindowHeight[1]);
		// draw params_view child_window.
		ImGui::BeginChild("##DEBUG_VIEW_PPACTOR", WindowSize);
		ImGui::SetCursorPosY(IMGUI_ITEM_SPAC);
		ImGui::Indent(IMGUI_ITEM_SPAC);
		{
			ImGui::Text(
				"Actor Type: %u, Unique: %u", 
				AEREF->ActorGetPrivate().ActorTypeCode, AEREF->ActorGetPrivate().ActorUniqueCode
			);
			ImGui::Text("W"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_W), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("A"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_A), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("S"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_S), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);
			ImGui::SameLine(); ImGui::Text("D"); ImGui::SameLine();
			LndicatorLED(ImGui::IsKeyDown(ImGuiKey_D), ImVec2(28.0f, 28.0f), H_COLOR, L_COLOR);

			ImGui::Text("Actor Position: %.2f, %.2f", AEREF->ActorGetPosition().vector_x, AEREF->ActorGetPosition().vector_y);
			ImGui::Text("Actor Speed: %.2f, %.2f", AEREF->ActorGetMoveSpeed().vector_x, AEREF->ActorGetMoveSpeed().vector_y);
			ImGui::Text("Actor Scale: %.2f, %.2f", AEREF->ActorGetScale().vector_x, AEREF->ActorGetScale().vector_y);
			ImGui::Text("Actor Angle: %.3f", AEREF->ActorGetAngle());
			ImGui::Text("Actor AngleVec: %.3f", AEREF->ActorGetRotateSpeed());
		}
		ImGui::Unindent(IMGUI_ITEM_SPAC);
		ImGui::EndChild();
	}

	void DebugGamePanel::GameInfoViewGlobal(float width) {
		const ImVec2 WindowSize(width, DebugWindowHeight[2]);
		// draw params_view child_window.
		ImGui::BeginChild("##DEBUG_VIEW_GLOBAL", WindowSize);
		ImGui::SetCursorPosY(IMGUI_ITEM_SPAC);
		ImGui::Indent(IMGUI_ITEM_SPAC);
		{
			size_t GPA = ActorSystemAtomic::GLOBAL_PARAMS_ACTORS, GPMA = ActorSystemAtomic::GLOBAL_PARAMS_M_ACTORS;
			size_t GPE = ActorSystemAtomic::GLOBAL_PARAMS_EVNS, GPME = ActorSystemAtomic::GLOBAL_PARAMS_M_EVNS;
			size_t GPS = ActorSystemAtomic::GLOBAL_PARAMS_SHADERS, GPMS = ActorSystemAtomic::GLOBAL_PARAMS_M_SHADERS;

			ImGui::Text("Game Global Params:");
			ImGui::Text("Actors: %u, AManager: %u", GPA, GPMA);
			ImGui::Text("Environments: %u, EManager: %u", GPE, GPME);
			ImGui::Text("Shaders: %u, SManager: %u", GPS, GPMS);
		}
		ImGui::Unindent(IMGUI_ITEM_SPAC);
		ImGui::EndChild();
	}

	void DebugGamePanel::SettingPPActorRef(GameActorCore::GameActorExecutor* actor) {
		AEREF = actor;
	}

	void DebugGamePanel::RenderingWindowGui() {
#if PSAG_DEBUG_MODE 
		if (ImPsag::GetDebugGuiFlag()) {
			// set window draw style.
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.28f, 0.0f, 0.58f, 0.38f));
			ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.18f, 0.0f, 0.38f, 0.72f));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.18f, 0.0f, 0.38f, 0.72f));

			ImGuiWindowFlags Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground;
			ImGui::SetNextWindowSize(ImVec2(335.0f, 550.0f));
			ImGui::Begin(DebugWindowName, (bool*)NULL, Flags);
			ImGui::SetWindowFontScale(1.08f);
			{
				float WIDTH = ImGui::GetWindowWidth() - IMGUI_ITEM_SPAC * 2.0f;
				GameInfoViewFPS(WIDTH);
				if (AEREF != nullptr) GameInfoViewPPActor(WIDTH);
				GameInfoViewGlobal(WIDTH);
			}
			ImGui::End();
			ImGui::PopStyleColor(3);
		}
#endif
	}
}