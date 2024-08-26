// v1demo_game_init.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameInitActorsShader(const Vector2T<uint32_t>& w_size) {
	// ͼƬ������.
	PsagManager::SyncLoader::SyncDecodeImage DecodeRawImage;

	PsagManager::SyncLoader::SyncBinFileLoad ImageBackground("demo_v1_material/psag_v1d_world_tex.png");
	PsagManager::SyncLoader::SyncBinFileLoad ImageBoundaryH ("demo_v1_material/psag_v1d_world_Hwall.png");
	PsagManager::SyncLoader::SyncBinFileLoad ImageBoundaryV ("demo_v1_material/psag_v1d_world_Vwall.png");
	PsagManager::SyncLoader::SyncBinFileLoad ImageNPC       ("demo_v1_material/psag_v1d_npc.png");

	PsagManager::SyncLoader::SyncBinFileLoad ImageGuiCrossHair("demo_v1_material/psag_v1d_hair.png");

	// ����ǽ��ɫ��.
	PsagActor::ActorShader* DemoRenderBackground = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), w_size);
	DemoRenderBackground->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(ImageBackground.GetDataBinary()));

	// �߽�ǽ��ɫ��, ����&����.
	PsagActor::ActorShader* DemoRenderBoundaryH = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), w_size);
	DemoRenderBoundaryH->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(ImageBoundaryH.GetDataBinary()));

	PsagActor::ActorShader* DemoRenderBoundaryV = new PsagActor::ActorShader(SYS_PRESET_SC.TmpScriptBrickImage(), w_size);
	DemoRenderBoundaryV->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(ImageBoundaryV.GetDataBinary()));

	// NPC Actors��ɫ��.
	PsagActor::ActorShader* DemoRenderNpcActor = new PsagActor::ActorShader(ShaderFragNPCActor, w_size);
	DemoRenderNpcActor->ShaderLoadImage(DecodeRawImage.DecodeImageRawData(ImageNPC.GetDataBinary()));

	// Pawn Actor��ɫ��, Bullet Actors��ɫ��.
	PsagActor::ActorShader* DemoRenderPawnActor   = new PsagActor::ActorShader(ShaderFragPawnActor, w_size);
	PsagActor::ActorShader* DemoRenderBulletActor = new PsagActor::ActorShader(ShaderFragBulletActor, w_size);

	DemoShaders->CreateActorShader("Background",  DemoRenderBackground);
	DemoShaders->CreateActorShader("BoundaryH",   DemoRenderBoundaryH);
	DemoShaders->CreateActorShader("BoundaryV",   DemoRenderBoundaryV);
	DemoShaders->CreateActorShader("PawnActor",   DemoRenderPawnActor);
	DemoShaders->CreateActorShader("BulletActor", DemoRenderBulletActor);
	DemoShaders->CreateActorShader("NpcActor",    DemoRenderNpcActor);

	GuiViewImage = new PsagManager::FxView::TextureViewImage(
		DecodeRawImage.DecodeImageRawData(ImageGuiCrossHair.GetDataBinary())
	);
}

void PsaGameV1Demo::GameInitBoundary() {
	PsagActor::BrickDESC BricksDESC;
	BricksDESC.BrickPhysicsWorld = "DemoPhysics";
	
	// ��������Χǽ(����DESC).
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

	PawnActorHealthDESC.InitialActorHealth.push_back(PsagActor::ActorHP(PawnActorHPmax, 0.52f, PawnActorHPmax));
	
    ConfigPawnActor.ActorPhysicsWorld   = "DemoPhysics";
    ConfigPawnActor.ActorShaderResource = DemoShaders->FindActorShader("PawnActor");

    ConfigPawnActor.ForceClacEnable   = false;
    ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

    ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
    ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);
	
	PawnActorUnqiue = DemoActors->CreateGameActor(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPawn"), ConfigPawnActor);
}