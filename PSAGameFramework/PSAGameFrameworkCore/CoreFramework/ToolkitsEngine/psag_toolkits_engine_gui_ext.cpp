// psag_toolkits_engine_gui_ext.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ToolkitsEngineGuiExt {

	void BoxText(const char* text, float box_length, const ImVec4& color) {
		ImVec2 TextSize = ImGui::CalcTextSize(text);
		ImVec2 BoxDummySize = ImVec2(box_length, TextSize.y + IMGUI_ITEM_SPAC);
		// text box(fill).
		ImPsag::ListDrawRectangleFill(
			ImGui::GetCursorPos(), BoxDummySize, 
			color
		);
		ImVec2 CursorTemp = ImGui::GetCursorPos();
		ImGui::SetCursorPos(CursorTemp + ImVec2(IMGUI_ITEM_SPAC, IMGUI_ITEM_SPAC * 0.5f));
		ImGui::Text(text);
		ImGui::SetCursorPos(CursorTemp);
		ImGui::Dummy(BoxDummySize);
	}
}