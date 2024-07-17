// TestDemoStar.
#include "TestDemoStar.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace TActor = GameActorCore::Type;

void StarDemoClass::CreateStarActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed) {
    // ******************************** TEST Star Actor 对象 ********************************
    GameActorCore::GameActorActuatorDESC ConfigStarActor;

    ConfigStarActor.EnablePawn = false;

    ConfigStarActor.ActorPhysicsWorld   = "MyPhyWorld";
    ConfigStarActor.ActorShaderResource = ActorShaderStar;

    ConfigStarActor.InitialPhysics  = Vector2T<float>(7.0f, 2.0f);
    ConfigStarActor.InitialPosition = PosBegin;
    ConfigStarActor.InitialScale    = Vector2T<float>(0.2f, 0.2f);
    ConfigStarActor.InitialSpeed    = PosSpeed;
    
    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_star"), ConfigStarActor);
}

void StarDemoClass::CreateRandomStarActors(size_t number) {
    // ******************************** TEST Star Actors 对象 ********************************

    GameToolsCore::Random::GenerateRandom2D RandomCreate;

    auto StarRandomParam = Vector2T<Vector2T<float>>(Vector2T<float>(-320.0f, 320.0f), Vector2T<float>(-320.0f, 320.0f));
    
    RandomCreate.RandomSeedMode(GameToolsCore::Random::TimeSeedMicroseconds);
    RandomCreate.CreateRandomDataset(number, StarRandomParam, 10.0f);
    RandomCreate.DatasetCropCircle(Vector2T<float>(0.0f, 0.0f), 10.0f);

    for (const auto& StarPos : RandomCreate.RandomCoordGroup)
        CreateStarActor(StarPos, Vector2T<float>());
}

bool StarDemoClass::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
   // ******************************** TEST 全局背景处理 ********************************

    PsagLow::PsagSupGraphicsOper::PsagGraphicsImageRawDat DecodeRawImage;

    PsagLow::PsagSupFilesysLoaderBin TestBinLoad1("Test/TEST_BACK1.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad2("Test/TEST_BACK2.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad3("Test/TEST_BACK3.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad4("Test/TEST_BACK4.png");

    PsagGameSys::SysBackground CreateBg;

    CreateBg.LayerDataPush(DecodeRawImage.DecodeImageRawData(TestBinLoad4.GetDataBinary()));
    CreateBg.LayerDataPush(DecodeRawImage.DecodeImageRawData(TestBinLoad3.GetDataBinary()));
    CreateBg.LayerDataPush(DecodeRawImage.DecodeImageRawData(TestBinLoad2.GetDataBinary()));
    CreateBg.LayerDataPush(DecodeRawImage.DecodeImageRawData(TestBinLoad1.GetDataBinary()));

    // 创建全局背景处理对象.
    CreateBg.CreateBackground(WinSize);

    // ******************************** TEST 粒子系统 ********************************

    PsagLow::PsagSupFilesysLoaderBin ParticleTexture("Test/ParticleImgTest.png");
    AshesParticles = new
        GraphicsEngineParticle::PsagGLEngineParticle(WinSize, DecodeRawImage.DecodeImageRawData(ParticleTexture.GetDataBinary()));
    AshesParticles->SetParticleTwisted(1.0f);

    // ******************************** TEST Physics & Graphics 资源 ********************************

    GameActorCore::GameActorPhysicalWorld CreatePhyWorld("MyPhyWorld", 1);

    // 创建Actor着色器资源.
    ActorShaderPawn = new GameActorCore::GameActorShader(ActorFragPawn, WinSize); ActorShaderPawn->CreateShaderRes();
    ActorShaderStar = new GameActorCore::GameActorShader(ActorFragStar, WinSize); ActorShaderStar->CreateShaderRes();

    PsagLow::PsagSupFilesysLoaderBin BrickTexture("Test/TEST_BOX.png");

    BrickShader = new GameActorCore::GameActorShader(GameActorScript::PsagShaderBrickPrivateFS, WinSize);
    BrickShader->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(BrickTexture.GetDataBinary()));
    BrickShader->CreateShaderRes();

    // ******************************** TEST Actor类型名称绑定 ********************************

    TActor::ActorTypeAllotter.ActorTypeCreate("actor_pawn");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_star");

    // ******************************** TEST PawnActor对象 ********************************

    GameActorCore::GameActorActuatorDESC ConfigPawnActor;

    ConfigPawnActor.ActorPhysicsWorld   = "MyPhyWorld";
    ConfigPawnActor.ActorShaderResource = ActorShaderPawn;

    ConfigPawnActor.ForceClacEnable = true;
    // 使用默认配置.
    GameActorCore::GameActorHealthDESC PawnActorHealthDESC = {};
    // config hp system.
    ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

    ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);

    PawnActorCode = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_pawn"), ConfigPawnActor);

    // ******************************** TEST 静态地图'Brick'对象 ********************************

    GameToolsCore::Random::GenerateRandom2D RandomCreate;

    auto BrickRandomParam = Vector2T<Vector2T<float>>(Vector2T<float>(-500.0f, 500.0f), Vector2T<float>(-500.0f, 500.0f));

    RandomCreate.RandomSeedMode(GameToolsCore::Random::TimeSeedMicroseconds);
    RandomCreate.CreateRandomDataset(72, BrickRandomParam, 50.0f);
    RandomCreate.DatasetCropCircle(Vector2T<float>(0.0f, 0.0f), 10.0f);

    GameBrickCore::GameBrickActuatorDESC BricksDESC;

    BricksDESC.BrickPhysicsWorld   = "MyPhyWorld";
    BricksDESC.BrickShaderResource = BrickShader;

    GameToolsCore::Random::GenerateRandom1D RandomSizeCreate;
    RandomSizeCreate.RandomSeedMode(GameToolsCore::Random::TimeSeedMicroseconds);

    for (const auto& BrickPos : RandomCreate.RandomCoordGroup) {
        // set brick scale_size.
        BricksDESC.InitialScale = 
            Vector2T<float>(RandomSizeCreate.CreateRandomValue(1.0f, 2.0f), RandomSizeCreate.CreateRandomValue(1.0f, 2.0f));
        BricksDESC.InitialRotate = RandomSizeCreate.CreateRandomValue(-90.0f, 90.0f);
        // set brick position.
        BricksDESC.InitialPosition = BrickPos;
        TestGameBricks.CreateGameActor(BricksDESC);
    }

    CreateRandomStarActors(100);
    return true;
}

