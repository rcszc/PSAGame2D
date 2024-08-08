// TestDemoStar.
#include "TestDemoStar.h"

using namespace std;
using namespace PSAG_LOGGER;

void StarDemoClass::CreateStarActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed) {
    // ******************************** TEST Star Actor 对象 ********************************
    PsagActor::ActorDESC ConfigStarActor;

    ConfigStarActor.EnableTrans = false;

    ConfigStarActor.ActorPhysicsWorld   = "TestPhyWorld";
    ConfigStarActor.ActorShaderResource = ActorShaderStar;

    ConfigStarActor.InitialPhysics  = Vector2T<float>(7.0f, 2.0f);
    ConfigStarActor.InitialPosition = PosBegin;
    ConfigStarActor.InitialScale    = Vector2T<float>(0.2f, 0.2f);
    ConfigStarActor.InitialSpeed    = PosSpeed;
    
    TestGameActors.CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_star"), ConfigStarActor);
}

void StarDemoClass::CreateRandomStarActors(size_t number) {
    // ******************************** TEST Star Actors 对象 ********************************
    PsagManager::Tools::Random::GenerateRandom2D RandomCreate;

    auto StarRandomParam = Vector2T<Vector2T<float>>(Vector2T<float>(-248.0f, 248.0f), Vector2T<float>(-248.0f, 248.0f));
    
    RandomCreate.RandomSeedMode(PsagManager::Tools::Random::TimeSeedMicroseconds);
    RandomCreate.CreateRandomDataset(number, StarRandomParam, 10.0f);

    for (const auto& StarPos : RandomCreate.RandomCoordGroup)
        CreateStarActor(StarPos, Vector2T<float>());
}

bool StarDemoClass::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
   // ******************************** TEST 全局背景处理 ********************************

    PsagLow::PsagSupGraphicsOper::PsagGraphicsImageRawDat DecodeRawImage;

    PsagLow::PsagSupFilesysLoaderBin TestBinLoad1("Test/TEST_BACK1.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad2("Test/TEST_BACK2.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad3("Test/TEST_BACK3.png");

    PsagGameSys::SysBackground CreateBg;

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

    PsagActor::OperPhysicalWorld CreatePhyWorld("TestPhyWorld", 1);

    // 创建Actor着色器资源.
    ActorShaderPawn = new PsagActor::ActorRender(ActorFragPawn, WinSize); ActorShaderPawn->CreateShaderResource();
    ActorShaderStar = new PsagActor::ActorRender(ActorFragStar, WinSize); ActorShaderStar->CreateShaderResource();

    PsagLow::PsagSupFilesysLoaderBin FxTexture1("Test/TEST_NOISE.png");
    PsagLow::PsagSupFilesysLoaderBin FxTexture2("Test/TEST_LASER.png");

    // 特效着色器: 能量球.
    ActorShaderFX1 = new PsagActor::ActorRender(ActorFragFX1, WinSize);
    ActorShaderFX1->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(FxTexture1.GetDataBinary()));
    ActorShaderFX1->CreateShaderResource();

    // 特效着色器: 闪电.
    ActorShaderFX2 = new PsagActor::ActorRender(ActorFragFX2, WinSize);
    ActorShaderFX2->CreateShaderResource();

    // 特效着色器: 能量束.
    ActorShaderFX3 = new PsagActor::ActorRender(ActorFragFX3, WinSize);
    ActorShaderFX3->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(FxTexture2.GetDataBinary()));
    ActorShaderFX3->CreateShaderResource();

    PsagLow::PsagSupFilesysLoaderBin BrickTexture("Test/TEST_BOX.png");

    BrickShader = new PsagActor::ActorRender(GameActorScript::PsagShaderBrickPrivateFS, WinSize);
    BrickShader->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(BrickTexture.GetDataBinary()));
    BrickShader->CreateShaderResource();

    // ******************************** TEST Actor类型名称绑定 ********************************

    PsagActorType::ActorTypeAllotter.ActorTypeCreate("actor_pawn");
    PsagActorType::ActorTypeAllotter.ActorTypeCreate("actor_star");
    PsagActorType::ActorTypeAllotter.ActorTypeCreate("actor_fx");

    // ******************************** TEST PawnActor对象 ********************************

    PsagActor::ActorDESC ConfigPawnActor;

    ConfigPawnActor.ActorPhysicsWorld   = "TestPhyWorld";
    ConfigPawnActor.ActorShaderResource = ActorShaderPawn;

    ConfigPawnActor.ForceClacEnable = false;
    // 使用默认配置.
    PsagActor::ActorHpDESC PawnActorHealthDESC = {};
    // config hp system.
    ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

    ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);

    PawnActorCode = TestGameActors.CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_pawn"), ConfigPawnActor);
    
    // ******************************** TEST 静态地图'Brick'对象 ********************************

    PsagActor::BrickDESC BricksDESC;

    BricksDESC.BrickPhysicsWorld   = "TestPhyWorld";
    BricksDESC.BrickShaderResource = BrickShader;

    CollectEngineRandom::GenerateRandom1D RandomSizeCreate;
    RandomSizeCreate.RandomSeedMode(CollectEngineRandom::TimeSeedMicroseconds);

    // 0.0 - 2.0PI, 0.0 - 360.0
    for (float i = 0.0f; i < 360.0f; i += 5.0f) {
        // set brick scale_size.
        BricksDESC.InitialScale = 
            Vector2T<float>(RandomSizeCreate.CreateRandomValue(1.0f, 1.5f), RandomSizeCreate.CreateRandomValue(1.0f, 1.5f));
        BricksDESC.InitialRotate = RandomSizeCreate.CreateRandomValue(-90.0f, 90.0f);
        // set brick position.
        BricksDESC.InitialPosition = Vector2T<float>(sin(i * (PSAG_M_PI / 180.0f)) * 350.0f, cos(i * (PSAG_M_PI / 180.0f)) * 350.0f);
        TestGameBricks.CreateGameBrick(BricksDESC);
    }

    CreateRandomStarActors(72);
    FpsDebug = new GameDebugGuiWindow::DebugWindowGuiFPS("GameFPS", 1200.0f);
    PlayerPawn = new PsagManager::Tools::Pawn::GamePlayerPawn(Vector2T<float>(128.0f, 128.0f));
    return true;
}

