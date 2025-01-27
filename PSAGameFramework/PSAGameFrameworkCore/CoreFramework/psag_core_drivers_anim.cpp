// psag_core_drivers_anim.
// game start animation player.
#define ENABLE_PSAG_IMGUI_MATHS
#include "psag_core_drivers.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PsagFrameworkAnim {
    namespace MsTools = GameManagerCore::GameMathsTools;

    vector<HeaderImage> PsagStartAnimResource::HeaderImages = {};

    float PsagStartAnimResource::ImagesPlayTimeSec = 1.0f;
    float PsagStartAnimResource::ImagesLerpTrans   = 1.0f;

    PsagStartAnimPlayer::~PsagStartAnimPlayer() {
        // free static resource.
        for (auto& Texture : HeaderImages)
            glDeleteTextures(1, &Texture.ImageTexture);

        PushLogger(LogInfo, PSAGM_DRIVE_ANIM_LABEL,
            "start_anim playback complete, time(s): %.3f",
            (float)HeaderImages.size() * ImagesPlayTimeSec
        );
        PushLogger(LogInfo, PSAGM_DRIVE_ANIM_LABEL, "free textures resource.");
    }

    bool PsagStartAnimPlayer::PlayStartAnimation() {
        if (HeaderImages.empty()) return false;

        ImVec2 ImWindowSize = ImVec2(WindowSize.vector_x, WindowSize.vector_y);
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImWindowSize);
        ImGuiWindowFlags WinFlags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;
        // full window image draw.
        ImGui::Begin("##PSAG2DSTARTANIM", (bool*)NULL, WinFlags);
        {
            // imgui api draw image.
            ImGui::Image(
                IMGUI_TEXTURE_ID(HeaderImages[ImagesIndexCount].ImageTexture),
                ImWindowSize, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                PsagConvectVec4ToImVec4(HeaderImages[ImagesIndexCount].ImageBlend)
            );
        }
        ImGui::End();

        float RateScale = 120.0f / ImGui::GetIO().Framerate;
        MsTools::CalcFuncLerpVec4(
            &HeaderImages[ImagesIndexCount].ImageBlend,
            &LerpTargetBlend[TragetIndexCount],
            ImagesLerpTrans * 0.02f * RateScale
        );

        // check lerp complete. diff v < 0.05 & !flag
        if (MsTools::CalcFuncAvgDiffeVec4(
            LerpTargetBlend[TragetIndexCount], HeaderImages[ImagesIndexCount].ImageBlend)
            < 0.05f && LerpStatusFlag == 0
        ) {
            LerpStatusFlag = 1;
            TimerTemp = chrono::system_clock::now();
        }
        // check lerp complete. diff v < 0.05 & !flag
        if (MsTools::CalcFuncAvgDiffeVec4(
            LerpTargetBlend[TragetIndexCount], HeaderImages[ImagesIndexCount].ImageBlend)
            > -0.05f && LerpStatusFlag == 2
        ) {
            // reset 'target' index & reset status flag.
            TragetIndexCount = 1;
            LerpStatusFlag = 0;
            // images index count.
            ++ImagesIndexCount;
        }
        // check image play time, sec => ms.
        chrono::milliseconds PlayTime = chrono::milliseconds(int64_t(ImagesPlayTimeSec * 1000.0f));
        if (chrono::system_clock::now() - TimerTemp > PlayTime && LerpStatusFlag == 1) {
            // 'target' index count &  set status flag.
            --TragetIndexCount;
            LerpStatusFlag = 2;
        }
        return ImagesIndexCount < HeaderImages.size();
    }

    bool PsagStartAnimLoad::AnimImageADD(const ImageRawData& image) {
        PsagLow::PsagSupGraphicsOper::PsagGraphicsTextureView CreateTexView;

        if (!CreateTexView.CreateTexViewData(image)) {
            PushLogger(LogError, PSAGM_DRIVE_ANIM_LABEL, "start_anim loader failed create texture.");
            return false;
        }
        HeaderImage ImageTemp = {};
        ImageTemp.ImageBlend   = Vector4T<float>(0.0f, 0.0f, 0.0f, 0.0f);
        ImageTemp.ImageTexture = CreateTexView.CreateTexture().Texture;
        PushLogger(LogInfo, PSAGM_DRIVE_ANIM_LABEL, "start_anim loader create texture: %u x %u",
            image.Width, image.Height
        );
        // add image(texture) resource.
        HeaderImages.push_back(ImageTemp);
        return true;
    }
}