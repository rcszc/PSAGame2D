// TestDemoStar.
#include "TestDemoStar.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace TActor = GameActorCore::Type;

void TestClassStar::CreateStarActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed) {
    // ******************************** TEST Star Actor ���� ********************************
    GameActorCore::GameActorActuatorDESC ConfigStarActor;

    ConfigStarActor.ActorPhysicsWorld   = "MyPhyWorld";
    ConfigStarActor.ActorShaderResource = ActorShaderStar;

    ConfigStarActor.ControlFricMove   = 0.1f;
    ConfigStarActor.ControlFricRotate = 1.0f;
    ConfigStarActor.ControlFricScale  = 1.0f;

    ConfigStarActor.InitialPhysics  = Vector2T<float>(7.0f, 2.0f);
    ConfigStarActor.InitialPosition = PosBegin;
    ConfigStarActor.InitialScale    = Vector2T<float>(0.2f, 0.2f);
    ConfigStarActor.InitialSpeed    = PosSpeed;
    
    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_star"), ConfigStarActor);
}

void TestClassStar::CreateRandomStarActors(size_t number) {
    // ******************************** TEST Star Actors ���� ********************************

    GameToolsCore::Random::GenerateRandom2D RandomCreate;

    auto StarRandomParam = Vector2T<Vector2T<float>>(Vector2T<float>(-320.0f, 320.0f), Vector2T<float>(-320.0f, 320.0f));
    
    RandomCreate.RandomSeedMode(GameToolsCore::Random::TimeSeedMicroseconds);
    RandomCreate.CreateRandomDataset(number, StarRandomParam, 10.0f);
    RandomCreate.DatasetCropCircle(Vector2T<float>(0.0f, 0.0f), 10.0f);

    for (const auto& StarPos : RandomCreate.RandomCoordGroup)
        CreateStarActor(StarPos, Vector2T<float>());
}

bool TestClassStar::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
    // ******************************** TEST ȫ�ֱ������� ********************************

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

    // ����ȫ�ֱ����������.
    CreateBg.CreateBackground(WinSize);

    // ******************************** TEST ����ϵͳ ********************************

    PsagLow::PsagSupFilesysLoaderBin ParticleTexture("Test/ParticleImgTest.png");
    AshesParticles = new
        GraphicsEngineParticle::PsagGLEngineParticle(WinSize, DecodeRawImage.DecodeImageRawData(ParticleTexture.GetDataBinary()));
    AshesParticles->SetParticleTwisted(1.0f);

    // ******************************** TEST Physics & Graphics ��Դ ********************************

    GameActorCore::GameActorPhysicalWorld CreatePhyWorld("MyPhyWorld", 1);

    // ����Actor��ɫ����Դ.
    ActorShaderPawn = new GameActorCore::GameActorShader(ActorFragPawn, WinSize); ActorShaderPawn->CreateShaderRes();
    ActorShaderStar = new GameActorCore::GameActorShader(ActorFragStar, WinSize); ActorShaderStar->CreateShaderRes();

    PsagLow::PsagSupFilesysLoaderBin BrickTexture("Test/TEST_BOX.png");

    BrickShader = new GameActorCore::GameActorShader(GameActorScript::PsagShaderBrickPrivateFS, WinSize);
    BrickShader->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(BrickTexture.GetDataBinary()));
    BrickShader->CreateShaderRes();

    // ******************************** TEST Actor�������ư� ********************************

    TActor::ActorTypeAllotter.ActorTypeCreate("actor_pawn");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_npc");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_star");

    // ******************************** TEST PawnActor���� ********************************

    GameActorCore::GameActorActuatorDESC ConfigPawnActor;

    ConfigPawnActor.ActorPhysicsWorld   = "MyPhyWorld";
    ConfigPawnActor.ActorShaderResource = ActorShaderPawn;

    ConfigPawnActor.ControlFricMove   = 7.2f;
    ConfigPawnActor.ControlFricRotate = 2.4f;
    ConfigPawnActor.ControlFricScale  = 1.0f;

    // ʹ��Ĭ������.
    GameActorCore::GameActorHealthDESC PawnActorHealthDESC = {};
    // config hp system.
    ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

    ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);

    PawnActorCode = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_pawn"), ConfigPawnActor);

    // ******************************** TEST NPC Actor���� ********************************

    GameActorCore::GameActorActuatorDESC ConfigNpcActor;

    ConfigNpcActor.ActorPhysicsWorld = "MyPhyWorld";
    ConfigNpcActor.ActorShaderResource = ActorShaderStar;

    ConfigNpcActor.ControlFricMove   = 0.1f;
    ConfigNpcActor.ControlFricRotate = 1.0f;
    ConfigNpcActor.ControlFricScale  = 1.0f;

    ConfigNpcActor.InitialPhysics  = Vector2T<float>(7.0f, 2.0f);
    ConfigNpcActor.InitialPosition = Vector2T<float>(15.0f, 15.0f);
    ConfigNpcActor.InitialScale    = Vector2T<float>(0.2f, 0.2f);
    ConfigNpcActor.InitialSpeed    = Vector2T<float>();

    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_npc"), ConfigNpcActor);

    // ******************************** TEST ��̬��ͼ'Brick'���� ********************************

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

