// TestDemoStar.
#include "TestDemoStar.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace TActor = GameActorCore::Type;

void TestClassStar::CreateBulletActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed) {
    GameActorCore::GameActorActuatorDESC ConfigBullet;

    ConfigBullet.ActorPhysicsWorld   = "MyPhyWorld";
    ConfigBullet.ActorShaderResource = ActorShaderNPC;

    ConfigBullet.ControlFricMove   = 0.1f;
    ConfigBullet.ControlFricRotate = 1.0f;
    ConfigBullet.ControlFricScale  = 1.0f;

    ConfigBullet.InitialPhysics  = Vector2T<float>(7.0f, 2.0f);
    ConfigBullet.InitialPosition = PosBegin;
    ConfigBullet.InitialScale    = Vector2T<float>(0.2f, 0.2f);
    ConfigBullet.InitialSpeed    = PosSpeed;
    
    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_bullet"), ConfigBullet);
}

bool TestClassStar::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
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

    // ******************************** TEST Physics & Graphics 资源 ********************************

    GameActorCore::GameActorPhysicalWorld CreatePhyWorld("MyPhyWorld", 1);

    // 创建Actor着色器资源.
    ActorShaderPawn = new GameActorCore::GameActorShader(ActorFragPawn, WinSize); ActorShaderPawn->CreateShaderRes();
    ActorShaderNPC  = new GameActorCore::GameActorShader(ActorFragNPC, WinSize);  ActorShaderNPC->CreateShaderRes();

    PsagLow::PsagSupFilesysLoaderBin BrickTexture("Test/TEST_BOX.png");

    BrickShader = new GameActorCore::GameActorShader(GameActorScript::PsagShaderBrickPrivateFS, WinSize);
    BrickShader->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(BrickTexture.GetDataBinary()));
    BrickShader->CreateShaderRes();

    // ******************************** TEST Actor类型名称绑定 ********************************

    TActor::ActorTypeAllotter.ActorTypeCreate("actor_pawn");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_npc");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_bullet");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_wall");

    // ******************************** TEST PawnActor对象 ********************************

    GameActorCore::GameActorActuatorDESC ConfigPawnActor;

    ConfigPawnActor.ActorPhysicsWorld   = "MyPhyWorld";
    ConfigPawnActor.ActorShaderResource = ActorShaderPawn;

    ConfigPawnActor.ControlFricMove   = 4.8f;
    ConfigPawnActor.ControlFricRotate = 2.4f;
    ConfigPawnActor.ControlFricScale  = 1.0f;

    // 使用默认配置.
    GameActorCore::GameActorHealthDESC PawnActorHealthDESC = {};
    // config hp system.
    ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

    ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);

    PawnActorCode = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_pawn"), ConfigPawnActor);

    // ******************************** TEST 静态地图'Brick'对象 ********************************

    GameToolsCore::Random::GenerateRandom2D RandomCreate;

    auto BrickRandomParam = Vector2T<Vector2T<float>>(Vector2T<float>(-300.0f, 300.0f), Vector2T<float>(-300.0f, 300.0f));

    RandomCreate.RandomSeedMode(GameToolsCore::Random::TimeSeedMicroseconds);
    RandomCreate.CreateRandomDataset(40, BrickRandomParam, 50.0f);
    RandomCreate.DatasetCropCircle(Vector2T<float>(0.0f, 0.0f), 10.0f);

    GameBrickCore::GameBrickActuatorDESC BricksDESC;

    BricksDESC.BrickPhysicsWorld   = "MyPhyWorld";
    BricksDESC.BrickShaderResource = BrickShader;

    GameToolsCore::Random::GenerateRandom1D RandomSizeCreate;
    RandomSizeCreate.RandomSeedMode(GameToolsCore::Random::TimeSeedMicroseconds);

    for (const auto& BrickPos : RandomCreate.RandomCoordGroup) {
        // set brick scale_size.
        BricksDESC.InitialScale = 
            Vector2T<float>(RandomSizeCreate.CreateRandomValue(1.0f, 1.6f), RandomSizeCreate.CreateRandomValue(1.0f, 1.6f));
        // set brick position.
        BricksDESC.InitialPosition = BrickPos;
        TestGameBricks.CreateGameActor(BricksDESC);
    }

    // ******************************** TEST NPC Actor 对象 ********************************

    auto NpcRandomParam = Vector2T<Vector2T<float>>(Vector2T<float>(-200.0f, 200.0f), Vector2T<float>(-200.0f, 200.0f));

    RandomCreate.RandomCoordGroup.clear();
    RandomCreate.CreateRandomDataset(40, NpcRandomParam, 10.0f);
    RandomCreate.DatasetCropCircle(Vector2T<float>(0.0f, 0.0f), 10.0f);

    for (const auto& NpcPos : RandomCreate.RandomCoordGroup)
        CreateBulletActor(NpcPos, Vector2T<float>());
    
    return true;
}

