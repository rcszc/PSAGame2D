// FrameworkTest.
#include "FrameworkTest.h"

using namespace std;
using namespace PSAG_LOGGER;

bool DevTestClass::LogicInitialization() {
    
    Vector2T<uint32_t> RenderRes(1680, 945);
    PsagLow::PsagSupGraphicsTool::PsagGraphicsImageRawDat LoadRawImage;

    PsagLow::PsagSupFilesysLoaderBin TestBinLoad1("Test/TEST_BACK1.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad2("Test/TEST_BACK2.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad3("Test/TEST_BACK3.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad4("Test/TEST_BACK4.png");
    
    PsagGameSys::SysBackground CreateBg;

    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad4.GetDataBinary()));
    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad3.GetDataBinary()));
    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad2.GetDataBinary()));
    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad1.GetDataBinary()));

    CreateBg.CreateBackground(RenderRes);

    //PsagGameSys::SysFrontMask CreateMask;
    //CreateMask.CreateFrontMask(TestRawLoad.DecodeImageRawData(TestBinLoad0.GetDataBinary()));

    PsagLow::PsagSupGraphicsTool::PsagGraphicsImageRawDat DecRawImage;
    PsagLow::PsagSupFilesysLoaderBin LoadParticle("Test/ParticleImgTest.png");

    AshesParticles = new GraphicsEngineParticle::PsagGLEngineParticle(RenderRes, DecRawImage.DecodeImageRawData(LoadParticle.GetDataBinary()));
    
    PsagLow::PsagSupFilesysLoaderBin LoadSequence("Test/fx_fire_test.png");

    GraphicsEnginePVFX::SequencePlayer SequenceParams = {};

    SequenceParams.UaxisFrameNumber = 6.0f;
    SequenceParams.VaxisFrameNumber = 6.0f;
    SequenceParams.PlayerSpeedScale = 1.0f;

    TestCaptureView = new GraphicsEnginePVFX::PsagGLEngineFxCaptureView(RenderRes);
    TestSequence = new GraphicsEnginePVFX::PsagGLEngineFxSequence(
        DecRawImage.DecodeImageRawData(LoadSequence.GetDataBinary()), SequenceParams
    );

	return true;
}

void DevTestClass::LogicCloseFree() {
    delete AshesParticles;
    delete TestCaptureView;
}

bool DevTestClass::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

    AshesParticles->UpdateParticleData();
    AshesParticles->RenderParticleFX();

    TestCaptureView->CaptureContextBind();
    TestSequence->DrawFxSequence(TestColor);
    TestCaptureView->CaptureContextUnBind();

    if (CountTimer % 300 == 0) {
        GraphicsEngineParticle::ParticleGenerator CreatePartc;
        CreatePartc.ConfigCreateMode(GraphicsEngineParticle::PrtcPoints);
        CreatePartc.ConfigCreateNumber(10);
        CreatePartc.ConfigLifeDispersion(Vector2T<float>(1280.0f, 2560.0f));

        CreatePartc.ConfigRandomColorSystem(
            Vector2T<float>(0.42f, 1.28f), Vector2T<float>(0.42f, 1.28f), Vector2T<float>(0.42f, 1.28f), GraphicsEngineParticle::ChannelsRB
        );
        CreatePartc.ConfigRandomDispersion(
            Vector2T<float>(-1.0f, 1.0f),
            Vector2T<float>(0.0f, 0.0f)
        );
        AshesParticles->ParticleCreate(&CreatePartc);
    }

    // background blend color_inter calc.
    ColorAnimInter += ImVec4(BgBlendColorSet - ColorAnimInter) * 0.02f;

    // background visibility inter calc.
    RunningState.BackShaderParams->BackgroundVisibility +=
        (BgVisibilitySet - RunningState.BackShaderParams->BackgroundVisibility) * 0.02f;

    RunningState.BackShaderParams->BackgroundStrength.vector_x +=
        (BgBlendStrengthSet - RunningState.BackShaderParams->BackgroundStrength.vector_x) * 0.02f;

    ++CountTimer;
	return true;
}

bool DevTestClass::LogicEventLoopGui(GameLogic::FrameworkParams& RunningState) {

    ImGui::PushStyleColor(ImGuiCol_WindowBg,       ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.72f, 0.72f, 0.72f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg,        ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,  ImVec4(0.58f, 0.58f, 0.58f, 0.92f));

    ImGui::Begin("PSA-Game2D Debug");
    {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        
        ImGui::ColorEdit4("COLOR", &BgBlendColorSet.x);
        ImGui::SliderFloat("BLOOM", &BloomRadius, 1.0f, 32.0f);
        ImGui::SliderFloat("VISIB", &BgVisibilitySet, 0.1f, 2.0f);
        ImGui::SliderFloat("BLEND", &BgBlendStrengthSet, 0.1f, 2.0f);

        ImGui::ProgressBar(RunningState.BackShaderParams->BackgroundVisibility / 2.0f);
        ImGui::ProgressBar(RunningState.BackShaderParams->BackgroundStrength.vector_x / 2.0f);
        ImGui::ProgressBar(BloomRadius / 32.0f);
    }
    ImGui::End();

    ImGui::Begin("PSA-Game2D VFX - TEST");
    {
        ImGui::Image(IMTOOL_CVT_IMPTR(TestCaptureView->GetCaptureTexView()), ImVec2(512.0f, 512.0f));
        ImGui::ColorEdit4("COLOR", TestColor.data());
    }
    ImGui::End();

    ImGui::PopStyleColor(5);

    RunningState.PostShaderParams->GameSceneBloomRadius = (uint32_t)BloomRadius;
    RunningState.BackShaderParams->BackgroundStrength.vector_y = RunningState.BackShaderParams->BackgroundVisibility + 0.32f;
    
    RunningState.BackShaderParams->BackgroundColor = 
        Vector4T<float>(ColorAnimInter.x, ColorAnimInter.y, ColorAnimInter.z, ColorAnimInter.w);

	return true;
}