void TestClassStar::LogicCloseFree() {

    GameActorCore::GameActorPhysicalWorld DeletePhyWorld("MyPhyWorld", 2);

    delete ActorShaderPawn;
    delete ActorShaderStar;
    delete BrickShader;

    delete AshesParticles;
}

bool TestClassStar::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

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

    // �Ե� 10 ��������� 12 ��.
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

bool TestClassStar::LogicEventLoopGui(GameLogic::FrameworkParams& RunningState) {
    // �޸� ImGui �ؼ���ɫ.
    ImGui::PushStyleColor(ImGuiCol_WindowBg,       ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.72f, 0.72f, 0.72f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg,        ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,  ImVec4(0.58f, 0.58f, 0.58f, 0.92f));

    ImGui::Begin("PSA-Game2D STAR");
    {
        if (ImGui::GetIO().Framerate > GameTestMaxFPS)
            GameTestMaxFPS = ImGui::GetIO().Framerate;

        ImGui::Text("FPS: %.1f MaxFPS: %.2f", ImGui::GetIO().Framerate, GameTestMaxFPS);
    }
    ImGui::End();

    auto PawnActorObj = TestGameActors.FindGameActor(PawnActorCode);

    //GameDebugGuiWindow::DebugWindowGuiActors(TestGameActors.GetSourceData());
    GameDebugGuiWindow::DebugWindowGuiActor("PawnActor", PawnActorObj);

    auto MoveSpeed   = PawnActorObj->ActorMappingMoveSpeed();
    auto RotateSpeed = PawnActorObj->ActorMappingRotateSpeed();

    CameraScale = 0.7f;

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        MoveSpeed->vector_y -= 0.7f * RunningState.GameRunTimeStep;
        CameraScale = 1.8f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        MoveSpeed->vector_y += 0.7f * RunningState.GameRunTimeStep;
        CameraScale = 1.8f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        MoveSpeed->vector_x -= 0.7f * RunningState.GameRunTimeStep;
        *RotateSpeed -= 0.02f * RunningState.GameRunTimeStep;
        CameraScale = 1.0f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        MoveSpeed->vector_x += 0.7f * RunningState.GameRunTimeStep;
        *RotateSpeed += 0.02f * RunningState.GameRunTimeStep;
        CameraScale = 1.0f;
    }

    auto ToWindowCoord = PawnActorObj->ActorConvertVirCoord(RunningState.WindowResolution);
    
    if (ToWindowCoord.vector_x > RunningState.WindowResolution.vector_x * 0.4f)
        CameraPosition.vector_x -= abs(MoveSpeed->vector_x) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_x < RunningState.WindowResolution.vector_x * 0.6f)
        CameraPosition.vector_x += abs(MoveSpeed->vector_x) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_y > RunningState.WindowResolution.vector_y * 0.4f)
        CameraPosition.vector_y += abs(MoveSpeed->vector_y) * RunningState.GameRunTimeStep;

    if (ToWindowCoord.vector_y < RunningState.WindowResolution.vector_y * 0.6f)
        CameraPosition.vector_y -= abs(MoveSpeed->vector_y) * RunningState.GameRunTimeStep;
    
    RunningState.CameraParams->MatrixScale.vector_x +=
        (CameraScale - RunningState.CameraParams->MatrixScale.vector_x) * 0.0025f;
    RunningState.CameraParams->MatrixScale.vector_y = RunningState.CameraParams->MatrixScale.vector_x;
    RunningState.CameraParams->MatrixPosition = CameraPosition;

    GuiTitleNumber(Vector2T<float>(ImGui::GetIO().DisplaySize.x / 2.0f - 160.0f, 72.0f), EatStarCountTotal);
    ImGui::PopStyleColor(8);
    return true;
}