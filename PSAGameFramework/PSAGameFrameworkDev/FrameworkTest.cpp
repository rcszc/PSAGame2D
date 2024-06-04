// FrameworkTest.
#include "FrameworkTest.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace TActor = GameActorCore::Type;

void DevTestClass::CreateBulletActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed) {
    GameActorCore::GameActorActuatorDESC ConfigBullet;

    ConfigBullet.ActorPhysicsWorld     = "MyPhyWorld";
    ConfigBullet.ActorShaderResource = ActorShaderBullet;

    ConfigBullet.ControlFricMove   = 0.1f;
    ConfigBullet.ControlFricRotate = 1.0f;
    ConfigBullet.ControlFricScale  = 1.0f;

    ConfigBullet.InitialPhysics  = Vector2T<float>(7.0f, 2.0f);
    ConfigBullet.InitialPosition = PosBegin;
    ConfigBullet.InitialScale    = Vector2T<float>(0.2f, 0.2f);
    ConfigBullet.InitialSpeed    = PosSpeed;
    
    size_t AC = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_bullet"), ConfigBullet);
}

void DevTestClass::CreateNpcActor(float max_hp) {
    // config actor.
    GameActorCore::GameActorActuatorDESC ConfigActors;

    ConfigActors.ActorPhysicsWorld     = "MyPhyWorld";
    ConfigActors.ActorShaderResource = ActorShaderNPC;

    ConfigActors.ControlFricMove   = 2.0f;
    ConfigActors.ControlFricRotate = 1.0f;
    ConfigActors.ControlFricScale  = 1.0f;

    GameActorCore::GameActorHealthDESC NPCActorHealthDESC = {};

    ConfigActors.InitialPhysics  = Vector2T<float>(7.2f, 3.2f);
    ConfigActors.InitialPosition = Vector2T<float>(-70.0f, 0.0f);

    NPCActorHealthDESC.InitialHealthState[0] = max_hp;
    NPCActorHealthDESC.InitialHealthSpeed[0] = 1.0f;
    NPCActorHealthDESC.HealthHandlerFunc = [this](GameActorCore::HealthFuncParams params) { HealthHandlerFuncNPC(params); };
    // HP DESC => ACTOR DESC.
    ConfigActors.ActorHealthSystem = NPCActorHealthDESC;

    // 初始化位置创建.
    ConfigActors.InitialPosition = Vector2T<float>(32.0f, 10.0f);
    NpcActorCode[0] = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_npc"), ConfigActors);

    ConfigActors.InitialPosition = Vector2T<float>(-20.0f, 10.0f);
    NpcActorCode[1] = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_npc"), ConfigActors);

    ConfigActors.InitialPosition = Vector2T<float>(10.0f, 32.0f);
    NpcActorCode[2] = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_npc"), ConfigActors);
}

void DevTestClass::HealthHandlerFuncNPC(const GameActorCore::HealthFuncParams& params) {
    // NPC 与 Bullet 发生碰撞.
    auto ActorObj = TestGameActors.FindGameActor(params.ActorCollision.ActorUniqueCode);
    if (ActorObj != nullptr) {
        if (ActorObj->ActorGetPrivate().ActorTypeCode == TActor::ActorTypeAllotter.ActorTypeIs("actor_bullet"))
            params.ActorHealthStates[0] -= 5.0f;
    }

    // NPC 血量为0发射粒子,销毁.
    if (params.ActorHealthStates[0] <= 0.0f) {
        auto NPCActor = TestGameActors.FindGameActor(params.ThisActorUniqueCode);

        GraphicsEngineParticle::ParticleGenerator CreatePartc;
        CreatePartc.ConfigCreateMode(GraphicsEngineParticle::PrtcPoints);
        CreatePartc.ConfigCreateNumber(72);
        CreatePartc.ConfigLifeDispersion(Vector2T<float>(1024.0f, 1560.0f));
        CreatePartc.ConfigSizeDispersion(Vector2T<float>(1.28f, 2.32f));

        CreatePartc.ConfigRandomColorSystem(
            Vector2T<float>(0.42f, 1.28f), Vector2T<float>(0.42f, 1.28f), Vector2T<float>(0.42f, 1.28f), GraphicsEngineParticle::ChannelsRB
        );
        CreatePartc.ConfigRandomDispersion(
            Vector2T<float>(-1.5f, 1.5f),
            Vector2T<float>(0.0f, 0.0f),
            Vector3T<float>(NPCActor->ActorGetPosition().vector_x, NPCActor->ActorGetPosition().vector_y, 0.0f)
        );
        AshesParticles->ParticleCreate(&CreatePartc);
        TestGameActors.DeleteGameActor(params.ThisActorUniqueCode);
    }
}

