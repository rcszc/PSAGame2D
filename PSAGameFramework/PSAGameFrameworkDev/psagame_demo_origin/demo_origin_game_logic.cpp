// demo_origin_game_logic.
#include "demo_origin_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void DemoGameOrigin::GameActorPawnTrans(float time_step) {
	auto PawnActor   = DemoActors->FindGameActor(PawnActorCode);
	auto PawnFxActor = DemoActors->FindGameActor(PawnActorFxCode);

	// ================================ "Actor"控制移动 ================================
	// move: x: +vec, y: -vec.
	if (ImGui::IsKeyDown(ImGuiKey_W)) PawnActorMove.vector_x += -0.02f * time_step;
	if (ImGui::IsKeyDown(ImGuiKey_S)) PawnActorMove.vector_x +=  0.02f * time_step;

	// rotate: x: +vec, y: -vec.
	if (ImGui::IsKeyDown(ImGuiKey_A)) PawnActorRotate.vector_x +=  10.0f * time_step;
	if (ImGui::IsKeyDown(ImGuiKey_D)) PawnActorRotate.vector_x += -10.0f * time_step;

	PawnActorMove.vector_x   += (0.0f - PawnActorMove.vector_x)   * 0.01f * time_step;
	PawnActorRotate.vector_x += (0.0f - PawnActorRotate.vector_x) * 0.01f * time_step;

	PawnActorMove.vector_x   = PSAG_IMVEC_CLAMP(PawnActorMove.vector_x,   -PawnActorMove.vector_y,   PawnActorMove.vector_y);
	PawnActorRotate.vector_x = PSAG_IMVEC_CLAMP(PawnActorRotate.vector_x, -PawnActorRotate.vector_y, PawnActorRotate.vector_y);

	PawnActor->ActorApplyForceRotate(PawnActorRotate.vector_x);
	// angle => calc => move vector.
	PawnActor->ActorApplyForceMove(Vector2T<float>(
		PawnActorMove.vector_x * cos(PSAG_M_DEGRAD(PawnActor->ActorGetRotate())),
		PawnActorMove.vector_x * sin(PSAG_M_DEGRAD(PawnActor->ActorGetRotate()))
	));

	// ================================ FxActor 跟随 PawnActor ================================
	PawnFxActor->ActorModifyState(PawnActor->ActorGetPosition(), PawnActor->ActorGetRotate());

	// ================================ 相机获取"Actor"参数 ================================
	
}

void DemoGameOrigin::GamePostProcessing(GameLogic::FrameworkParams& params) {
	// ================================ 游戏后期处理参数 ================================
	params.ShaderParamsFinal->GameSceneBloomRadius = 12;
	params.ShaderParamsFinal->GameSceneFilterAVG   = 1.0f;

	params.ShaderParamsFinal->GameSceneOutContrast = 1.115f;
	params.ShaderParamsFinal->GameSceneOutColor    = Vector3T<float>(1.28f, 1.12f, 1.0f);
}

bool DemoGameOrigin::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// ================================ 创建 全局物理系统 ================================
	// mode(1): create, (2): delete.
	PsagActor::OperPhysicalWorld CreatePhysics("DemoOrigin", 1);

	// ================================ 创建 各对象管理器 ================================
    DemoShaders = new PsagActor::ShaderManager();
	DemoStatic  = new PsagActor::BricksManager();
	DemoActors  = new PsagActor::ActorsManager();

	GameCreateShaderResource(DemoShaders, WinSize);
	GameCreateStaticScene();

	// 创建Actors类型绑定.
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorPawn");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorBullet");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorNPC");

	GameCreatePawnActor(&PawnActorCode, &PawnActorFxCode);

	PawnActorShader   = DemoShaders->FindActorShader("actor_pawn");
	PawnActorFxShader = DemoShaders->FindActorShader("actor_base_fx");

	PawnActorMove.vector_y   = 3.8f;
	PawnActorRotate.vector_y = 3.2f;
	
	PlayerCamera = new PsagManager::Tools::Camera::GamePlayerCameraGM(
		Vector2T<float>(-500.0f, -500.0f), Vector2T<float>(500.0f, 500.0f)
	);

	//GameCreateNpcActor(PawnActorCode, Vector2T<float>(120.0f, 0.0f));

	//TestNotify = new PsagManager::Notify::NotifySystem("N_TEST");

	StationTestA = new PsagManager::Notify::StationSystem("SSA");
	//StationTestREC = new PsagManager::Notify::StationSystem("SSB");

	//TestNotify->RegisterStation(StationTestA);
	//TestNotify->RegisterStation(StationTestB);

	CameraScaleLerp.x = 0.5f;
	return true;
}

