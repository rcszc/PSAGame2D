// framework_window_imgui_tools.
#include "framework_window.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ImPsag {
	atomic<bool> DEBUG_GUI_GLOBAL_FLAG = PSAG_FALSE;
	void DebugGuiFlagStatus() {
		if (ImGui::IsKeyPressed(ImGuiKey_F10, false))
			DEBUG_GUI_GLOBAL_FLAG = !DEBUG_GUI_GLOBAL_FLAG;
	}
	bool GetDebugGuiFlag() {
		return DEBUG_GUI_GLOBAL_FLAG;
	}

#define IM_SCALE_CALC(high, value) high + value < 0.0f ? 0.0f : high + value && high + value > 1.0f ? 1.0f : high + value
	ImVec4 ColorGrayscaleScale(const ImVec4& color, float value, float ahpla) {
		return ImVec4(
			IM_SCALE_CALC(color.x, value), IM_SCALE_CALC(color.y, value), 
			IM_SCALE_CALC(color.z, value), IM_SCALE_CALC(color.w, ahpla)
		);
	}

	float ItemCenteredCalcX(float width) {
		// window,child_window item centered.
		return ImGui::GetWindowSize().x / 2.0f - width / 2.0f;
	}

	float ItemCenteredCalcY(float height) {
		// window,child_window item centered.
		return ImGui::GetWindowSize().y / 2.0f - height / 2.0f;
	}

	void ListDrawLine(const ImVec2& point0, const ImVec2& point1, const ImVec4& color, float wline) {
		// draw line_segment.
		ImGui::GetWindowDrawList()->AddLine(
			ImGui::GetWindowPos() + point0,
			ImGui::GetWindowPos() + point1,
			IMVEC4_CVT_COLU32(color),
			wline
		);
	}

