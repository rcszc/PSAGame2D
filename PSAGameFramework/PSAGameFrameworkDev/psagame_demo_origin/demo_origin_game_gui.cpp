// demo_origin_game_gui.
#include "demo_origin_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void DemoGameOrigin::GameRenderGui() {
	auto PawnActor = DemoActors->FindGameActor(PawnActorCode);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    7.2f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,    7.8f);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.85f, 1.0f));

	ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.0f, 0.28f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.0f, 0.88f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.22f, 0.0f, 0.44f, 0.92f));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f,  0.0f,  0.0f,  0.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg,  ImVec4(0.1f,  0.0f,  0.24f, 0.72f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg,  ImVec4(0.16f, 0.0f,  0.32f, 0.84f));

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.22f, 0.0f, 0.44f, 0.78f));
	{
		float BarHeight = 166.0f;
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, BarHeight));
		ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetIO().DisplaySize.y - BarHeight));

		ImGuiWindowFlags WindowFlags =
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("##GameStateBar", (bool*)NULL, WindowFlags);
		{
			// ================================ 游戏状态栏: 参数显示 ================================
			ImGui::BeginChild("##ParamsView", ImVec2(384.0f, BarHeight - IMGUI_ITEM_SPAC * 2.0f));
			ImGui::SetWindowFontScale(1.18f);

			ImGui::Indent(IMGUI_ITEM_SPAC);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IMGUI_ITEM_SPAC);

			auto Position = PawnActor->ActorGetPosition();

			ImVec4 BoxColor = ImVec4(0.22f, 0.0f, 0.7f, 0.92f);

			PsagManager::GuiTools::BoxText("GAME [FPS]:", 160.0f, BoxColor);
			ImGui::SameLine(); ImGui::Text("%.2f fps", ImGui::GetIO().Framerate);

			PsagManager::GuiTools::BoxText("SHIP [SPEED]:", 160.0f, BoxColor);
			ImGui::SameLine(); ImGui::Text("%.2f km/s", PawnActor->ActorGetSpeed());

			PsagManager::GuiTools::BoxText("SHIP [POS]:", 160.0f, BoxColor);
			ImGui::SameLine(); ImGui::Text("x %.2f y %.2f", Position.vector_x, Position.vector_y);
			PsagManager::GuiTools::BoxText("SHIP [TURN]:", 160.0f, BoxColor);
			ImGui::SameLine(); ImGui::Text("%.2f deg", PawnActor->ActorGetRotate());

			ImGui::Unindent(IMGUI_ITEM_SPAC);
			ImGui::EndChild();

			ImGui::SameLine();
			ImVec2 HPwindowSize(512.0f, BarHeight - IMGUI_ITEM_SPAC * 2.0f);
			ImGui::BeginChild("##HealthView", HPwindowSize);
			ImGui::SetWindowFontScale(1.18f);

			ImGui::Indent(IMGUI_ITEM_SPAC);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IMGUI_ITEM_SPAC);

			float BarWidth = HPwindowSize.x - 184.0f - IMGUI_ITEM_SPAC * 4.0f;
			for (size_t i = 0; i < 3; ++i) {
				PsagManager::GuiTools::BoxText("SHIP-HP", 92.0f, BoxColor);
				ImGui::SameLine(); 
				PsagManager::GuiTools::BoxText(
					to_string((int32_t)PawnActor->ActorGetHealth(i)).c_str(), 92.0f, BoxColor
				);
				ImGui::SameLine();
				ImGui::ProgressBar(PawnActor->ActorGetHealth(i) / PawnMaxHealth[i], ImVec2(BarWidth, 0.0f));
			}
			ImGui::Unindent(IMGUI_ITEM_SPAC);
			ImGui::EndChild();
		}
		ImGui::End();
	}
	ImGui::PopStyleColor(8);
	ImGui::PopStyleVar(3);

	PsagActor::DebugTools::DebugWindowGuiActorPawn("PAWN_ACTOR", PawnActor);
	PsagActor::DebugTools::DebugWindowGuiActors("ACTORS", DemoActors->GetSourceData());
}