void StarDemoClass::LogicCloseFree() {

    PsagActor::OperPhysicalWorld DeletePhyWorld("TestPhyWorld", 2);

    delete ActorShaderPawn;
    delete ActorShaderStar;
    delete BrickShader;

    delete AshesParticles;
}

bool StarDemoClass::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
    auto PawnActorObj = TestGameActors.FindGameActor(PawnActorCode);

    auto CollisionActorObj = TestGameActors.FindGameActor(PawnActorObj->ActorGetCollision().ActorUniqueCode);

    if (CollisionActorObj != nullptr) {
        if (CollisionActorObj->ActorGetPrivate().ActorTypeCode == PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_star")) {

            GraphicsEngineParticle::ParticleGenerator CreatePartc;
            CreatePartc.ConfigCreateMode(GraphicsEngineParticle::ParticlesGenMode::PrtcPoints);
            CreatePartc.ConfigCreateNumber(20);
            CreatePartc.ConfigLifeDispersion(Vector2T<float>(512.0f, 1024.0f));
            CreatePartc.ConfigSizeDispersion(Vector2T<float>(0.42f, 1.28f));

            CreatePartc.ConfigRandomColorSystem(
                Vector2T<float>(0.0f, 0.0f), Vector2T<float>(0.42f, 1.78f), Vector2T<float>(0.42f, 1.78f),
                GraphicsEngineParticle::ParticlesGenMode::ChannelsRGB
            );
            CreatePartc.ConfigRandomDispersion(
                Vector2T<float>(-1.0f, 1.0f),
                Vector2T<float>(0.0f, 0.0f),
                Vector3T<float>(CollisionActorObj->ActorGetPosition().vector_x, CollisionActorObj->ActorGetPosition().vector_y, 0.0f)
            );
            AshesParticles->ParticleCreate(&CreatePartc);
            TestGameActors.DeleteGameActor(CollisionActorObj->ActorGetPrivate().ActorUniqueCode);

            ++EatStarCount;
            ++EatStarCountTotal;
        }
    }

    // 吃掉 10 个随机生成 12 个.
    if (EatStarCount > 10) {
        CreateRandomStarActors(12);
        EatStarCount = NULL;
    }

    TestGameBricks.RunAllGameBrick();

    TestGameActors.RunAllGameActor();
    TestGameActors.UpdateManagerData();

    AshesParticles->UpdateParticleData();
    AshesParticles->RenderParticleFX();

    if (ActorUltimateFX != nullptr)
        ActorUltimateFX->StarDemoFxRender();

    PlayerPawn->PlayerPawnRun(32.0f);

    RunningState.BackShaderParams->BackgroundVisibility = 1.48f;
    RunningState.PostShaderParams->GameSceneFilterAVG   = 0.254f;
    RunningState.PostShaderParams->GameSceneBloomRadius = 18;

    return true;
}

