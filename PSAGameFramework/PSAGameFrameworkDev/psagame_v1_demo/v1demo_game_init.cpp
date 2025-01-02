// v1demo_game_init.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameInitActorsShader(const Vector2T<uint32_t>& w_size) {
	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;

	auto ImageBackground = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("PSAGameFrameworkDev/psagame_v1_demo/demo_v1_material/psag_v1d_world_tex.png");
	auto ImageBoundaryH  = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("PSAGameFrameworkDev/psagame_v1_demo/demo_v1_material/psag_v1d_world_Hwall.png");
	auto ImageBoundaryV  = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("PSAGameFrameworkDev/psagame_v1_demo/demo_v1_material/psag_v1d_world_Vwall.png");
	auto ImageNPC        = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("PSAGameFrameworkDev/psagame_v1_demo/demo_v1_material/psag_v1d_npc.png");

	auto ImageGuiCrossHair = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("PSAGameFrameworkDev/psagame_v1_demo/demo_v1_material/psag_v1d_hair.png");

	// 背景墙着色器.
	PsagActor::ActorShader* DemoRenderBackground = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), w_size);
	DemoRenderBackground->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(ImageBackground));

	// 边界墙着色器, 横向&纵向.
	PsagActor::ActorShader* DemoRenderBoundaryH = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), w_size);
	DemoRenderBoundaryH->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(ImageBoundaryH));

	PsagActor::ActorShader* DemoRenderBoundaryV = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), w_size);
	DemoRenderBoundaryV->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(ImageBoundaryV));

	// NPC Actors着色器.
	PsagActor::ActorShader* DemoRenderNpcActor = new PsagActor::ActorShader(ShaderFragNPCActor, w_size);
	DemoRenderNpcActor->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(ImageNPC));

	// Pawn Actor着色器, Bullet Actors着色器.
	PsagActor::ActorShader* DemoRenderPawnActor   = new PsagActor::ActorShader(ShaderFragPawnActor, w_size);
	PsagActor::ActorShader* DemoRenderBulletActor = new PsagActor::ActorShader(ShaderFragBulletActor, w_size);

	DemoShaders->CreateActorShader("Background",  DemoRenderBackground);
	DemoShaders->CreateActorShader("BoundaryH",   DemoRenderBoundaryH);
	DemoShaders->CreateActorShader("BoundaryV",   DemoRenderBoundaryV);
	DemoShaders->CreateActorShader("PawnActor",   DemoRenderPawnActor);
	DemoShaders->CreateActorShader("BulletActor", DemoRenderBulletActor);
	DemoShaders->CreateActorShader("NpcActor",    DemoRenderNpcActor);

	GuiViewImage = new PsagManager::FxView::TextureViewImage(DecodeRawImage.DecodeImageRawData(ImageGuiCrossHair));
}

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

void PsaGameV1Demo::GameCreatePawnActor() {
    PsagActor::ActorDESC ConfigPawnActor;
    PsagActor::ActorHpDESC PawnActorHealthDESC;

	PawnActorHealthDESC.InitialActorHealth.push_back(PsagActor::ActorHP(PawnActorHPmax, 0.52f, PawnActorHPmax));
	
    ConfigPawnActor.ActorPhysicsWorld   = "DemoPhysics";
    ConfigPawnActor.ActorShaderResource = DemoShaders->FindActorShader("PawnActor");

    ConfigPawnActor.VectorCalcIsForce   = false;
    ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

    ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);
	
	PawnActorUnqiue = DemoActors->CreateGameActor(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPawn"), ConfigPawnActor);
}