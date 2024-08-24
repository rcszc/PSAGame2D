// v1demo_game.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

bool PsaGameV1Demo::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// ******************************** ���� ȫ������ϵͳ ********************************
	PsagActor::OperPhysicalWorld CreatePhysics("DemoPhysics", 1);

	// ******************************** ���� ��ɫ����Դ(render) ********************************
	// ͼƬ������.
	PsagManager::SyncLoader::SyncDecodeImage DecodeRawImage;

	PsagManager::SyncLoader::SyncBinFileLoad BackgroundImage("demo_v1_material/psag_v1d_world_tex.png");
	PsagManager::SyncLoader::SyncBinFileLoad BoundaryImageH ("demo_v1_material/psag_v1d_world_Hwall.png");
	PsagManager::SyncLoader::SyncBinFileLoad BoundaryImageV ("demo_v1_material/psag_v1d_world_Vwall.png");

	// ����ǽ��ɫ��.
	PsagActor::ActorShader* DemoRenderBackground = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), WinSize);
	DemoRenderBackground->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(BackgroundImage.GetDataBinary()));
	
	// �߽�ǽ��ɫ��, ����&����.
	PsagActor::ActorShader* DemoRenderBoundaryH = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), WinSize);
	DemoRenderBoundaryH->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(BoundaryImageH.GetDataBinary()));
	
	PsagActor::ActorShader* DemoRenderBoundaryV = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), WinSize);
	DemoRenderBoundaryV->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(BoundaryImageV.GetDataBinary()));
	
	// PawnActor��ɫ��.
	PsagActor::ActorShader* DemoRenderPawnActor = new PsagActor::ActorShader(PawnActorFrag, WinSize);
	
	// �ӵ�Actors��ɫ��.
	PsagActor::ActorShader* DemoRenderBulletActor = new PsagActor::ActorShader(BulletActorFrag, WinSize);
	
	DemoShaders = new PsagActor::ShaderManager();

	DemoShaders->CreateActorShader("Background",  DemoRenderBackground);
	DemoShaders->CreateActorShader("BoundaryH",   DemoRenderBoundaryH);
	DemoShaders->CreateActorShader("BoundaryV",   DemoRenderBoundaryV);
	DemoShaders->CreateActorShader("PawnActor",   DemoRenderPawnActor);
	DemoShaders->CreateActorShader("BulletActor", DemoRenderBulletActor);

	// ******************************** ���� ��ͼ������Դ(brick) ********************************
	// ����ͳһ������.
	DemoArchitecture = new PsagActor::BricksManager();

	PsagActor::BrickDESC BricksDESC;

	BricksDESC.BrickPhysicsWorld   = "DemoPhysics";
	BricksDESC.BrickShaderResource = DemoRenderBackground;
	// background brick scale.
	BricksDESC.InitialScale = Vector2T<float>(50.0f, 50.0f);
	// close phy_collision system.
	BricksDESC.EnableCollision = false;

	DemoArchitecture->CreateGameBrick(BricksDESC);
	GameInitBoundary();

	// ******************************** ���� Actor ********************************
	// ����ͳһ������.
	DemoActors = new PsagActor::ActorsManager();

	// ����Actors���Ͱ�.
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorPawn");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorBullet");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorNpc");

	GameInitPawnActor();

	PActorHP   = new PsagManager::Tools::Timer::GameCycleTimer();
	PActorFIRE = new PsagManager::Tools::Timer::GameCycleTimer();

	// ������� �������,����˶�.
	PlayerPawn   = new PsagManager::Tools::Pawn::GamePlayerPawn(Vector2T<float>(128.0f, 128.0f));
	PlayerCamera = new PsagManager::Tools::Camera::GamePlayerComaeraMP(Vector2T<float>(0.25f, 0.2f), WinSize, 0.5f);

	GameInitParticleSystem(WinSize);

	FPS_TEST = new PsagActor::DebugTools::DebugWindowGuiFPS("DemoV1-FPS", 2000.0f);
	return true;
}