bool DevTestClass::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
    /*
    PsagLow::PsagSupGraphicsOper::PsagGraphicsImageRawDat LoadRawImage;

    PsagLow::PsagSupFilesysLoaderBin TestBinLoad1("Test/TEST_BACK1.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad2("Test/TEST_BACK2.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad3("Test/TEST_BACK3.png");
    PsagLow::PsagSupFilesysLoaderBin TestBinLoad4("Test/TEST_BACK4.png");
    
    PsagGameSys::SysBackground CreateBg;

    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad4.GetDataBinary()));
    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad3.GetDataBinary()));
    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad2.GetDataBinary()));
    CreateBg.LayerDataPush(LoadRawImage.DecodeImageRawData(TestBinLoad1.GetDataBinary()));

    // 创建全局背景处理对象.
    CreateBg.CreateBackground(WinSize);
    */

    PsagLow::PsagSupGraphicsOper::PsagGraphicsImageRawDat DecRawImage;
    PsagLow::PsagSupFilesysLoaderBin LoadParticle("Test/ParticleImgTest.png");

    // ******************************** TEST 序列帧贴图 ********************************

    PsagLow::PsagSupFilesysLoaderBin LoadSequence("Test/fx_fire_test.png");
    GraphicsEnginePVFX::SequencePlayer SequenceParams = {};

    SequenceParams.UaxisFrameNumber = 6.0f;
    SequenceParams.VaxisFrameNumber = 6.0f;
    SequenceParams.PlayerSpeedScale = 2.0f;

    TestCaptureView = new GraphicsEnginePVFX::PsagGLEngineFxCaptureView(WinSize);
    TestSequence = new GraphicsEnginePVFX::PsagGLEngineFxSequence(
        DecRawImage.DecodeImageRawData(LoadSequence.GetDataBinary()), SequenceParams
    );

    PsagLow::PsagSupFilesysLoaderBin SoundBin("Test/TestSoundMC.ogg");
    PsagLow::PsagSupSoundData TestSoundDat(PsagSupSoundRawCVT(SoundBin.GetDataBinary()));

    // ******************************** TEST ACTORS ********************************

    GameActorCore::GameActorPhysicalWorld CreatePhyWorld("MyPhyWorld", 1);

    TActor::ActorTypeAllotter.ActorTypeCreate("actor_pawn");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_npc");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_bullet");
    TActor::ActorTypeAllotter.ActorTypeCreate("actor_wall");

    // 创建Actor着色器资源.
    ActorShaderPawn   = new GameActorCore::GameActorShader(ActorFragPawn, WinSize);   ActorShaderPawn->CreateShaderRes();
    ActorShaderBullet = new GameActorCore::GameActorShader(ActorFragBullet, WinSize); ActorShaderBullet->CreateShaderRes();
    ActorShaderNPC    = new GameActorCore::GameActorShader(ActorFragNPC, WinSize);    ActorShaderNPC->CreateShaderRes();
    ActorShaderWall   = new GameActorCore::GameActorShader(ActorFragWall, WinSize);   ActorShaderWall->CreateShaderRes();

    GameBrickCore::GameBrickActuatorDESC ConfigBrick = {};

    ConfigBrick.BrickShaderResource = ActorShaderWall;
    ConfigBrick.BrickPhysicsWorld = "MyPhyWorld";

    TestBrick = new GameBrickCore::GameBrickActuator(ConfigBrick);

    // config actor.
    GameActorCore::GameActorActuatorDESC ConfigActors;

    ConfigActors.ActorPhysicsWorld     = "MyPhyWorld";
    ConfigActors.ActorShaderResource = ActorShaderPawn;

    ConfigActors.ControlFricMove   = 2.0f;
    ConfigActors.ControlFricRotate = 1.0f;
    ConfigActors.ControlFricScale  = 1.0f;

    GameActorCore::GameActorHealthDESC NPCActorHealthDESC = {};
    // config hp system.
    ConfigActors.ActorHealthSystem = NPCActorHealthDESC;

    ConfigActors.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigActors.InitialPosition = Vector2T<float>(-70.0f, 0.0f);

    PawnActorCode = TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_pawn"), ConfigActors);

    CreateNpcActor(100.0f);

    // 创建测试墙体.

    GameActorCore::GameActorActuatorDESC ConfigWalls;

    ConfigWalls.ActorPhysicsWorld   = "MyPhyWorld";
    ConfigWalls.ActorShaderResource = ActorShaderWall;
    ConfigWalls.ActorPhysicalMode   = GameActorCore::PhyFixedActor;

    ConfigWalls.ControlFricMove   = 1.0f;
    ConfigWalls.ControlFricRotate = 1.0f;
    ConfigWalls.ControlFricScale  = 1.0f;
    ConfigWalls.InitialPhysics = Vector2T<float>(0.0f, 0.1f);

    ConfigWalls.InitialPosition = Vector2T<float>(0.0f, 95.0f);
    ConfigWalls.InitialScale    = Vector2T<float>(16.0f, 0.2f);
    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_wall"), ConfigWalls);

    ConfigWalls.InitialPosition = Vector2T<float>(0.0f, -95.0f);
    ConfigWalls.InitialScale    = Vector2T<float>(16.0f, 0.2f);
    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_wall"), ConfigWalls);

    ConfigWalls.InitialPosition = Vector2T<float>(160.0f, 0.0f);
    ConfigWalls.InitialScale    = Vector2T<float>(0.2f, 8.5f);
    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_wall"), ConfigWalls);

    ConfigWalls.InitialPosition = Vector2T<float>(-160.0f, 0.0f);
    ConfigWalls.InitialScale    = Vector2T<float>(0.2f, 8.5f);
    TestGameActors.CreateGameActor(TActor::ActorTypeAllotter.ActorTypeIs("actor_wall"), ConfigWalls);

	return true;
}

