// psag_toolkits_engine_uiext.
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

	namespace Large {
		// value = min || value = max ?
		inline bool FLOAT32_IS_LIMIT(float value, float min, float max) {
			return abs(min - value) < 1e-5F || abs(max - value) < 1e-5F;
		}
		// vec4(rgba) => scale vec3(rgb), ahpla: 1.0f => vec4(rgba).
		inline ImVec4 TRANS_RGB(float value) { return ImVec4(value, value, value, 1.0f); }

		constexpr float VIEW_GRID_HEIGHT = 11.0f;

		void ImMegaPlotsDataView::CoreViewWindowRender(PlotsDatasetPTR data, float smp_scale) {
			DataSampleRate = 1.0f; // reset sample_rate.
			// 采样值极限差值.
			float LimitValueDiff = ViewValueAxisLimit.y - ViewValueAxisLimit.x;
			float CurrentTemp = ImGui::GetCursorPosY();

			ImGui::BeginChild("RULER_DRAW", ImVec2(ViewWindowDrawWidthRuler, ViewWindowSize.y), false);
			{
				float ScaleValueHeight = ImGui::CalcTextSize("0.0").y;
				float ScaleValueCount  = ViewValueAxisLimit.x;

				float STEP = ImGui::GetWindowHeight() / VIEW_GRID_HEIGHT;
				for (float i = ImGui::GetWindowHeight() - STEP * 0.5f; i >= STEP * 0.4f; i -= STEP) {

					ImPsag::ListDrawText(
						ImVec2(IMGUI_ITEM_SPAC, i - ScaleValueHeight * 0.5f),
						ViewWindowColorsystem,
						"%.2f",
						ScaleValueCount
					);
					ImPsag::ListDrawLine(
						ImVec2(ViewWindowDrawWidthRuler - 3.85f, i),
						ImVec2(ViewWindowDrawWidthRuler - 14.5f, i),
						ViewWindowColorsystem * TRANS_RGB(0.5f), 4.5f
					);
					ScaleValueCount += LimitValueDiff / (VIEW_GRID_HEIGHT - 1.0f);
				}
				// set next child_window draw_position.
				ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ScaleValueHeight);
			}
			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + ViewWindowDrawWidthRuler + IMGUI_ITEM_SPAC, CurrentTemp));
			ImGui::BeginChild("PLOT_DRAW", ViewWindowSize, false);
			{
				float XasixOffset = ViewWindowScrollxPosition * (ViewWindowDrawLength - ImGui::GetWindowWidth());
				for (float i = 0.0f; i < ViewWindowDrawLength; i += 50.0f) {
					ImPsag::ListDrawLine(
						ImVec2(i - XasixOffset, 0.0f),
						ImVec2(i - XasixOffset, ImGui::GetWindowSize().y),
						ViewWindowColorsystem * TRANS_RGB(0.24f),
						ViewWindowDrawWidthPlots
					);
				}
				float STEP = ImGui::GetWindowHeight() / VIEW_GRID_HEIGHT;
				uint32_t LinesCount = NULL;
				// draw height_ruler lines.
				for (float i = STEP * 0.5f; i < ImGui::GetWindowHeight(); i += STEP) {
					// min,max lines color_light.
					float LinesColorTrans = 0.24f;
					if (LinesCount == 0 || LinesCount == (uint32_t)VIEW_GRID_HEIGHT - 1)
						LinesColorTrans = 0.58f;

					ImPsag::ListDrawLine(
						ImVec2(0.0f, i),
						ImVec2(ImGui::GetWindowSize().x, i),
						ViewWindowColorsystem * TRANS_RGB(LinesColorTrans),
						ViewWindowDrawWidthPlots
					);
					++LinesCount;
				}
				// sample data value => draw value.
				float ValueRatio = (ImGui::GetWindowHeight() - STEP) / LimitValueDiff;

				float DrawHeightBegin = ImGui::GetWindowHeight() - STEP * 0.5f;
				float DrawHeightEnd   = STEP * 0.5f;

				size_t MaxDataArraySize = NULL;
				for (size_t i = 0; i < (*data).size(); ++i) {
					MaxDataArraySize = __psag_bit_max(MaxDataArraySize, (*data)[i].size());
					// trans sample_rate calc.
					if ((float)MaxDataArraySize >= ViewWindowDrawLength)
						DataSampleRate = (float)MaxDataArraySize / ViewWindowDrawLength;
				}
				DataSampleRate *= smp_scale;
				// low-sample-rate, std-sample-rate.
				float LowSampleRate = PSAG_IMVEC_CLAMP(DataSampleRate, 0.001f, 1.0f);
				DataSampleRate = PSAG_IMVEC_CLAMP(DataSampleRate, 1.0f, 10000.0f);
				
				size_t DrawColorsIndex = NULL;
				for (size_t i = 0; i < (*data).size(); ++i) {
					float DrawPointsSTEP = 0.0f;
					// draw sample_points & plot_view.
					for (float j = 0.0; j < (float)(*data)[i].size() - DataSampleRate; j += DataSampleRate) {

						float DrawPoints[2] = {};
						for (size_t point = 0; point < 2; ++point) {
							float ValueTemp = (*data)[i][(size_t)j + point * (size_t)DataSampleRate];
							// draw points first,last y_axis.
							DrawPoints[point] = PSAG_IMVEC_CLAMP(
								DrawHeightBegin - (abs(ViewValueAxisLimit.x) + ValueTemp) * ValueRatio,
								DrawHeightEnd, DrawHeightBegin
							);
						}
						// set lines style_color.
						DrawColorsIndex = i < DrawPlotColors.size() ? i : DrawPlotColors.size() - 1;
						ImVec4 DrawLinesColor = DrawPlotColors[DrawColorsIndex];
						// value > limit => error color(red).
						if (FLOAT32_IS_LIMIT(DrawPoints[0], DrawHeightEnd, DrawHeightBegin) ||
							FLOAT32_IS_LIMIT(DrawPoints[1], DrawHeightEnd, DrawHeightBegin)
						) {
							DrawLinesColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						}
						ImPsag::ListDrawLine(
							ImVec2(DrawPointsSTEP - XasixOffset, DrawPoints[0]),
							ImVec2(DrawPointsSTEP - XasixOffset + 1.0f / LowSampleRate, DrawPoints[1]),
							DrawLinesColor,
							ViewWindowDrawWidthPlots * 0.5f
						);
						DrawPointsSTEP += 1.0f / LowSampleRate;
					}
				}
			}
			ImGui::EndChild();
		}

		void ImMegaPlotsDataView::SettingWindowRender(bool* window_open) {
			ImGui::SetNextWindowSize(ViewWindowSize * 0.45f);
			if (*window_open) {
				ImGui::Begin(" ParamsSetting", window_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
				{
					ImGui::Text("SCALE: ");
					float Items = (ImGui::GetWindowWidth() - ImGui::CalcTextSize("SCALE: ").x) * 0.5f 
						- IMGUI_ITEM_SPAC * 2.0f;

					ImGui::SameLine(); ImGui::SetNextItemWidth(Items);
					ImGui::SliderFloat("##SCSLIDER", &DataSampleRateScale, 0.001f, 1.0f);

					ImGui::SameLine(); ImGui::SetNextItemWidth(Items);
					ImGui::InputFloat("##SCVALUE", &DataSampleRateScale);

					DataSampleRateScale = PSAG_IMVEC_CLAMP(DataSampleRateScale, 0.001f, 1.0f);

					int32_t LinesCounter = NULL;
					for (auto& ColorItem : DrawPlotColors) {
						auto NumberStr = FMT_NUMBER_FILLZERO(LinesCounter, 2);
						ImGui::TextColored(ColorItem, "%s-LineColor: ", NumberStr.c_str());
						ImGui::SameLine();
						ImGui::PushID(LinesCounter + 1);
						{
							ImGui::ColorEdit4("##EDITOR", &ColorItem.x);
						}
						ImGui::PopID();
						++LinesCounter;
					}
					if (ImGui::Button("Push ColorConfig"))
						DrawPlotColors.push_back(ImVec4(0.7f, 0.7f, 0.7f, 0.92f));
					ImGui::SameLine();
					if (ImGui::Button("Pop ColorConfig") && DrawPlotColors.size() > 1)
						DrawPlotColors.pop_back();
				}
				ImGui::End();
			}
		}

		ImMegaPlotsDataView::ImMegaPlotsDataView() {
			// system default color_system.
			ViewWindowColorsystem = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
			PushLogger(LogInfo, PSAGM_TOOLKITS_GUI_EXT, "plot_comp: object: ptr[%x] create.", this);
		}

		void ImMegaPlotsDataView::DrawImGuiDataPlot(const string& window_name, bool fixed) {
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   7.2f);
			// frame push_style colors.
			ImGui::PushStyleColor(ImGuiCol_TitleBg,        ViewWindowColorsystem * TRANS_RGB(0.22f));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ViewWindowColorsystem * TRANS_RGB(0.22f));
			ImGui::PushStyleColor(ImGuiCol_FrameBg,        ViewWindowColorsystem * TRANS_RGB(0.28f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ViewWindowColorsystem * TRANS_RGB(0.45f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ViewWindowColorsystem * TRANS_RGB(0.35f));
			ImGui::PushStyleColor(ImGuiCol_Button,         ViewWindowColorsystem * TRANS_RGB(0.28f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ViewWindowColorsystem * TRANS_RGB(0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ViewWindowColorsystem * TRANS_RGB(0.35f));

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ViewWindowColorsystem * TRANS_RGB(0.2f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg,  ViewWindowColorsystem * TRANS_RGB(0.28f));

			ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ViewWindowColorsystem * TRANS_RGB(0.58f));
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ViewWindowColorsystem * TRANS_RGB(0.92f));
			ImGui::PushStyleColor(ImGuiCol_Text,             ViewWindowColorsystem * TRANS_RGB(1.18f));

			ImGuiWindowFlags WinFlags = ImGuiWindowFlags_None;
			if (fixed) WinFlags = 
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
			ImGui::Begin(window_name.c_str(), (bool*)NULL, WinFlags);
			{
				size_t DatasetTotalSize = NULL;
				for (size_t i = 0; i < SampleDatasetPoints.size(); ++i)
					DatasetTotalSize += SampleDatasetPoints[i].size();

				float SampleUsage = float(double(DatasetTotalSize * sizeof(float)) / 1048576.0);
				float SampleRate = 1.0f / (float)DataSampleRate * 100.0f;
				ImGui::Text(
					"INFO: [Min]: %.3f [Max]: %.3f [Total]: %u [Mem]: %.3f MiB [SampleRate]: %.4f%% [Channels]: %u",
					ViewValueAxisLimit.x,
					ViewValueAxisLimit.y,
					DatasetTotalSize,
					SampleUsage,
					SampleRate,
					SampleDatasetPoints.size()
				);
				ViewWindowDrawWidthRuler = max(
					ImGui::CalcTextSize(to_string(ViewValueAxisLimit.x).c_str()).x,
					ImGui::CalcTextSize(to_string(ViewValueAxisLimit.y).c_str()).x
				) - 3.2f;
				ViewWindowSize = ImGui::GetWindowSize() 
					- ImVec2(IMGUI_ITEM_SPAC * 3.0f + ViewWindowDrawWidthRuler, 96.0f);
				// clamp window size.
				ViewWindowSize = ImClamp(ViewWindowSize, ImVec2(384.0f, 160.0f), ImVec2(10000.0f, 10000.0f));
				ViewWindowDrawLength = ViewWindowSize.x * 2.0f;

				CoreViewWindowRender(&SampleDatasetPoints, DataSampleRateScale);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.2f);
				if (ImGui::Button("OpenSetting", ImVec2(160.0f, 0.0f)))
					SettingWindowFlag = !SettingWindowFlag;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 160.0f - IMGUI_ITEM_SPAC * 3.0f);
				ImGui::SliderFloat("##SCPOSX", &ViewWindowScrollxPosition, 0.0f, 1.0f);
			}
			ImGui::End();

			SettingWindowRender(&SettingWindowFlag);

			ImGui::PopStyleColor(13);
			ImGui::PopStyleVar(2);
		}

		bool ImMegaPlotsDataView::SettingValueLimit(float min, float max) {
			if (min >= max) return false;
			ViewValueAxisLimit = ImVec2(min, max);
			return true;
		}
		void ImMegaPlotsDataView::SettingColorsystem(const ImVec4& color) {
			ViewWindowColorsystem = color;
		}
		bool ImMegaPlotsDataView::SettingWindowSize(float width, float height) {
			if (width < 512.0f || height < 512.0f) return false;
			ViewWindowSize = ImVec2(width, height);
			return true;
		}
		void ImMegaPlotsDataView::SettingWindowPosition(float x, float y) {
			ViewWindowSize = ImVec2(x, y);
		}
	}
}