void StarDemoClass::LogicCloseFree() {

    GameActorCore::GameActorPhysicalWorld DeletePhyWorld("MyPhyWorld", 2);

    delete ActorShaderPawn;
    delete ActorShaderStar;
    delete BrickShader;

    delete AshesParticles;
}

bool StarDemoClass::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

    AshesParticles->UpdateParticleData();
    AshesParticles->RenderParticleFX();

    for (auto& Star : *TestGameActors.GetSourceData()) {
        if (Star.second->ActorGetCollision().ActorUniqueCode != NULL &&
            Star.second->ActorGetPrivate().ActorTypeCode == TActor::ActorTypeAllotter.ActorTypeIs("actor_star")
            ) {
            GraphicsEngineParticle::ParticleGenerator CreatePartc;
            CreatePartc.ConfigCreateMode(GraphicsEngineParticle::PrtcPoints);
            CreatePartc.ConfigCreateNumber(20);
            CreatePartc.ConfigLifeDispersion(Vector2T<float>(512.0f, 1024.0f));
            CreatePartc.ConfigSizeDispersion(Vector2T<float>(0.42f, 1.28f));

            CreatePartc.ConfigRandomColorSystem(
                Vector2T<float>(0.0f, 0.0f), Vector2T<float>(0.42f, 1.78f), Vector2T<float>(0.42f, 1.78f), GraphicsEngineParticle::ChannelsRGB
            );
            CreatePartc.ConfigRandomDispersion(
                Vector2T<float>(-1.0f, 1.0f),
                Vector2T<float>(0.0f, 0.0f),
                Vector3T<float>(Star.second->ActorGetPosition().vector_x, Star.second->ActorGetPosition().vector_y, 0.0f)
            );
            AshesParticles->ParticleCreate(&CreatePartc);
            TestGameActors.DeleteGameActor(Star.second->ActorGetPrivate().ActorUniqueCode);

            auto CollisionPtr = TestGameActors.FindGameActor(Star.second->ActorGetCollision().ActorUniqueCode);

            if (CollisionPtr != nullptr) {
                if (CollisionPtr->ActorGetPrivate().ActorTypeCode == TActor::ActorTypeAllotter.ActorTypeIs("actor_pawn")) {
                    ++EatStarCount;
                    ++EatStarCountTotal;
                }
            }
        }
    }

    // 吃掉 10 个随机生成 12 个.
    if (EatStarCount > 10) {
        CreateRandomStarActors(12);
        EatStarCount = NULL;
    }

    TestGameBricks.RunAllGameActor();

    TestGameActors.RunAllGameActor();
    TestGameActors.UpdateManagerData();

    RunningState.BackShaderParams->BackgroundColor = Vector4T<float>(0.0f, 1.0f, 0.92f, 1.0f);
    RunningState.BackShaderParams->BackgroundVisibility = 0.642f;
    RunningState.PostShaderParams->GameSceneFilterAVG   = 0.254f;
    RunningState.PostShaderParams->GameSceneBloomRadius = 18;

    return true;
}

