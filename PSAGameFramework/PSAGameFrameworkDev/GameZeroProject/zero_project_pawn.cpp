// zero_project_pawn.
#include "zero_project_pawn.h"

using namespace std;
using namespace PSAG_LOGGER;

bool ZPGamePawnActor::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// create actor types.
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorPAWN");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorBULLET");
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorNPC");

	// create pawn & camera
	PlayerActorCamera.CreatePointer(Vector2T<float>(0.0f, 0.0f), WinSize, 0.5f);
	PlayerActorPawn.CreatePointer(Vector2T<float>(128.0f, 128.0f));

	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
	auto ActorNOR = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_ppactor_nor.png"));
	// create player pawn actor shader.
	PsagActor::ActorShader* PPActorNormal = new PsagActor::ActorShader(PawnActorNormal, WinSize);
	PPActorNormal->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(ActorNOR));

	ActorShaders.CreatePointer();
	ActorShaders.Get()->CreateActorShader("PlayerActor.NOR", PPActorNormal);

	// create npc pawn actor shader.
	PsagActor::ActorDESC PPActorDESC;
	PPActorDESC.ActorPhysicsWorld = "ZPGamePhysics";

	PsagActor::ActorHpDESC PPActorHealthDESC;

	PPActorHealthDESC.InitialActorHealth.push_back(
		PsagActor::ActorHP(StateMaxHealth, 0.52f, StateMaxHealth)
	);
	PPActorDESC.InitialRenderLayer  = 10.0f;
	PPActorDESC.ActorHealthSystem   = PPActorHealthDESC;
	PPActorDESC.ActorShaderResource = ActorShaders.Get()->FindActorShader("PlayerActor.NOR");

	PPActorDESC.VectorCalcIsForce = false;

	PPActorDESC.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
	PPActorDESC.InitialPosition = Vector2T<float>(0.0f, 0.0f);

	PlayerActorEntity.CreatePointer(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPAWN"), 
		PPActorDESC
	);
	return true;
}

void ZPGamePawnActor::LogicCloseFree() {
	// free resource.
	PlayerActorEntity.DeletePointer();
	ActorShaders.DeletePointer();

	PlayerActorCamera.DeletePointer();
	PlayerActorPawn.DeletePointer();
}

bool ZPGamePawnActor::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	// player actor move.
	PlayerActorPawn.Get()->PlayerPawnRun(10.0f);
	PlayerActorEntity.Get()->ActorApplyForceMove(PlayerActorPawn.Get()->ControlMoveVector);

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