void DemoGameOrigin::LogicCloseFree() {
	// ================================ 销毁 各对象管理器 ================================
	delete DemoShaders;
	delete DemoStatic;
	delete DemoActors;

	delete PlayerCamera;
	cout << "0000" << endl;
	delete StationTestA;
	//delete StationTestREC;
	cout << "0000" << endl;
	//delete TestNotify;

	PsagActor::OperPhysicalWorld DeletePhysics("DemoOrigin", 2);
}

bool DemoGameOrigin::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	GamePostProcessing(RunningState);

	DemoStatic->RunAllGameBrick();
	DemoActors->RunAllGameActor();

	// ================================ "Actor"着色器参数 ================================
	PawnActorFxLightBarPosition[0] -= RunningState.GameRunTimeSTEP * 0.002f;
	PawnActorFxLightBarPosition[0] = PawnActorFxLightBarPosition[0] < -0.5f ? 1.5f : PawnActorFxLightBarPosition[0];
	PawnActorFxLightBarPosition[1] -= RunningState.GameRunTimeSTEP * 0.008f;
	PawnActorFxLightBarPosition[1] = PawnActorFxLightBarPosition[1] < -0.35f ? 1.35f : PawnActorFxLightBarPosition[1];

	if (ImGui::IsKeyPressed(ImGuiKey_R)) {
		//StationTestA->SED_SetInfoMessage("Hello TEST!");
		//StationTestA->SED_SetInfoTarget("SSB");
		//StationTestA->SED_SendInformation();
	}

	auto PawnUniform = [&]() {
		PawnActorShader->UniformFP32("BarWidth", 1.0f);
		PawnActorShader->UniformFP32("BarPosition", PawnActorFxLightBarPosition[0]);
		PawnActorShader->UniformFP32("Figure", PawnActorFigure);
	};
	PawnActorShader->UniformSetContext(PawnUniform);

	auto PawnFxUniform = [&]() {
		PawnActorFxShader->UniformFP32("BarWidth", 0.7f);
		PawnActorFxShader->UniformFP32("BarPosition", PawnActorFxLightBarPosition[1]);
	};
	PawnActorFxShader->UniformSetContext(PawnFxUniform);

	// ================================ 操作"Actor" & 绘制Gui ================================

	GameActorPawnTrans(RunningState.GameRunTimeSTEP);
	GameRenderGui();
	
	// camera scale_lerp calc, const speed value 0.15
	CameraScaleLerp.y += (CameraScaleLerp.x - CameraScaleLerp.y) * 0.1f * RunningState.GameRunTimeSTEP;
	// setting scale_lerp value.
	CameraScaleLerp.x += ImGui::GetIO().MouseWheel * 0.1f;
	CameraScaleLerp.x = PSAG_IMVEC_CLAMP(CameraScaleLerp.x, 0.2f, 1.32f);

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		PlayerCamera->PlayerCameraRun(Vector2T<float>(ImGui::GetIO().MouseDelta.x, ImGui::GetIO().MouseDelta.y), CameraScaleLerp.y);

	//if (StationTestB->REC_InfoStatusGet()) {
		//cout << StationTestB->REC_GetInfoMessage() << endl;
		//StationTestB->REC_InfoStatusOK();
	//}

	RunningState.CameraParams->MatrixPosition = PlayerCamera->GetCameraPosition();
	RunningState.CameraParams->MatrixScale    = Vector2T<float>(CameraScaleLerp.y, CameraScaleLerp.y);
	return true;
}