void DevTestClass::LogicCloseFree() {

    GameActorCore::GameActorPhysicalWorld CreatePhyWorld("MyPhyWorld", 2);

    delete AshesParticles;
    delete TestCaptureView;
}

bool DevTestClass::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

    AshesParticles->UpdateParticleData();
    AshesParticles->RenderParticleFX();

    for (auto& Bullet : *TestGameActors.GetSourceData()) {
        if (Bullet.second->ActorGetLifeTime() > 2.5f && 
            Bullet.second->ActorGetPrivate().ActorTypeCode == TActor::ActorTypeAllotter.ActorTypeIs("actor_bullet") ||
            Bullet.second->ActorGetCollision().ActorUniqueCode != NULL && 
            Bullet.second->ActorGetPrivate().ActorTypeCode == TActor::ActorTypeAllotter.ActorTypeIs("actor_bullet")
        ) {
            GraphicsEngineParticle::ParticleGenerator CreatePartc;
            CreatePartc.ConfigCreateMode(GraphicsEngineParticle::PrtcPoints);
            CreatePartc.ConfigCreateNumber(16);
            CreatePartc.ConfigLifeDispersion(Vector2T<float>(512.0f, 1024.0f));
            CreatePartc.ConfigSizeDispersion(Vector2T<float>(0.42f, 1.28f));

            CreatePartc.ConfigRandomColorSystem(
                Vector2T<float>(0.0f, 0.0f), Vector2T<float>(0.42f, 1.28f), Vector2T<float>(0.42f, 1.28f), GraphicsEngineParticle::ChannelsRGB
            );
            CreatePartc.ConfigRandomDispersion(
                Vector2T<float>(-1.0f, 1.0f),
                Vector2T<float>(0.0f, 0.0f),
                Vector3T<float>(Bullet.second->ActorGetPosition().vector_x, Bullet.second->ActorGetPosition().vector_y, 0.0f)
            );
            AshesParticles->ParticleCreate(&CreatePartc);
            TestGameActors.DeleteGameActor(Bullet.second->ActorGetPrivate().ActorUniqueCode);
        }
    }

    TestGameActors.RunAllGameActor();
    TestGameActors.UpdateManagerData();

    TestBrick->BrickRendering();

    // background blend color_inter calc.
    ColorAnimInter += ImVec4(BgBlendColorSet - ColorAnimInter) * 0.02f;

    // background visibility inter calc.
    RunningState.BackShaderParams->BackgroundVisibility +=
        (BgVisibilitySet - RunningState.BackShaderParams->BackgroundVisibility) * 0.02f;

    RunningState.BackShaderParams->BackgroundStrength.vector_x +=
        (BgBlendStrengthSet - RunningState.BackShaderParams->BackgroundStrength.vector_x) * 0.02f;

    TestCaptureView->CaptureContextBind();
    TestSequence->DrawFxSequence(TestColor);
    TestCaptureView->CaptureContextUnBind();

    ++CountTimer;
	return true;
}

