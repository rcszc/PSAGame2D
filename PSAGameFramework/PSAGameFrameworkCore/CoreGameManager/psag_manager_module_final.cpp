// psag_manager_module_final.
#include "psag_manager_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameManagerCore {
	namespace GameSceneForFinal {

		void FinalParamsLerpSystem::ParamsLerpFloat1(float* value, float target) {
			*value += (target - *value) * 0.025f * LERP_TIMESETP_SEPPD;
		}

		void FinalParamsLerpSystem::ParamsLerpFloat2(Vector2T<float>* value, const Vector2T<float>& target) {
			ParamsLerpFloat1(&value->vector_x, target.vector_x);
			ParamsLerpFloat1(&value->vector_y, target.vector_y);
		}

		void FinalParamsLerpSystem::ParamsLerpFloat3(Vector3T<float>* value, const Vector3T<float>& target) {
			ParamsLerpFloat1(&value->vector_x, target.vector_x);
			ParamsLerpFloat1(&value->vector_y, target.vector_y);
			ParamsLerpFloat1(&value->vector_z, target.vector_z);
		}

		void GameFinalProcessing::RenderDebugParamsPanel(const char* name) {
#if PSAG_DEBUG_MODE
			ImGui::PushStyleColor(ImGuiCol_ChildBg,       ImVec4(0.16f, 0.0f, 0.16f, 0.92f));
			ImGui::PushStyleColor(ImGuiCol_TitleBg,       ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
			ImGui::PushStyleColor(ImGuiCol_WindowBg,      ImVec4(0.1f, 0.1f, 0.1f, 0.92f));

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.92f, 1.0f));
			ImGui::Begin(name);
			{
				ImGui::BeginChild("##SETTING0", ImVec2(ImGui::GetWindowSize().x, 128.0f));
				{
					ImGui::SetCursorPosY(8.0f);
					ImGui::Indent(8.0f);
					ImGui::SliderFloat ("FilterColorAvg", &FrameworkFinalParamsPtr->GameSceneFilterAVG,         0.0f, 2.0f);
					ImGui::SliderFloat3("FilterColorRGB",  FrameworkFinalParamsPtr->GameSceneFilterCOL.data(),  0.0f, 2.0f);
					ImGui::SliderFloat2("BloomBlend",      FrameworkFinalParamsPtr->GameSceneBloomBlend.data(), 0.2f, 4.0f);
					
					int32_t RadiusValueTemp = (int32_t)FrameworkFinalParamsPtr->GameSceneBloomRadius;
					if (ImGui::SliderInt("BloomRadius", &RadiusValueTemp, 1, 31))
						FrameworkFinalParamsPtr->GameSceneBloomRadius = (uint32_t)RadiusValueTemp;
					ImGui::Unindent(8.0f);
				}
				ImGui::EndChild();

				ImGui::BeginChild("##SETTING1", ImVec2(ImGui::GetWindowSize().x, 100.0f));
				{
					ImGui::SetCursorPosY(8.0f);
					ImGui::Indent(8.0f);
					ImGui::SliderFloat3("FinalFragBlend",     FrameworkFinalParamsPtr->GameSceneOutColor.data(),    0.0f, 2.0f);
					ImGui::SliderFloat ("FinalFragContrast", &FrameworkFinalParamsPtr->GameSceneOutContrast,        0.0f, 2.0f);
					ImGui::SliderFloat2("FinalFragVignette",  FrameworkFinalParamsPtr->GameSceneOutVignette.data(), 0.0f, 2.0f);
					ImGui::Unindent(8.0f);
				}
				ImGui::EndChild();

				ImGui::BeginChild("##SETTING2", ImVec2(ImGui::GetWindowSize().x, 156.0f));
				{
					ImGui::SetCursorPosY(8.0f);
					ImGui::Indent(8.0f);
					ImGui::SliderFloat ("LightCollColorAvg",   &FrameworkFinalParamsPtr->LightCollisionValue, 0.0f, 1.0f);
					ImGui::SliderFloat3("LightColor",           FrameworkFinalParamsPtr->LightColor.data(),   0.0f, 1.0f);
					ImGui::SliderFloat ("LightIntensity",      &FrameworkFinalParamsPtr->LightIntensity,      0.0f, 1.0f);
					ImGui::SliderFloat ("LightIntensityDecay", &FrameworkFinalParamsPtr->LightIntensityDecay, 0.0f, 1.0f);
					ImGui::SliderInt   ("LightSample",         &FrameworkFinalParamsPtr->LightSampleStep,      1, 512);
					ImGui::Unindent(8.0f);
				}
				ImGui::EndChild();

				ImGui::BeginChild("##TYPE", ImVec2(ImGui::GetWindowSize().x, 100.0f));
				{
					ImGui::SetCursorPosY(8.0f);
					ImGui::Indent(8.0f);
					for (size_t i = 0; i < PresetFinalParams.size(); ++i) {
						if (ImGui::RadioButton(PresetFinalParams[i].first.c_str(), DebugTypeIndex == i))
							DebugTypeIndex = i;
					}
					TYPE_PARAMS(DebugTypeIndex);
					ImGui::Unindent(8.0f);
				}
				ImGui::EndChild();
			}
			ImGui::End();
			ImGui::PopStyleColor(5);
#endif
		}

		void GameFinalProcessing::RunFinalProcessing(float time_step) {
			ParamsLerpFloat1(&FrameworkFinalParamsPtr->GameSceneFilterAVG,  CurrentFinalParams.GameSceneFilterAVG);
			ParamsLerpFloat3(&FrameworkFinalParamsPtr->GameSceneFilterCOL,  CurrentFinalParams.GameSceneFilterCOL);
			ParamsLerpFloat2(&FrameworkFinalParamsPtr->GameSceneBloomBlend, CurrentFinalParams.GameSceneBloomBlend);

			ParamsLerpFloat3(&FrameworkFinalParamsPtr->GameSceneOutColor,    CurrentFinalParams.GameSceneOutColor);
			ParamsLerpFloat1(&FrameworkFinalParamsPtr->GameSceneOutContrast, CurrentFinalParams.GameSceneOutContrast);
			ParamsLerpFloat2(&FrameworkFinalParamsPtr->GameSceneOutVignette, CurrentFinalParams.GameSceneOutVignette);

			ParamsLerpFloat1(&FrameworkFinalParamsPtr->LightCollisionValue, CurrentFinalParams.LightCollisionValue);
			ParamsLerpFloat3(&FrameworkFinalParamsPtr->LightColor,          CurrentFinalParams.LightColor);
			ParamsLerpFloat1(&FrameworkFinalParamsPtr->LightIntensity,      CurrentFinalParams.LightIntensity);
			ParamsLerpFloat1(&FrameworkFinalParamsPtr->LightIntensityDecay, CurrentFinalParams.LightIntensityDecay);

			FrameworkFinalParamsPtr->GameSceneBloomRadius = CurrentFinalParams.GameSceneBloomRadius;
			FrameworkFinalParamsPtr->LightSampleStep = CurrentFinalParams.LightSampleStep;

			LERP_TIMESETP_SEPPD = time_step;
		}
	}
}