void GuiTitleNumber(Vector2T<float> position, size_t star_count) {
    ImGui::SetNextWindowPos(ImVec2(position.vector_x, position.vector_y));
    ImGui::SetNextWindowSize(ImVec2(320.0f, 160.0f));

    ImGui::Begin("StarNumWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    {
        ImGui::SetWindowFontScale(5.0f);
        ImVec2 TextWidth = ImGui::CalcTextSize(std::to_string(star_count).c_str());
        float CenterPosition = (ImGui::GetWindowSize().x - TextWidth.x) * 0.5f;

        ImGui::Indent(CenterPosition);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%u", star_count);
        ImGui::Unindent(CenterPosition);
    }
    ImGui::End();
}

bool StarDemoClass::LogicEventLoopGui(GameLogic::FrameworkParams& RunningState) {
    // 修改 ImGui 控件颜色.
    ImGui::PushStyleColor(ImGuiCol_WindowBg,       ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.72f, 0.72f, 0.72f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg,        ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,  ImVec4(0.58f, 0.58f, 0.58f, 0.92f));

    GameDebugGuiWindow::DebugWindowGuiFPS("PSA-Game2D STAR", FramerateParams);

    auto PawnActorObj = TestGameActors.FindGameActor(PawnActorCode);

    //GameDebugGuiWindow::DebugWindowGuiActors(TestGameActors.GetSourceData());
    GameDebugGuiWindow::DebugWindowGuiActor("PawnActor", PawnActorObj);

    CameraScale = 2.7f;

    /*
    ImGui::Begin("TestSample");

    ImGui::SliderInt("STEP", &LightSamplerStep, 1, 384);
    RunningState.PostShaderParams->LightSampleStep = LightSamplerStep;

    ImGui::SliderFloat("LIGHT", &RunningState.PostShaderParams->LightIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("DECAY", &RunningState.PostShaderParams->LightIntensityDecay, 0.0f, 1.0f);

    ImGui::ColorEdit3("COLOR", RunningState.PostShaderParams->LightColor.data());
    ImGui::End();
    */

    PawnActorObj->ActorApplyForceMove(Vector2T<float>());
    PawnActorObj->ActorApplyForceRotate(0.0f);

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        PawnActorObj->ActorApplyForceMove(Vector2T<float>(0.0f, 10.0f));
        CameraScale = 1.8f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        PawnActorObj->ActorApplyForceMove(Vector2T<float>(0.0f, -10.0f));
        CameraScale = 1.8f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        PawnActorObj->ActorApplyForceMove(Vector2T<float>(10.0f, 0.0f));
        PawnActorObj->ActorApplyForceRotate(2.0f);
        CameraScale = 1.0f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        PawnActorObj->ActorApplyForceMove(Vector2T<float>(-10.0f, 0.0f));
        PawnActorObj->ActorApplyForceRotate(-2.0f);
        CameraScale = 1.0f;
    }

    if (ImGui::IsMouseDown(1)) {
        RunningState.PostShaderParams->LightPosition = Vector2T<float>(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
    }

    auto ToWindowCoord = PawnActorObj->ActorConvertVirCoord(RunningState.WindowResolution);
    
    if (ToWindowCoord.vector_x > RunningState.WindowResolution.vector_x * 0.4f)
        CameraPosition.vector_x -= abs(PawnActorObj->ActorGetMoveSpeed().vector_x) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_x < RunningState.WindowResolution.vector_x * 0.6f)
        CameraPosition.vector_x += abs(PawnActorObj->ActorGetMoveSpeed().vector_x) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_y > RunningState.WindowResolution.vector_y * 0.4f)
        CameraPosition.vector_y += abs(PawnActorObj->ActorGetMoveSpeed().vector_y) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_y < RunningState.WindowResolution.vector_y * 0.6f)
        CameraPosition.vector_y -= abs(PawnActorObj->ActorGetMoveSpeed().vector_y) * RunningState.GameRunTimeStep;

    RunningState.CameraParams->MatrixScale.vector_x +=
        (CameraScale - RunningState.CameraParams->MatrixScale.vector_x) * 0.0025f;
    RunningState.CameraParams->MatrixScale.vector_y = RunningState.CameraParams->MatrixScale.vector_x;
    RunningState.CameraParams->MatrixPosition = CameraPosition;

    GuiTitleNumber(Vector2T<float>(ImGui::GetIO().DisplaySize.x / 2.0f - 160.0f, 72.0f), EatStarCountTotal);
    ImGui::PopStyleColor(8);
    return true;
}