float FmtValue(float value) {
    return value > 0.0f ? 1.0f : -1.0f;
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
        ImGui::Text("Particles: %u", AshesParticles->GetParticleState().DarwParticlesNumber);
        
        ImGui::ColorEdit4("COLOR", &BgBlendColorSet.x);
        ImGui::SliderFloat("BLOOM", &BloomRadius, 1.0f, 32.0f);
        ImGui::SliderFloat("VISIB", &BgVisibilitySet, 0.1f, 2.0f);
        ImGui::SliderFloat("BLEND", &BgBlendStrengthSet, 0.1f, 2.0f);

        ImGui::SliderFloat3("Filter RGB", RunningState.PostShaderParams->GameSceneFilterCOL.data(), 0.0f, 2.0f);
        ImGui::SliderFloat("Filter AVG", &RunningState.PostShaderParams->GameSceneFilterAVG, 0.0f, 2.0f);

        ImGui::SliderFloat2("CAMERA", RunningState.PostShaderParams->GameCameraTrans.CameraOffset.data(), -100.0f, 100.0f);

        ImGui::ProgressBar(RunningState.BackShaderParams->BackgroundVisibility / 2.0f);
        ImGui::ProgressBar(RunningState.BackShaderParams->BackgroundStrength.vector_x / 2.0f);
        ImGui::ProgressBar(BloomRadius / 32.0f);
    }
    ImGui::End();

    auto PawnActorObj = TestGameActors.FindGameActor(PawnActorCode);

    GameDebugGuiWindow::DebugWindowGuiActors(TestGameActors.GetSourceData());
    GameDebugGuiWindow::DebugWindowGuiActor("PawnActor", PawnActorObj);

    if (ImGui::IsKeyDown(ImGuiKey_W))
        PawnActorObj->ActorMappingMoveSpeed()->vector_y -= 0.2f * RunningState.GameRunTimeStep;
    if (ImGui::IsKeyDown(ImGuiKey_S))
        PawnActorObj->ActorMappingMoveSpeed()->vector_y += 0.2f * RunningState.GameRunTimeStep;

    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        PawnActorObj->ActorMappingMoveSpeed()->vector_x -= 0.2f * RunningState.GameRunTimeStep;
        *PawnActorObj->ActorMappingRotateSpeed() -= 0.0032f * RunningState.GameRunTimeStep;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        PawnActorObj->ActorMappingMoveSpeed()->vector_x += 0.2f * RunningState.GameRunTimeStep;
        *PawnActorObj->ActorMappingRotateSpeed() += 0.0032f * RunningState.GameRunTimeStep;
    }
    
    bool NpcFlag = false;
    for (size_t i = 0; i < 3; ++i) {
        auto NpcActorObj = TestGameActors.FindGameActor(NpcActorCode[i]);
        if (NpcActorObj != nullptr) {
            Vector2T<float> NPCPosition = NpcActorObj->ActorConvertVirCoord(RunningState.WindowResolution);
            ImGui::SetNextWindowPos(ImVec2(NPCPosition.vector_x, NPCPosition.vector_y));
            ImGui::SetNextWindowSize(ImVec2(200.0f, 42.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::Begin(
                string("NPC" + to_string(i)).c_str(), 
                (bool*)0, 
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            );
            ImGui::ProgressBar(NpcActorObj->ActorGetHealth(0) / 100.0f);
            ImGui::End();
            ImGui::PopStyleColor(2);
            NpcFlag |= true;
        }
    }
    if (!NpcFlag) CreateNpcActor(100.0f);

    // mouse_fire.
    Vector2T<float> ConvertCoord = PawnActorObj->ActorConvertVirCoord(RunningState.WindowResolution);
    // 鼠标左键 & 不悬停任何UI控件.
    if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        Vector2T<float> VectorSpeed(ImGui::GetMousePos().x - ConvertCoord.vector_x, ImGui::GetMousePos().y - ConvertCoord.vector_y);

        Vector2T<float> ActorSpeed(VectorSpeed.vector_x * 2.0f * RunningState.GameRunTimeStep, VectorSpeed.vector_y * 2.0f * RunningState.GameRunTimeStep);
        Vector2T<float> ActorPosition(
            PawnActorObj->ActorGetPosition().vector_x + 12.0f * FmtValue(VectorSpeed.vector_x),
            PawnActorObj->ActorGetPosition().vector_y + 12.0f * FmtValue(VectorSpeed.vector_y)
        );
        CreateBulletActor(ActorPosition, ActorSpeed);
    }

    ImGui::Begin("PSA-Game2D VFX - TEST");
    {
        ImGui::Image(IMTOOL_CVT_IMPTR(TestCaptureView->GetCaptureTexView()), ImVec2(512.0f, 512.0f));
        ImGui::ColorEdit4("COLOR", TestColor.data());
    }
    ImGui::End();

    ImGui::PopStyleColor(8);

    RunningState.PostShaderParams->GameSceneBloomRadius = (uint32_t)BloomRadius;
    RunningState.BackShaderParams->BackgroundStrength.vector_y = RunningState.BackShaderParams->BackgroundVisibility + 0.32f;
    
    RunningState.BackShaderParams->BackgroundColor = 
        Vector4T<float>(ColorAnimInter.x, ColorAnimInter.y, ColorAnimInter.z, ColorAnimInter.w);
        
	return true;
}