void TestClassStar::LogicCloseFree() {

    GameActorCore::GameActorPhysicalWorld DeletePhyWorld("MyPhyWorld", 2);

    delete ActorShaderPawn;
    delete ActorShaderNPC;
    delete BrickShader;

    delete AshesParticles;
}

bool TestClassStar::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

    AshesParticles->UpdateParticleData();
    AshesParticles->RenderParticleFX();

    for (auto& Bullet : *TestGameActors.GetSourceData()) {
        if (Bullet.second->ActorGetCollision().ActorUniqueCode != NULL &&
            Bullet.second->ActorGetPrivate().ActorTypeCode == TActor::ActorTypeAllotter.ActorTypeIs("actor_bullet")
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
                Vector3T<float>(Bullet.second->ActorGetPosition().vector_x, Bullet.second->ActorGetPosition().vector_y, 0.0f)
            );
            AshesParticles->ParticleCreate(&CreatePartc);
            TestGameActors.DeleteGameActor(Bullet.second->ActorGetPrivate().ActorUniqueCode);

            ++EatNpcCount;
        }
    }

    // 吃掉 10 个随机生成 12 个.
    if (EatNpcCount > 10) {
        GameToolsCore::Random::GenerateRandom2D RandomCreate;
        RandomCreate.RandomSeedMode(GameToolsCore::Random::TimeSeedMicroseconds);

        auto RandomParam = Vector2T<Vector2T<float>>(Vector2T<float>(-200.0f, 200.0f), Vector2T<float>(-200.0f, 200.0f));

        RandomCreate.RandomCoordGroup.clear();
        RandomCreate.CreateRandomDataset(12, RandomParam, 10.0f);
        RandomCreate.DatasetCropCircle(Vector2T<float>(0.0f, 0.0f), 10.0f);

        for (const auto& NpcPos : RandomCreate.RandomCoordGroup)
            CreateBulletActor(NpcPos, Vector2T<float>());
        EatNpcCount = NULL;
    }

    TestGameBricks.RunAllGameActor();

    TestGameActors.RunAllGameActor();
    TestGameActors.UpdateManagerData();

    RunningState.BackShaderParams->BackgroundVisibility = Visibility;
    RunningState.PostShaderParams->GameSceneBloomRadius = (uint32_t)BloomRadius;
    return true;
}

bool TestClassStar::LogicEventLoopGui(GameLogic::FrameworkParams& RunningState) {
    // 修改 ImGui 控件颜色.
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
        ImGui::Text("Particles: %u", AshesParticles->GetParticleState().DarwParticlesNumber);

        ImGui::ColorEdit4 ("COLOR", RunningState.BackShaderParams->BackgroundColor.data());
        ImGui::SliderFloat("BLOOM", &BloomRadius, 1.0f, 32.0f);
        ImGui::SliderFloat("VISIB", &Visibility, 0.1f, 2.0f);

        ImGui::SliderFloat3("Filter RGB", RunningState.PostShaderParams->GameSceneFilterCOL.data(), 0.0f, 2.0f);
        ImGui::SliderFloat ("Filter AVG", &RunningState.PostShaderParams->GameSceneFilterAVG, 0.0f, 2.0f);
    }
    ImGui::End();

    auto PawnActorObj = TestGameActors.FindGameActor(PawnActorCode);

    GameDebugGuiWindow::DebugWindowGuiActors(TestGameActors.GetSourceData());
    GameDebugGuiWindow::DebugWindowGuiActor("PawnActor", PawnActorObj);

    auto MoveSpeed   = PawnActorObj->ActorMappingMoveSpeed();
    auto RotateSpeed = PawnActorObj->ActorMappingRotateSpeed();

    CameraScale = 0.84f;

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        MoveSpeed->vector_y -= 0.5f * RunningState.GameRunTimeStep;
        CameraScale = 1.72f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        MoveSpeed->vector_y += 0.5f * RunningState.GameRunTimeStep;
        CameraScale = 1.72f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        MoveSpeed->vector_x -= 0.5f * RunningState.GameRunTimeStep;
        *RotateSpeed -= 0.01f * RunningState.GameRunTimeStep;
        CameraScale = 1.25f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        MoveSpeed->vector_x += 0.5f * RunningState.GameRunTimeStep;
        *RotateSpeed += 0.01f * RunningState.GameRunTimeStep;
        CameraScale = 1.25f;
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
        (CameraScale - RunningState.CameraParams->MatrixScale.vector_x) * 0.001f;
    RunningState.CameraParams->MatrixScale.vector_y = RunningState.CameraParams->MatrixScale.vector_x;
    RunningState.CameraParams->MatrixPosition = CameraPosition;

    ImGui::PopStyleColor(8);
    return true;
}