void GuiTitleNumber(Vector2T<float> position, float value) {
    ImGui::SetNextWindowPos(ImVec2(position.vector_x, position.vector_y));
    ImGui::SetNextWindowSize(ImVec2(640.0f, 160.0f));

    ImGui::Begin("StarNumWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    {
        ImGui::SetWindowFontScale(5.0f);
        ImVec2 TextWidth = ImGui::CalcTextSize(std::to_string(value).c_str()) * 0.5;
        float CenterPosition = (ImGui::GetWindowSize().x - TextWidth.x) * 0.5f;

        ImGui::Indent(CenterPosition);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%.2f", value);
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

    auto PawnActorObj = TestGameActors.FindGameActor(PawnActorCode);

    //GameDebugGuiWindow::DebugWindowGuiActors(TestGameActors.GetSourceData());
    GameDebugGuiWindow::DebugWindowGuiActorPawn("PawnActor", PawnActorObj);

    CameraScale = 1.25f;

    /*
    ImGui::Begin("TestSample");

    ImGui::SliderInt("STEP", &LightSamplerStep, 1, 384);
    RunningState.PostShaderParams->LightSampleStep = LightSamplerStep;

    ImGui::SliderFloat("LIGHT", &RunningState.PostShaderParams->LightIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("DECAY", &RunningState.PostShaderParams->LightIntensityDecay, 0.0f, 1.0f);

    ImGui::ColorEdit3("COLOR", RunningState.PostShaderParams->LightColor.data());
    ImGui::End();

    if (ImGui::IsMouseDown(1)) {
        RunningState.PostShaderParams->LightPosition = Vector2T<float>(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
    }
    */
    FpsDebug->RenderingWindowGui();

    PawnActorObj->ActorApplyForceMove(Vector2T<float>());
    PawnActorObj->ActorApplyForceRotate(0.0f);

    PawnActorObj->ActorApplyForceMove(PlayerPawn->ControlMoveVector);
    //PawnActorObj->ActorApplyForceRotate(20.0f);
    //PawnActorObj->ActorApplyForceRotate(-20.0f);

    if (ImGui::IsKeyPressed(ImGuiKey_R, false))
        ActorUltimateFX->StarDemoFxFire();

    ImGui::Begin("TestWindow");

    ImGui::InputFloat2("FIRE POS", UltimateSettingPosition.data());
    ImGui::InputFloat("FIRE ANGLE", &UltimateSettingRotate);

    if (ImGui::Button("ULTIMATE CREATE") && ActorUltimateFX == nullptr) {
        ActorUltimateFX = new StarDemoFX(
            &TestGameActors, Vector3T<PsagActor::ActorRender*>(ActorShaderFX1, ActorShaderFX2, ActorShaderFX3),
            UltimateSettingPosition, UltimateSettingRotate
        );
    }
    ImGui::End();

    RunningState.CameraParams->MatrixRotate += (CameraRotate - RunningState.CameraParams->MatrixRotate) * 0.05f;

    auto ToWindowCoord = PawnActorObj->ActorConvertVirCoord(RunningState.WindowResolution);

    if (ToWindowCoord.vector_x > RunningState.WindowResolution.vector_x * 0.4f)
        CameraPosition.vector_x -= abs(PawnActorObj->ActorGetMoveSpeed().vector_x) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_x < RunningState.WindowResolution.vector_x * 0.6f)
        CameraPosition.vector_x += abs(PawnActorObj->ActorGetMoveSpeed().vector_x) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_y > RunningState.WindowResolution.vector_y * 0.4f)
        CameraPosition.vector_y += abs(PawnActorObj->ActorGetMoveSpeed().vector_y) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_y < RunningState.WindowResolution.vector_y * 0.6f)
        CameraPosition.vector_y -= abs(PawnActorObj->ActorGetMoveSpeed().vector_y) * RunningState.GameRunTimeStep;

    Vector2T<float> CameraOffset = {};
    if (ActorUltimateFX != nullptr) {

        CameraOffset = ActorUltimateFX->StarDemoFxCameraOffset(&CameraScale, &CameraRotate);
        GuiTitleNumber(Vector2T<float>(ImGui::GetIO().DisplaySize.x / 2.0f - 320.0f, 72.0f), ActorUltimateFX->StarDemoFxProgress() * 100.0f);

        CameraPosition.vector_x += (UltimateSettingPosition.vector_x - CameraPosition.vector_x) * 0.002f;
        CameraPosition.vector_y += (UltimateSettingPosition.vector_y - CameraPosition.vector_y) * 0.002f;
    }

    RunningState.CameraParams->MatrixScale.vector_x +=
        (CameraScale - RunningState.CameraParams->MatrixScale.vector_x) * 0.0025f;
    RunningState.CameraParams->MatrixScale.vector_y = RunningState.CameraParams->MatrixScale.vector_x;

    RunningState.CameraParams->MatrixPosition = 
        Vector2T<float>(CameraPosition.vector_x + CameraOffset.vector_x, CameraPosition.vector_y + CameraOffset.vector_y);

    ImGui::PopStyleColor(8);
    return true;
}