// psagame_dev_test_panel.
#include "psagame_dev_test.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSA_PANEL {

	void MainControlPanel::RunThreadFlagsProcess() {
		if (!ThreadFlagExit)
			RunTimePoint = chrono::system_clock::now();
	}

	void MainControlPanel::RenderPanel() {
		// mul imgui flags.
		ImGuiWindowFlags FlagsTemp = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
		constexpr float WindowWidth = 380.0f;

		// push style colors.
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.0f, 0.22f, 0.92f));

		ImVec4 WinUniformColor = ImVec4(0.32f, 0.0f, 0.72f, 0.92f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg,      WinUniformColor);
		ImGui::PushStyleColor(ImGuiCol_TitleBg,       WinUniformColor);
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, WinUniformColor);

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.0f, 0.28f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.0f, 0.58f, 0.92f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.28f, 0.0f, 0.42f, 0.92f));

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.78f, 1.0f));

		// push style values.
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 7.2f);

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - WindowWidth, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(WindowWidth, ImGui::GetIO().DisplaySize.y));
		ImGui::Begin("MAIN - CONTROL - RCSZ v0.12", (bool*)NULL, FlagsTemp);
		{
			float ChildWinWidth = WindowWidth - IMGUI_ITEM_SPAC * 2.0f;
			ImVec2 StartWinSize = ImVec2(ChildWinWidth, 50.0f);

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::BeginChild("BUTTON0", StartWinSize);
			ImGui::SetWindowFontScale(2.0f);
			{
				if (ImGui::Button("START", StartWinSize)) {
					ThreadFlagStart = true;
					ThreadFlagExit  = false;
					// init timer_clock.
					StartTimePoint = chrono::system_clock::now();
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleColor(1);

			ImGui::BeginChild("TIMER", ImVec2(ChildWinWidth, 140.0f));
			ImGui::SetWindowFontScale(2.0f);
			{
				ImGui::SetCursorPosY(IMGUI_ITEM_SPAC);
				ImGui::Indent(IMGUI_ITEM_SPAC);

				int64_t TimeCount = chrono::duration_cast<chrono::milliseconds>(RunTimePoint - StartTimePoint).count();
				float SecTimeTemp = (float)TimeCount / 1000.0f;
				ImGui::Text("TIME: %.3f sec", SecTimeTemp);

				// calc size_t => double => fp32.
				float DataCount = float((double)RunThreadDataCount / 1048576.0);
				ImGui::Text("DATA: %.3f mib", DataCount);

				ImGui::Text("TICK: %llu", (size_t)RunThreadTickCount);

				ImGui::Unindent(IMGUI_ITEM_SPAC);
			}
			ImGui::EndChild();

			ImGui::BeginChild("SETTING", ImVec2(ChildWinWidth, 80.0f));
			ImGui::SetWindowFontScale(1.2f);
			{
				ImGui::SetCursorPosY(IMGUI_ITEM_SPAC);
				ImGui::Indent(IMGUI_ITEM_SPAC);

				ImGui::SetNextItemWidth(170.0f);
				ImGui::InputInt(" MaxCycles[TICK]", &SettingRunCycles);
				ImGui::SetNextItemWidth(170.0f);
				ImGui::InputFloat(" MaxMemory[MiB]", &SettingRunMemoryMib);

				ImGui::Unindent(IMGUI_ITEM_SPAC);
			}
			ImGui::EndChild();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.0f, 0.20f, 0.92f));
			{
				unique_lock<mutex> Lock(RunThreadStrLog.ResourceMutex);
				ImGui::InputTextMultiline(
					"##source", 
					RunThreadStrLog.Resource.data(), 
					RunThreadStrLog.Resource.size(), 
					ImVec2(ChildWinWidth, ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - IMGUI_ITEM_SPAC),
					ImGuiInputTextFlags_ReadOnly
				);
			}
			ImGui::PopStyleColor(1);
		}
		ImGui::End();
		ImGui::PopStyleColor(8);
		ImGui::PopStyleVar(1);

		RunThreadFlagsProcess();
	}
}