#define IMCB_CHARTMEP_BYTES 1024
	void ListDrawText(const ImVec2& position, const ImVec4& color, const char* text, ...) {
		char LoggerStrTemp[IMCB_CHARTMEP_BYTES] = {};

		va_list ParamArgs;
		va_start(ParamArgs, text);
		vsnprintf(LoggerStrTemp, IMCB_CHARTMEP_BYTES, text, ParamArgs);
		va_end(ParamArgs);

		ImGui::GetWindowDrawList()->AddText(
			ImGui::GetWindowPos() + position,
			IMVEC4_CVT_COLU32(color),
			LoggerStrTemp
		);
	}

	void ListDrawRectangle(const ImVec2& position, const ImVec2& size, const ImVec4& color, float width) {
		// draw line_rectangle.
		ImGui::GetWindowDrawList()->AddRect(
			ImGui::GetWindowPos() + position,
			ImGui::GetWindowPos() + position + size,
			IMVEC4_CVT_COLU32(color),
			ImGui::GetStyle().FrameRounding,
			NULL, width
		);
	}

	void ListDrawRectangleFill(const ImVec2& position, const ImVec2& size, const ImVec4& color) {
		// draw fill_rectangle.
		ImGui::GetWindowDrawList()->AddRectFilled(
			ImGui::GetWindowPos() + position,
			ImGui::GetWindowPos() + position + size,
			IMVEC4_CVT_COLU32(color),
			ImGui::GetStyle().FrameRounding,
			NULL
		);
	}

	void ListDrawCircle(const ImVec2& position, float size, const ImVec4& color, float wline) {
		// draw line_circle.
		ImGui::GetWindowDrawList()->AddCircle(
			ImGui::GetWindowPos() + position,
			size,
			IMVEC4_CVT_COLU32(color),
			NULL, wline
		);
	}

	void ListDrawCircleFill(const ImVec2& position, float size, const ImVec4& color) {
		// draw fill_circle.
		ImGui::GetWindowDrawList()->AddCircleFilled(
			ImGui::GetWindowPos() + position,
			size,
			IMVEC4_CVT_COLU32(color)
		);
	}

	void ListDrawTriangle(const ImVec2& position, const ImVec2& offset1, const ImVec2& offset2, const ImVec4& color, float wline) {
		// draw line_triangle.
		ImGui::GetWindowDrawList()->AddTriangle(
			ImGui::GetWindowPos() + position,
			ImGui::GetWindowPos() + position + offset1,
			ImGui::GetWindowPos() + position + offset2,
			IMVEC4_CVT_COLU32(color), wline
		);
	}

	void ListDrawTriangleFill(const ImVec2& position, const ImVec2& offset1, const ImVec2& offset2, const ImVec4& color) {
		// draw fill_triangle.
		ImGui::GetWindowDrawList()->AddTriangleFilled(
			ImGui::GetWindowPos() + position,
			ImGui::GetWindowPos() + position + offset1,
			ImGui::GetWindowPos() + position + offset2,
			IMVEC4_CVT_COLU32(color)
		);
	}

	void ListDrawRoundImage(const ImVec2& position, const ImVec2& size, ImTextureID texture) {
		// draw fill_uv rounded_image.
		ImGui::GetWindowDrawList()->AddImageRounded(
			texture,
			ImGui::GetWindowPos() + position,
			ImGui::GetWindowPos() + position + size,
			ImVec2(0.0f, 0.0f),
			ImVec2(1.0f, 1.0f),
			IMVEC4_CVT_COLU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)),
			ImGui::GetStyle().FrameRounding
		);
	}

	void ListDrawPolyLine(std::vector<ImVec2>* vertices, const ImVec4& color, float wline) {
		// draw points lines.
		ImGui::GetWindowDrawList()->AddPolyline(
			vertices->data(),
			(int)vertices->size(),
			IMVEC4_CVT_COLU32(color),
			ImDrawFlags_None,
			wline
		);
	}

	void ListDrawPolyFilled(std::vector<ImVec2>* vertices, const ImVec4& color) {
		// draw points element.
		ImGui::GetWindowDrawList()->AddConvexPolyFilled(
			vertices->data(), (int)vertices->size(),
			IMVEC4_CVT_COLU32(color)
		);
	}

	namespace ImAnim {
#define FLOAT_SIZE_CLAMP(size) ImVec2(PSAG_IMVEC_CLAMP((size).x, 0.0f, FLT_MAX), PSAG_IMVEC_CLAMP((size).y, 0.0f, FLT_MAX))

		ImVec4 FLOAT_COLOR_CLAMP(const ImVec4& color) {
			return ImVec4(
				PSAG_IMVEC_CLAMP(color.x, 0.0f, 1.0f), PSAG_IMVEC_CLAMP(color.y, 0.0f, 1.0f),
				PSAG_IMVEC_CLAMP(color.z, 0.0f, 1.0f), PSAG_IMVEC_CLAMP(color.w, 0.0f, 1.0f)
			);
		}

		bool ButtonAnim::DrawButton(const char* name, float speed) {
			bool StateFlagTemp = false;

			ImGui::PushStyleColor(ImGuiCol_Button,        ButtonAnimColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ButtonAnimColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ButtonAnimColor);

			// 重叠绘制: InvisibleButton & Button.
			ImVec2 ReposTemp = ImGui::GetCursorPos();
			{
				StateFlagTemp = ImGui::InvisibleButton(name, AnimStatSize[0]);
				size_t TypeIndex = NULL;

				if (!ImGui::IsItemHovered() && !ImGui::IsMouseDown(0)) TypeIndex = 0; // status: normal.
				if (ImGui::IsItemHovered() && !ImGui::IsMouseDown(0))  TypeIndex = 1; // status: hover.
				if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0))   TypeIndex = 2; // status: active.

				ButtonAnimColor += (AnimStatColor[TypeIndex] - ButtonAnimColor) * ANIM_STD_STEP_BUTTON * speed;
				ButtonAnimSize  += (AnimStatSize[TypeIndex]  - ButtonAnimSize)  * ANIM_STD_STEP_BUTTON * speed;
			}
			ImGui::SetCursorPos(ReposTemp);
			// 固定中心位置.
			ImVec2 PosTemp = ImGui::GetCursorPos() + (AnimStatSize[0] - ButtonAnimSize) * 0.5f;
			ImGui::SetCursorPos(PosTemp);
			ImGui::Button(name, ButtonAnimSize);

			ImGui::PopStyleColor(3);
			return StateFlagTemp;
		}

		void ButtonAnim::SetButtonStatColor(const ImVec4& normal, const ImVec4& hover, const ImVec4& active) {
			AnimStatColor[0] = FLOAT_COLOR_CLAMP(normal);
			AnimStatColor[1] = FLOAT_COLOR_CLAMP(hover);
			AnimStatColor[2] = FLOAT_COLOR_CLAMP(active);
			ButtonAnimColor = AnimStatColor[0];
		}

		void ButtonAnim::SetButtonStatSize(const ImVec2& normal, const ImVec2& hover, const ImVec2& active) {
			AnimStatSize[0] = FLOAT_SIZE_CLAMP(normal);
			AnimStatSize[1] = FLOAT_SIZE_CLAMP(hover);
			AnimStatSize[2] = FLOAT_SIZE_CLAMP(active);
			ButtonAnimSize = AnimStatSize[0];
		}
	}
}