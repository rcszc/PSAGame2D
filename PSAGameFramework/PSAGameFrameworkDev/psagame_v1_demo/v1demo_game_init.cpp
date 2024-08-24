// v1demo_game_init.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameInitBoundary() {
	PsagActor::BrickDESC BricksDESC;
	BricksDESC.BrickPhysicsWorld = "DemoPhysics";
	
	// 创建四面围墙(复用DESC).
	BricksDESC.InitialRenderLayer  = 2.0f;
	BricksDESC.BrickShaderResource = DemoShaders->FindActorShader("BoundaryH");

	BricksDESC.InitialScale    = Vector2T<float>(50.0f, 2.5f);
	BricksDESC.InitialPosition = Vector2T<float>(0.0f, -525.0f);
	// create WALL-1. [y+]
	DemoArchitecture->CreateGameBrick(BricksDESC);

	BricksDESC.InitialScale    = Vector2T<float>(50.0f, 2.5f);
	BricksDESC.InitialPosition = Vector2T<float>(0.0f, 525.0f);
	// create WALL-1. [y-]
	DemoArchitecture->CreateGameBrick(BricksDESC);

	BricksDESC.BrickShaderResource = DemoShaders->FindActorShader("BoundaryV");

	BricksDESC.InitialScale    = Vector2T<float>(2.5f, 50.0f);
	BricksDESC.InitialPosition = Vector2T<float>(-525.0f, 0.0f);
	// create WALL-1. [x+]
	DemoArchitecture->CreateGameBrick(BricksDESC);

	BricksDESC.InitialScale    = Vector2T<float>(2.5f, 50.0f);
	BricksDESC.InitialPosition = Vector2T<float>(525.0f, 0.0f);
	// create WALL-1. [x+]
	DemoArchitecture->CreateGameBrick(BricksDESC);
}

void PsaGameV1Demo::GameInitPawnActor() {
    PsagActor::ActorDESC ConfigPawnActor;
    PsagActor::ActorHpDESC PawnActorHealthDESC;

	PawnActorHealthDESC.InitialActorHealth.push_back(PsagActor::ActorHP(PawnActorHPmax, 0.1f));
	
    ConfigPawnActor.ActorPhysicsWorld   = "DemoPhysics";
    ConfigPawnActor.ActorShaderResource = DemoShaders->FindActorShader("PawnActor");

    ConfigPawnActor.ForceClacEnable   = false;
    ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

    ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);
	
	PawnActorUnqiue = DemoActors->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPawn"), ConfigPawnActor);
}