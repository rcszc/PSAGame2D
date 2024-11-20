// demo_origin_game_init.
#include "demo_origin_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void DemoGameOriginInit::GameCreateShaderResource(PsagActor::ShaderManager* manager, Vector2T<uint32_t> win_size) {
	namespace LFILE = PsagManager::SyncLoader::FSLD;
	string ResourcePath = "PSAGameFrameworkDev/psagame_demo_origin/origin_resource/";

	// ================================ 加载着色器资源 创建着色器 ================================
	// 加载图片资源.
	auto ImageBackground = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_background.png");
	auto ImageBoundaryX  = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_wall_h.png");
	auto ImageBoundaryY  = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_wall_v.png");

	auto ImageActorShip    = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_nyx.png");
	auto ImageActorShipHDR = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_nyx_hdr.png");
	auto ImageActorShipFX  = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_nyx_fx.png");

	auto ImageActorNpc    = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_npc.png");
	auto ImageActorNpcHDR = LFILE::EasyFileReadRawData(ResourcePath + "demo_origin_npc_hdr.png");

	// 图片源数据编解码器.
	PsagManager::SyncLoader::SyncEncDecImage OperRawImage;

	// 背景着色器.
	PsagActor::ActorShader* RenderBackground = new PsagActor::ActorShader(ShaderCode.TmpScriptBrickImage(), win_size);
	RenderBackground->ShaderImageLoad(OperRawImage.DecodeImageRawData(ImageBackground));

	// 边界墙着色器, 横向&纵向.
	PsagActor::ActorShader* RenderBoundaryX = new PsagActor::ActorShader(ShaderCode.TmpScriptBrickImage(), win_size);
	RenderBoundaryX->ShaderImageLoad(OperRawImage.DecodeImageRawData(ImageBoundaryX));

	PsagActor::ActorShader* RenderBoundaryY = new PsagActor::ActorShader(ShaderCode.TmpScriptBrickImage(), win_size);
	RenderBoundaryY->ShaderImageLoad(OperRawImage.DecodeImageRawData(ImageBoundaryY));

	DemoShaders->CreateActorShader("background", RenderBackground);
	DemoShaders->CreateActorShader("boundary_x", RenderBoundaryX);
	DemoShaders->CreateActorShader("boundary_y", RenderBoundaryY);

	// PawnActor & Fx 着色器.
	PsagActor::ActorShader* RenderActorPawn = new PsagActor::ActorShader(ActorPawnShaderFrag, win_size);
	RenderActorPawn->ShaderImageLoad   (OperRawImage.DecodeImageRawData(ImageActorShip));
	RenderActorPawn->ShaderImageLoadHDR(OperRawImage.DecodeImageRawData(ImageActorShipHDR));

	PsagActor::ActorShader* RenderActorFx = new PsagActor::ActorShader(ActorPawnFxShaderFrag, win_size);
	RenderActorFx->ShaderImageLoad(OperRawImage.DecodeImageRawData(ImageActorShipFX));

	DemoShaders->CreateActorShader("actor_pawn", RenderActorPawn);
	DemoShaders->CreateActorShader("actor_base_fx", RenderActorFx, true);

	// ActorNPC 着色器.
	PsagActor::ActorShader* RenderActorNPC = new PsagActor::ActorShader(ActorNpcShaderFrag, win_size);
	RenderActorNPC->ShaderImageLoad   (OperRawImage.DecodeImageRawData(ImageActorNpc));
	RenderActorNPC->ShaderImageLoadHDR(OperRawImage.DecodeImageRawData(ImageActorNpcHDR));

	DemoShaders->CreateActorShader("actor_npc", RenderActorNPC);
}

void DemoGameOriginInit::GameCreatePawnActor(ResUnique* u_pawn, ResUnique* u_fx) {
	PsagActor::ActorDESC ConfigPawnActor;
	PsagActor::ActorHpDESC PawnActorHealthDESC;

	for (size_t i = 0; i < 3; ++i) {
		PawnActorHealthDESC.InitialActorHealth
			.push_back(PsagActor::ActorHP(PawnMaxHealth[i], 0.42f, PawnMaxHealth[i]));
	}
	ConfigPawnActor.ActorPhysicsWorld   = "DemoOrigin";
	ConfigPawnActor.ActorShaderResource = DemoShaders->FindActorShader("actor_pawn");

	ConfigPawnActor.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup0;
	ConfigPawnActor.ActorCollisionFilter = PsagActor::ActorFlagsPhy::ActorPhyGroup1;

	ConfigPawnActor.ActorHealthSystem = PawnActorHealthDESC;

	ConfigPawnActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
	ConfigPawnActor.InitialPosition = Vector2T<float>(0.0f, 0.0f);
	ConfigPawnActor.InitialScale    = Vector2T<float>(3.2f, 1.8f);

	// ================================ 创建 PawnActor ================================
	*u_pawn = DemoActors->CreateGameActor(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPawn"), ConfigPawnActor
	);

	// ================================ 创建 PawnActor 附属特效 Actor ================================

	ConfigPawnActor.ActorShaderResource = DemoShaders->FindActorShader("actor_base_fx");
	ConfigPawnActor.ActorHealthSystem   = {};
	ConfigPawnActor.InitialScale        = Vector2T<float>(4.6f, 2.8f);

	*u_fx = DemoActors->CreateGameActor(
		PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPawn"), ConfigPawnActor
	);
}