void PsaGameV1Demo::LogicCloseFree() {
	// ******************************** ������Ϸ��Դ ********************************
	delete FPS_TEST;

	delete PlayerCamera;
	delete PlayerPawn;

	delete DemoShaders;

	delete DemoActors;
	delete DemoArchitecture;
}

bool PsaGameV1Demo::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

	DemoArchitecture->RunAllGameBrick();
	DemoActors->RunAllGameActor();

	auto PawnActorOBJ = DemoActors->FindGameActor(PawnActorUnqiue);
	auto MappingWinCoord = PawnActorOBJ->ActorConvertVirCoord(RunningState.WindowResolution);

	// actor move pawn.
	PlayerPawn->PlayerPawnRun(32.0f);
	PawnActorOBJ->ActorApplyForceMove(PlayerPawn->ControlMoveVector);

	PawnActorAngle = PlayerPawn->ControlMousePosition.vector_x - 960.0f;
	// actor_position, mouse_position => angle.
	PawnActorOBJ->ActorApplyForceRotate((PawnActorOBJ->ActorGetRotate() - PawnActorAngle * 0.5f) * 0.72f);

	// camera move pawn.
	PlayerCamera->PlayerCameraRun(MappingWinCoord, PawnActorOBJ->ActorGetMoveSpeed());
	RunningState.CameraParams->MatrixPosition = PlayerCamera->GetCameraPosition();

	if (PlayerPawn->MouseButtonPressed_L() && PActorFIRE->CycleTimerFlagGet() && PawnActorBullet > 0.0f) {
		// ��Ѫ => ����Actor�ӵ� => ������.
		GameCreateBullet(PawnActorOBJ->ActorGetPosition(), PawnActorOBJ->ActorGetRotate());
		PawnActorBullet -= 1.0f;
		PActorFIRE->CycleTimerClearReset(50.0f);
	}

	if (PlayerPawn->KeyboardPressed_R()) {
		PawnActorBullet = 320.0f;
	}

	if (PlayerPawn->MouseButtonPressed_R() && PActorHP->CycleTimerFlagGet()) {
		// ��Ѫ => ���� => ���ü�ʱ��.
		GameCreateParticlesPActor(16.0f);
		PActorHP->CycleTimerClearReset(1200.0f);
	}

	// delete_cmd => exe.
	DemoActors->UpdateManagerData();

	DemoParticlePActor->SetFxParticlesCenter(PawnActorOBJ->ActorGetPosition());
	DemoParticlePActor->FxParticleRendering();

	RunningState.CameraParams->MatrixScale = Vector2T<float>(1.32f, 1.32f);
	//RunningState.PostShaderParams->GameSceneFilterAVG   = 0.32f;
	//RunningState.PostShaderParams->GameSceneBloomRadius = 20;
	return true;
}

bool PsaGameV1Demo::LogicEventLoopGui(GameLogic::FrameworkParams& RunningState) {
	auto PawnActorOBJ = DemoActors->FindGameActor(PawnActorUnqiue);

	// �޸� ImGui �ؼ���ɫ.
	ImGui::PushStyleColor(ImGuiCol_WindowBg,       ImVec4(0.16f, 0.16f, 0.16f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.12f, 0.12f, 0.12f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.72f, 0.72f, 0.72f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg,        ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ImVec4(0.22f, 0.22f, 0.22f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram,  ImVec4(0.25f, 0.25f, 0.25f, 0.72f));

	ImGuiWindowFlags FlagsTemp = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImVec2 HPWindowSize = ImVec2(0.7f, 0.2f) * ImGui::GetIO().DisplaySize;

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - HPWindowSize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.92f));
	ImGui::SetNextWindowSize(HPWindowSize);

	ImGui::Begin("##PActorHP", (bool*)0, FlagsTemp);
	ImGui::Text("AMM: %.0f HP: %.1f", PawnActorBullet, PawnActorOBJ->ActorGetHealth(0));
	ImGui::ProgressBar(PawnActorOBJ->ActorGetHealth(0) / PawnActorHPmax);
	ImGui::End();

	FPS_TEST->RenderingWindowGui();

	ImGui::PopStyleColor(8);
	return true;
}