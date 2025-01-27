// zero_project_pawn.
#include "zero_project_pawn.h"

using namespace std;
using namespace PSAG_LOGGER;

bool ZPGamePawnActor::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// create actor types: player_pawn, npc
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorPAWN");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorNPC");

	// create pawn & camera
	PlayerActorCamera.CreatePointer(Vector2T<float>(0.0f, 0.0f), WinSize, 0.5f);
	PlayerActorPawn.CreatePointer(Vector2T<float>(128.0f, 128.0f));

	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
	PsagActor::PresetScript ShaderScript = {};

	auto PlayerActorNOR = 
		PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_ppactor_nor.png"));
	// create player pawn actor shader.
	PsagActor::ActorShader* PPActorNormal = new PsagActor::ActorShader(PawnActorNormal, WinSize);
	PPActorNormal->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(PlayerActorNOR));

	auto ActorNPC = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_npc_a.png"));

	// create npc pawn actor shader.
	PsagActor::ActorShader* RenderNPC = new PsagActor::ActorShader(ShaderScript.TmpScriptDrawImage(), WinSize);
	RenderNPC->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(ActorNPC));

	ActorShaders.CreatePointer();
	ActorShaders.Get()->CreateActorShader("PlayerActor.NOR", PPActorNormal);
	ActorShaders.Get()->CreateActorShader("NpcActor.A",      RenderNPC);

	// create player pawn actor shader.
	PsagActor::ActorDESC PPActorDESC;
	PPActorDESC.ActorPhysicsWorld = "ZPGamePhysics";
	
	PsagActor::ActorHpDESC PPActorHealthDESC;

	PPActorHealthDESC.InitialActorHealth.push_back(
		PsagActor::ActorHP(PPActorParams.StateMaxHealth, 0.52f, PPActorParams.StateMaxHealth)
	);
	PPActorDESC.InitialRenderLayer  = 10.0f;
	PPActorDESC.ActorHealthSystem   = PPActorHealthDESC;
	PPActorDESC.ActorShaderResource = ActorShaders.Get()->FindActorShader("PlayerActor.NOR");

	PPActorDESC.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup0;
	PPActorDESC.ActorCollisionFilter = PsagActor::ActorFlagsPhy::ActorPhyGroupALL;

	PPActorDESC.VectorCalcIsForce = false;

	PPActorDESC.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
	PPActorDESC.InitialPosition = Vector2T<float>(0.0f, 0.0f);

	PlayerActorEntity.CreatePointer(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPAWN"), 
		PPActorDESC
	);
	// 创建 & 注册 ppactor 全局广播站点.
	PlayerActorStation.CreatePointer("PPActor");
	GLO_Notify.Get()->RegisterStation(PlayerActorStation.Get());

	NpcActorEntities.CreatePointer();
	GLO_NpcActorsRef = &NpcActorEntities;

	DEBUG_GUI.CreatePointer("GameDebug", 1500.0f);
	DEBUG_GUI.Get()->SettingPPActorRef(PlayerActorEntity.Get());

	CreateNpcActor(10);
	return true;
}

void ZPGamePawnActor::LogicCloseFree() {
	// free resource.
	PlayerActorEntity.DeletePointer();
	ActorShaders.DeletePointer();

	PlayerActorCamera.DeletePointer();
	PlayerActorPawn.DeletePointer();

	PlayerActorStation.DeletePointer();
}

bool ZPGamePawnActor::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	// player actor move.
	PlayerActorPawn.Get()->PlayerPawnRun(25.0f);
	PlayerActorEntity.Get()->ActorApplyForceMove(PlayerActorPawn.Get()->ControlMoveVector);

	NpcActorEntities.Get()->UpdateManagerData();
	NpcActorEntities.Get()->RunAllGameActor();

	// 游戏操作: 鼠标左键射击.
	if (PlayerActorPawn.Get()->MouseButtonPressed_L() && GLO_PlayerBullets < 5) {
		PPActorBulletFire FireParams = {};

		FireParams.Position = PlayerActorEntity.Get()->ActorGetPosition();
		FireParams.Angle    = PlayerActorEntity.Get()->ActorGetAngle();

		// 序列化状态参数发送.
		PlayerActorStation.Get()->SED_SetInfoRawData(
			PsagManager::Notify::GameDataSER::PsagSerialize(FireParams), 
			PsagManager::Notify::DataMode::NotifyDataMode_Object
		);
		PlayerActorStation.Get()->SED_SetInfoTarget("PPActorBullet");
		PlayerActorStation.Get()->SED_SendInformation();
	}
	DEBUG_GUI.Get()->RenderingWindowGui();

	PlayerActorEntity.Get()->ActorUpdate();
	PlayerActorEntity.Get()->ActorRendering();

	// player actor => mouse_pos => rotate.
	float PawnActorAngle = PsagManager::Maths::CalcFuncPointsAngle(
		PlayerActorEntity.Get()->ActorMappingWindowCoord(), 
		PlayerActorPawn.Get()->ControlMousePosition
	);
	PlayerActorEntity.Get()->ActorModifyState(
		PlayerActorEntity.Get()->ActorGetPosition(), 
		PawnActorAngle
	);
	// player actor camera.
	PlayerActorCamera.Get()->PlayerCameraRunFixed(PlayerActorEntity.Get()->ActorGetPosition());
	RunningState.CameraParams->MatrixPosition = PlayerActorCamera.Get()->GetCameraPosition();
	return true;
}