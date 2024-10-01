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
	PActorCAMM = new PsagManager::Tools::Timer::GameCycleTimer();

	// 创建玩家 输入控制,相机运动.
	PlayerPawn   = new PsagManager::Tools::Pawn::GamePlayerPawn(Vector2T<float>(128.0f, 128.0f));
	PlayerCamera = new PsagManager::Tools::Camera::GamePlayerComaeraMP(Vector2T<float>(0.0f, 0.0f), WinSize, 0.5f);

	GameInitParticleSystem(WinSize);

	for (float i = 0.0f; i < 360.0f; i += 60.0f) {
		GameCreateNPC(Vector2T<float>(sin(PSAG_M_DEGRAD(i)) * 380.0f, cos(PSAG_M_DEGRAD(i)) * 380.0f));
	}

	FINAL = new PsagManager::SceneFinal::GameFinalProcessing();

	PsagManager::FINAL_PARAMS PresetParams0 = {};

	PresetParams0.GameSceneFilterAVG   = 0.325f;
	PresetParams0.GameSceneBloomRadius = 22;
	PresetParams0.GameSceneBloomBlend  = Vector2T<float>(1.1128f, 1.1032f);

	PresetParams0.GameSceneOutContrast = 1.2232f;
	PresetParams0.GameSceneOutColor    = Vector3T<float>(1.28f, 1.14f, 1.0f);
	PresetParams0.GameSceneOutVignette = Vector2T<float>(0.685f, 0.204f);

	PresetParams0.LightIntensity  = 0.142f;
	PresetParams0.LightColor      = Vector3T<float>(1.0f, 0.92f, 0.72f);
	PresetParams0.LightSampleStep = 240;

	FINAL->PushPresetParams("Movie-Mode", PresetParams0);

	PresetParams0 = PsagManager::FINAL_PARAMS();

	PresetParams0.GameSceneFilterAVG   = 0.292f;
	PresetParams0.GameSceneBloomRadius = 22;
	PresetParams0.GameSceneBloomBlend  = Vector2T<float>(1.0f, 1.25f);

	PresetParams0.GameSceneOutContrast = 1.0;
	PresetParams0.GameSceneOutColor    = Vector3T<float>(1.1f, 1.1f, 1.1f);

	PresetParams0.LightIntensity  = 0.158f;
	PresetParams0.LightColor      = Vector3T<float>(1.0f, 0.72f, 0.912f);
	PresetParams0.LightSampleStep = 200;

	FINAL->PushPresetParams("Dream-Mode", PresetParams0);

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

// ImGui 绘制大标题字体.
void DrawTitleFloatValue(const char* name, float value, const ImVec2& pos, float width, streamsize num = 2) {
	ImVec2 WindowSize(width, 92.0f);
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(WindowSize);

	ImGuiWindowFlags FlagsTemp = 
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::Begin(name, (bool*)NULL, FlagsTemp);
	ImGui::SetWindowFontScale(3.2f);

	ostringstream OstreamString;
	OstreamString << fixed << setprecision(num) << value;
	float TitleLength = ImGui::CalcTextSize(OstreamString.str().c_str()).x;

	ImGui::SetCursorPosX(WindowSize.x * 0.5f - TitleLength * 0.5f);
	ImGui::Text("%s", OstreamString.str().c_str());
	ImGui::End();
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
	//PlayerCamera->PlayerCameraRunFixed(MappingWinCoord, PawnActorOBJ->ActorGetMoveSpeed());
	   
	// 摄像机抖动.
	RunningState.CameraParams->MatrixPosition = Vector2T<float>(
		-PawnActorOBJ->ActorGetPosition().vector_x * 10.0f,
		PawnActorOBJ->ActorGetPosition().vector_y * 10.0f
	);

	// 鼠标左键, 循环计时器, 剩余子弹,  非换弹时间.
	if (PlayerPawn->MouseButtonPressed_L() && PActorFIRE->CycleTimerFlagGet() && 
		PawnActorBullet > 0.0f && !PawnActorChangeAMM
	) {
		// 创建Actor子弹 => 减弹夹 => 重置计时器.
		GameCreateBulletPawn(PawnActorOBJ->ActorGetPosition(), PawnActorOBJ->ActorGetRotate());
		PawnActorBullet -= 1.0f;
		PActorFIRE->CycleTimerClearReset(50.0f);
	}

	if (PawnActorChangeAMM && PActorCAMM->CycleTimerFlagGet()) {
		GameCreateParticlesPActorCAMM(8.0f);
		PawnActorBullet += 4.0f;
		PActorCAMM->CycleTimerClearReset(200.0f);
		// 换弹完成.
		if (PawnActorBullet >= 128.0f) PawnActorChangeAMM = false;
	}

	if (PlayerPawn->KeyboardPressed_R() && PawnActorBullet < 1.0f)
		PawnActorChangeAMM = true;

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
	ImGui::PushStyleColor(ImGuiCol_FrameBg,       ImVec4(0.16f, 0.16f, 0.16f, 0.58f));
	ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.58f, 0.58f, 0.58f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.32f, 0.32f, 0.32f, 0.32f));

	ImGuiWindowFlags FlagsTemp = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImVec2 HPWindowSize = ImVec2(0.5f, 0.2f) * ImGui::GetIO().DisplaySize;

	DrawTitleFloatValue("##VALUE", PawnActorBullet, ImVec2(ImGui::GetIO().DisplaySize.x - 220.0f, 32.0f), 200.0f, 0);

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - HPWindowSize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.9f));
	ImGui::SetNextWindowSize(HPWindowSize);

	ImGui::Begin("##PActorHP", (bool*)0, FlagsTemp);
	ImGui::SetWindowFontScale(1.2f);
	ImGui::Text("AMM: %.0f HP: %.1f", PawnActorBullet, PawnActorOBJ->ActorGetHealth(0));
	ImGui::ProgressBar(PawnActorOBJ->ActorGetHealth(0) / PawnActorHPmax);
	ImGui::End();

	//ImGui::SetNextWindowPos(ImGui::GetMousePos() - 64.0f);
	//ImGui::SetNextWindowSize(ImVec2(128.0f, 128.0f) + IMGUI_ITEM_SPAC * 2.0f);

	//ImGui::Begin("##MOUSE", (bool*)0, FlagsTemp);
	//ImGui::Image((ImTextureID)(uintptr_t)GuiViewImage->GetTextureView(), ImVec2(128.0f, 128.0f));
	//ImGui::End();

	ImGui::PopStyleColor(4);

	RunningState.CameraParams->MatrixScale = Vector2T<float>(1.32f, 1.32f);

	FINAL->GetFinalParamsPonter(RunningState.ShaderParamsFinal);

	FINAL->RenderDebugParamsPanel("GAME FINAL FX");
	FINAL->RunFinalProcessing(RunningState.GameRunTimeSTEP);
	return true;
}