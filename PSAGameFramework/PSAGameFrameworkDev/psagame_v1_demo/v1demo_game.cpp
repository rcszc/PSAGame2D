// v1demo_game.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

bool PsaGameV1Demo::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// ******************************** 创建 全局物理系统 ********************************
	PsagActor::OperPhysicalWorld CreatePhysics("DemoPhysics", 1);

	// ******************************** 创建 Actors管理器 ********************************
	DemoShaders = new PsagActor::ShaderManager();

	GameInitActorsShader(WinSize);

	// ******************************** 创建 地图建筑资源(brick) ********************************
	// 创建统一管理器.
	DemoArchitecture = new PsagActor::BricksManager();

	PsagActor::BrickDESC BricksDESC;

	BricksDESC.BrickPhysicsWorld   = "DemoPhysics";
	BricksDESC.BrickShaderResource = DemoShaders->FindActorShader("Background");
	// background brick scale.
	BricksDESC.InitialScale = Vector2T<float>(50.0f, 50.0f);
	// close phy_collision system.
	BricksDESC.EnableCollision = false;

	DemoArchitecture->CreateGameBrick(BricksDESC);
	GameInitBoundary();

	// ******************************** 创建 Actor ********************************
	// 创建统一管理器.
	DemoActors = new PsagActor::ActorsManager();

	// 创建Actors类型绑定.
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorPawn");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorBullet");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorNpc");

	GameInitPawnActor();

	PActorHP   = new PsagManager::Tools::Timer::GameCycleTimer();
	PActorFIRE = new PsagManager::Tools::Timer::GameCycleTimer();

	// 创建玩家 输入控制,相机运动.
	PlayerPawn   = new PsagManager::Tools::Pawn::GamePlayerPawn(Vector2T<float>(128.0f, 128.0f));
	PlayerCamera = new PsagManager::Tools::Camera::GamePlayerComaeraMP(Vector2T<float>(0.0f, 0.0f), WinSize, 0.5f);

	GameInitParticleSystem(WinSize);

	for (float i = 0.0f; i < 361.0f; i += 60.0f) {
		GameCreateNPC(Vector2T<float>(sin(PSAG_M_DEGRAD(i)) * 380.0f, cos(PSAG_M_DEGRAD(i)) * 380.0f));
	}
	return true;
}

void PsaGameV1Demo::LogicCloseFree() {
	// ******************************** 销毁游戏资源 ********************************
	
	delete PlayerCamera;
	delete PlayerPawn;

	delete DemoParticlePActor1;
	delete DemoParticlePActor2;
	delete DemoParticleBullet;

	delete DemoActors;
	delete DemoShaders;
	delete DemoArchitecture;
}

bool PsaGameV1Demo::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	// ******************************** Run,Update,Rendering ********************************

	auto PawnActorOBJ = DemoActors->FindGameActor(PawnActorUnqiue);
	auto MappingWinCoord = PawnActorOBJ->ActorMappingWindowCoord();

	// actor move pawn.
	PlayerPawn->PlayerPawnRun(32.0f);
	PawnActorOBJ->ActorApplyForceMove(PlayerPawn->ControlMoveVector);

	DemoArchitecture->RunAllGameBrick();
	DemoActors->RunAllGameActor();

	float PawnActorAngle = PsagManager::Maths::CalcFuncPointsAngle(MappingWinCoord, PlayerPawn->ControlMousePosition);
	PawnActorOBJ->ActorModifyState(PawnActorOBJ->ActorGetPosition(), PawnActorAngle);

	// camera move pawn.
	PlayerCamera->PlayerCameraRun(MappingWinCoord, PawnActorOBJ->ActorGetMoveSpeed());

	// 摄像机抖动.
	RunningState.CameraParams->MatrixPosition = 
		PlayerCamera->GetCameraPosition(
			Vector2T<float>(
				sin(PSAG_M_DEGRAD(PawnActorAngle + 90.0f)) * sin(CameraShakeValue) * 22.0f, 
				cos(PSAG_M_DEGRAD(PawnActorAngle + 90.0f)) * sin(CameraShakeValue) * 22.0f
			));

	if (PlayerPawn->MouseButtonPressed_L() && PActorFIRE->CycleTimerFlagGet() && PawnActorBullet > 0.0f) {
		// 创建Actor子弹 => 减弹夹 => 重置计时器.
		GameCreateBulletPawn(PawnActorOBJ->ActorGetPosition(), PawnActorOBJ->ActorGetRotate());
		PawnActorBullet -= 1.0f;
		PActorFIRE->CycleTimerClearReset(50.0f);

		CameraShakeValue += 0.72f;
	}

	if (PlayerPawn->KeyboardPressed_R() && PawnActorBullet < 1.0f) {
		GameCreateParticlesPActorCAMM(42.0f);
		PawnActorBullet = 70.0f;
	}

	if (PlayerPawn->MouseButtonPressed_R() && PActorHP->CycleTimerFlagGet()) {
		// 回血 => 粒子 => 重置计时器.
		PawnActorOBJ->ActorModifyHealth(
			0, PawnActorOBJ->ActorGetHealth(0) + PsagManager::Tools::RAND::GenerateRandomFunc(896.0f, 768.0f)
		);
		GameCreateParticlesPActorADHP(16.0f);
		PActorHP->CycleTimerClearReset(1200.0f);
	}

	// ******************************** 全局主界面GUI ********************************

	DemoParticlePActor1->SetFxParticlesCenter(PawnActorOBJ->ActorGetPosition());
	DemoParticlePActor2->SetFxParticlesCenter(PawnActorOBJ->ActorGetPosition());

	DemoParticlePActor1->FxParticleRendering();
	DemoParticlePActor2->FxParticleRendering();
	DemoParticleBullet->FxParticleRendering();

	// delete_list => exe.
	DemoActors->UpdateManagerData();

	// 修改 ImGui 控件颜色.
	ImGui::PushStyleColor(ImGuiCol_WindowBg,      ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg,       ImVec4(0.12f, 0.12f, 0.12f, 0.72f));
	ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.0f, 1.0f, 1.0f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 1.0f, 1.0f, 0.38f));

	ImGuiWindowFlags FlagsTemp = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImVec2 HPWindowSize = ImVec2(0.7f, 0.2f) * ImGui::GetIO().DisplaySize;

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - HPWindowSize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.92f));
	ImGui::SetNextWindowSize(HPWindowSize);

	ImGui::Begin("##PActorHP", (bool*)0, FlagsTemp);
	ImGui::Text("AMM: %.0f HP: %.1f", PawnActorBullet, PawnActorOBJ->ActorGetHealth(0));
	ImGui::ProgressBar(PawnActorOBJ->ActorGetHealth(0) / PawnActorHPmax);
	ImGui::End();

	ImGui::SetNextWindowPos(ImGui::GetMousePos() - 64.0f);
	ImGui::SetNextWindowSize(ImVec2(128.0f, 128.0f) + IMGUI_ITEM_SPAC * 2.0f);

	ImGui::Begin("##MOUSE", (bool*)0, FlagsTemp);
	ImGui::Image((ImTextureID)(uintptr_t)GuiViewImage->GetTextureView(), ImVec2(128.0f, 128.0f));
	ImGui::End();

	ImGui::PopStyleColor(4);

	RunningState.CameraParams->MatrixScale = Vector2T<float>(1.32f, 1.32f);
	RunningState.PostShaderParams->GameSceneFilterAVG   = 0.28f;
	RunningState.PostShaderParams->GameSceneBloomRadius = 18;
	return true;
}