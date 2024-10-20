// psag_toolkits_engine_gui_temp.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ToolkitsEngineGuiTemp {
	// value = min || value = max ?
	inline bool FLOAT32_IS_LIMIT(float value, float min, float max) {
		return abs(min - value) < 1e-5F || abs(max - value) < 1e-5F;
	}
	// vec4(rgba) => scale vec3(rgb), ahpla: 1.0f => vec4(rgba).
	inline ImVec4 TRANS_RGB(float value) { return ImVec4(value, value, value, 1.0f); }

	constexpr float SampleHight = 11.0f;

	void ImMegaPlotDataViewThread::ThreadCalcEventLoop() {
		while (!ThreadObjectExit) {
			if (!(*SrcDataResource.GetConsumeBuffer()).empty()) {

				cout << "OK" << endl;

				(*SrcDataResource.GetConsumeBuffer()).clear();
			}
		}
	}

	ImMegaPlotDataViewThread::ImMegaPlotDataViewThread() {
		// create start processing thread.
		ThreadObject = new std::thread(&ImMegaPlotDataViewThread::ThreadCalcEventLoop, this);
		PushLogger(LogInfo, PSAGM_TOOLKITS_GUI_TEMP, "calc_thread: object create.");
	}

	ImMegaPlotDataViewThread::~ImMegaPlotDataViewThread() {
		ThreadObjectExit = true;
		ThreadObject->join();
		delete ThreadObject;
		PushLogger(LogInfo, PSAGM_TOOLKITS_GUI_TEMP, "calc_thread: object delete.");
	}

	void ImMegaPlotDataView::CoreViewWindowRender(PlotSmpDataPTR data) {
		DataSampleRate = 1.0f; // reset sample_rate.
		// 采样值极限差值.
		float LimitValueDiff = ViewWindowYCoordLimit.y - ViewWindowYCoordLimit.x;
		float CurrentTemp = ImGui::GetCursorPosY();

		ImGui::BeginChild("RULER_DRAW", ImVec2(ViewWindowDwWidthRuler, ViewWindowSize.y), false);
		{
			float TextValueHeight = ImGui::CalcTextSize("0.0").y;
			float TextValueCount  = 0.0f;

			float STEP = ImGui::GetWindowHeight() / SampleHight;
			for (float i = ImGui::GetWindowHeight() - STEP * 0.5f; i >= STEP * 0.4f; i -= STEP) {

				ImPsag::ListDrawText(
					ImVec2(IMGUI_ITEM_SPAC, i - TextValueHeight * 0.5f), 
					ViewWindowColorsystem, 
					"%.2f", 
					TextValueCount
				);
				ImPsag::ListDrawLine(
					ImVec2(ViewWindowDwWidthRuler - 3.85f, i), 
					ImVec2(ViewWindowDwWidthRuler - 14.5f, i),
					ViewWindowColorsystem * TRANS_RGB(0.5f),
					4.5f
				);
				TextValueCount += LimitValueDiff / (SampleHight - 1.0f);
			}
			// set next child_window draw_position.
			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - TextValueHeight);
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + ViewWindowDwWidthRuler + IMGUI_ITEM_SPAC, CurrentTemp));
		ImGui::BeginChild("PLOT_DRAW", ViewWindowSize, false);
		{
			float XCoordOffset = ViewWindowScrollxPos * (ViewWindowDrawLength - ImGui::GetWindowWidth());
			for (float i = 0.0f; i < ViewWindowDrawLength; i += 50.0f) {
				ImPsag::ListDrawLine(
					ImVec2(i - XCoordOffset, 0.0f),
					ImVec2(i - XCoordOffset, ImGui::GetWindowSize().y),
					ViewWindowColorsystem * TRANS_RGB(0.24f),
					ViewWindowDwWidthPlot 
				);
			}
			float STEP = ImGui::GetWindowHeight() / SampleHight;
			uint32_t LinesCount = NULL;
			// draw height_ruler lines.
			for (float i = STEP * 0.5f; i < ImGui::GetWindowHeight(); i += STEP) {
				// min,max lines color_light.
				float LinesColorTrans = 0.24f;
				if (LinesCount == 0 || LinesCount == (uint32_t)SampleHight - 1)
					LinesColorTrans = 0.58f;

				ImPsag::ListDrawLine(
					ImVec2(0.0f, i),
					ImVec2(ImGui::GetWindowSize().x, i),
					ViewWindowColorsystem * TRANS_RGB(LinesColorTrans),
					ViewWindowDwWidthPlot
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

			for (size_t i = 0; i < (*data).size(); ++i) {
				float DrawPointsSTEP = 0.0f;
				// draw sample_points & plot_view.
				for (float j = 0.0; j < (float)(*data)[i].size() - DataSampleRate; j += DataSampleRate) {

					float DrawPoints[2] = {};
					for (size_t point = 0; point < 2; ++point) {
						DrawPoints[point] = PSAG_IMVEC_CLAMP(
							DrawHeightBegin - (*data)[i][(size_t)j + point * (size_t)DataSampleRate] * ValueRatio,
							DrawHeightEnd, DrawHeightBegin
						);
					}
					ImVec4 DrawLinesColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
					// value > limit => error color(red).
					if (FLOAT32_IS_LIMIT(DrawPoints[0], DrawHeightEnd, DrawHeightBegin) || 
						FLOAT32_IS_LIMIT(DrawPoints[1], DrawHeightEnd, DrawHeightBegin)
					) {
						DrawLinesColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
					}
					ImPsag::ListDrawLine(
						ImVec2(DrawPointsSTEP - XCoordOffset,        DrawPoints[0]),
						ImVec2(DrawPointsSTEP - XCoordOffset + 1.0f, DrawPoints[1]),
						DrawLinesColor,
						ViewWindowDwWidthPlot * 0.5f
					);
					DrawPointsSTEP += 1.0f;
				}
			}
		}
		ImGui::EndChild();
	}

	void ImMegaPlotDataView::SourceDataSubmit(size_t data_block) {
		size_t AddressOffset = __psag_bit_min(data_block, SampleDatasetPoints[SubmitDatasetCount].size() - SubmitDataCount);
		// init(resize) channels number.
		if ((*SrcDataResource.GetProduceBuffer()).size() != SampleDatasetPoints.size())
			(*SrcDataResource.GetProduceBuffer()).resize(SampleDatasetPoints.size());
		
		(*SrcDataResource.GetProduceBuffer())[SubmitDatasetCount].insert(
			(*SrcDataResource.GetProduceBuffer())[SubmitDatasetCount].end(),
			// insert(push) points data(block).
			SampleDatasetPoints[SubmitDatasetCount].begin() + SubmitDataCount,
			SampleDatasetPoints[SubmitDatasetCount].begin() + SubmitDataCount + AddressOffset
		);
		SubmitDataCount += AddressOffset;
		SubmitTotal     += AddressOffset;
		// channel(data) end_pos.
		if (AddressOffset < data_block) {
			SubmitDataCount = 0;
			++SubmitDatasetCount;
		}
		if (SubmitDatasetCount == SampleDatasetPoints.size()) {
			SubmitDataCount    = 0;
			SubmitDatasetCount = 0;
			// dataset submit completed.
			SrcDataResource.SwapResourceBuffers();
			SubmitTotal = 0;
		}
	}

	ImMegaPlotDataView::ImMegaPlotDataView() {
		// system default color_system.
		ViewWindowColorsystem = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		PushLogger(LogInfo, PSAGM_TOOLKITS_GUI_TEMP, "plot_comp: object: ptr[%x] create.", this);
	}

	void ImMegaPlotDataView::DrawImGuiDataPlot(const char* window_name) {
		// frame push_style colors.
		ImGui::PushStyleColor(ImGuiCol_TitleBg,        ViewWindowColorsystem * TRANS_RGB(0.22f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ViewWindowColorsystem * TRANS_RGB(0.22f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg,        ViewWindowColorsystem * TRANS_RGB(0.28f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ViewWindowColorsystem * TRANS_RGB(0.45f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ViewWindowColorsystem * TRANS_RGB(0.35f));
		ImGui::PushStyleColor(ImGuiCol_Button,         ViewWindowColorsystem * TRANS_RGB(0.28f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ViewWindowColorsystem * TRANS_RGB(0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ViewWindowColorsystem * TRANS_RGB(0.35f));

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ViewWindowColorsystem * TRANS_RGB(0.18f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg,  ViewWindowColorsystem * TRANS_RGB(0.28f));

		ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ViewWindowColorsystem * TRANS_RGB(0.58f));
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ViewWindowColorsystem * TRANS_RGB(0.92f));
		ImGui::PushStyleColor(ImGuiCol_Text,             ViewWindowColorsystem * TRANS_RGB(1.18f));

		ImGui::Begin(window_name);
		{
			size_t DatasetTotalSize = NULL;
			for (size_t i = 0; i < SampleDatasetPoints.size(); ++i)
				DatasetTotalSize += SampleDatasetPoints[i].size();

			float SampleUsage = float(double(DatasetTotalSize * sizeof(float)) / 1048576.0);
			float SampleRate  = 1.0f / (float)DataSampleRate * 100.0f;
			ImGui::Text(
				"INFO: [Min]: %.3f [Max]: %.3f [Total]: %u [Mem]: %.3f MiB [SampleRate]: %.4f%% [Channels]: %u [Submit]: %.1f%%",
				ViewWindowYCoordLimit.x,
				ViewWindowYCoordLimit.y,
				DatasetTotalSize,
				SampleUsage, 
				SampleRate,
				SampleDatasetPoints.size(),
				float((double)SubmitTotal / (double)DatasetTotalSize) * 100.0f
			);
			ViewWindowDwWidthRuler = max(
				ImGui::CalcTextSize(to_string(ViewWindowYCoordLimit.x).c_str()).x,
				ImGui::CalcTextSize(to_string(ViewWindowYCoordLimit.y).c_str()).x
			) - 3.2f;
			ViewWindowSize = 
				ImGui::GetWindowSize() - ImVec2(IMGUI_ITEM_SPAC * 3.0f + ViewWindowDwWidthRuler, 96.0f);
			ViewWindowDrawLength = ViewWindowSize.x * 2.0f;

			CoreViewWindowRender(&SampleDatasetPoints);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.2f);
			if (ImGui::Button("OpenSetting", ImVec2(160.0f, 0.0f)))
				SettingWindowFlag = !SettingWindowFlag;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 160.0f - IMGUI_ITEM_SPAC * 3.0f);
			ImGui::SliderFloat("##SCPOSX", &ViewWindowScrollxPos, 0.0f, 1.0f);
		}
		ImGui::End();
		ImGui::PopStyleColor(13);

		SourceDataSubmit(10240);
	}
}