void DemoGameOriginInit::GameCreateNpcActor(ResUnique u_pawn, const Vector2T<float>& position) {
	PsagActor::ActorDESC ConfigNpcActor;
	PsagActor::ActorHpDESC PawnActorHealthDESC;

	// 创建NPC逻辑, 指针由逻辑执行组件自动销毁.
	NpcActorLogic* LogicObjectNPC = new NpcActorLogic(
		DemoActors->FindGameActor(u_pawn),
		[&](const Vector2T<float>& position, float angle) {  }
	);

	PawnActorHealthDESC.InitialActorHealth.push_back(PsagActor::ActorHP(NpcMaxHealth, 0.5f, NpcMaxHealth));

	ConfigNpcActor.ActorPhysicsWorld    = "DemoOrigin";
	ConfigNpcActor.ActorShaderResource  = DemoShaders->FindActorShader("actor_npc");
	ConfigNpcActor.ActorHealthSystem    = PawnActorHealthDESC;
	ConfigNpcActor.ActorComponentConifg |= PsagActor::ActorFlagsComp::ActorEnableLogic;
	ConfigNpcActor.ActorLogicObject     = LogicObjectNPC;

	ConfigNpcActor.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup1;
	ConfigNpcActor.ActorCollisionFilter = PsagActor::ActorFlagsPhy::ActorPhyGroup0;

	ConfigNpcActor.InitialPhysics  = Vector2T<float>(5.8f, 1.0f);
	ConfigNpcActor.InitialPosition = position;

	ConfigNpcActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{
		// 与子弹发生碰撞.
		if (DemoActors->FindGameActor(
			ThisActor->ActorGetPrivate().ActorUniqueCode
		)->ActorGetPrivate().ActorTypeCode == PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorBullet")) {
			// 小范围随机化 受击伤害.
			float Affected = PsagManager::Tools::RAND::GenerateRandomFuncFP32(NpcMaxAffected - 100.0f, NpcMaxAffected);
			// NPC 受击 => 扣血.
			float SetHP = ThisActor->ActorGetHealth(0) <= 0.0f ?
				0.0f :
				ThisActor->ActorGetHealth(0) - Affected;
			ThisActor->ActorModifyHealth(0, SetHP);
		}
	};
	// ================================ 创建 NPC ================================
	DemoActors->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorNPC"), ConfigNpcActor);
}

void DemoGameOriginInit::GameCreateStaticScene() {
	// ================================ 创建 背景静态场景 ================================
	PsagActor::BrickDESC Backgroud;

	Backgroud.BrickPhysicsWorld   = "DemoOrigin";
	Backgroud.BrickShaderResource = DemoShaders->FindActorShader("background");
	// background brick scale.
	Backgroud.InitialScale = Vector2T<float>(50.0f, 50.0f);
	// close phy_collision system.
	Backgroud.EnableCollision = false;

	DemoStatic->CreateGameBrick(Backgroud);

	// ================================ 创建 边界墙静态场景 ================================
	PsagActor::BrickDESC Boundary;
	Boundary.BrickPhysicsWorld = "DemoOrigin";

	// 创建四面围墙(复用DESC).
	Boundary.InitialRenderLayer  = 2.0f;
	Boundary.BrickShaderResource = DemoShaders->FindActorShader("boundary_x");

	Boundary.InitialScale    = Vector2T<float>(50.0f, 2.5f);
	Boundary.InitialPosition = Vector2T<float>(0.0f, -525.0f);
	// create +y.
	DemoStatic->CreateGameBrick(Boundary);

	Boundary.InitialScale    = Vector2T<float>(50.0f, 2.5f);
	Boundary.InitialPosition = Vector2T<float>(0.0f, 525.0f);
	// create -y.
	DemoStatic->CreateGameBrick(Boundary);

	Boundary.BrickShaderResource = DemoShaders->FindActorShader("boundary_y");

	Boundary.InitialScale    = Vector2T<float>(2.5f, 50.0f);
	Boundary.InitialPosition = Vector2T<float>(-525.0f, 0.0f);
	// create +x.
	DemoStatic->CreateGameBrick(Boundary);

	Boundary.InitialScale    = Vector2T<float>(2.5f, 50.0f);
	Boundary.InitialPosition = Vector2T<float>(525.0f, 0.0f);
	// create -x.
	DemoStatic->CreateGameBrick(Boundary);
}