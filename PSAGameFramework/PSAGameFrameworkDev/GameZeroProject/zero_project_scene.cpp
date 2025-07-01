// zero_project_scene.
#include "zero_project_scene.h"

using namespace std;
using namespace PSAG_LOGGER;

void ZPGameSceneMain::CreateSceneStatic() {
	PsagActor::EnvmtDESC EnvmtsDESC;
	EnvmtsDESC.EnvmtPhysicsWorld = "ZPGamePhysics";

	EnvmtsDESC.InitialRenderLayer  = 2.0f;
	EnvmtsDESC.EnvmtShaderResource = SceneShaders.Get()->FindActorShader("BoundaryH");

	EnvmtsDESC.InitialScale    = Vector2T<float>(50.0f, 2.5f);
	EnvmtsDESC.InitialPosition = Vector2T<float>(0.0f, -525.0f);
	// create boundary.[y+]
	SceneStatic.Get()->CreateGameEnvmt(EnvmtsDESC);

	EnvmtsDESC.InitialScale    = Vector2T<float>(50.0f, 2.5f);
	EnvmtsDESC.InitialPosition = Vector2T<float>(0.0f, 525.0f);
	// create boundary.[y-]
	SceneStatic.Get()->CreateGameEnvmt(EnvmtsDESC);

	EnvmtsDESC.EnvmtShaderResource = SceneShaders.Get()->FindActorShader("BoundaryV");

	EnvmtsDESC.InitialScale    = Vector2T<float>(2.5f, 50.0f);
	EnvmtsDESC.InitialPosition = Vector2T<float>(-525.0f, 0.0f);
	// create boundary.[x+]
	SceneStatic.Get()->CreateGameEnvmt(EnvmtsDESC);

	EnvmtsDESC.InitialScale    = Vector2T<float>(2.5f, 50.0f);
	EnvmtsDESC.InitialPosition = Vector2T<float>(525.0f, 0.0f);
	// create boundary.[x+]
	SceneStatic.Get()->CreateGameEnvmt(EnvmtsDESC);

	PsagActor::EnvmtDESC BackgroundDESC;
	BackgroundDESC.EnvmtPhysicsWorld = "ZPGamePhysics";

	BackgroundDESC.InitialRenderLayer  = 2.0f;
	BackgroundDESC.EnvmtShaderResource = SceneShaders.Get()->FindActorShader("Background");
	// close phy_collision system.
	BackgroundDESC.EnableCollision = false;

	BackgroundDESC.InitialScale    = Vector2T<float>(100.0f, 100.0f);
	BackgroundDESC.InitialPosition = Vector2T<float>(0.0f, 0.0f);
	// create background.
	SceneStatic.Get()->CreateGameEnvmt(BackgroundDESC);
}

void ZPGameSceneMain::SceneMouseScaleLerp(float timestep, float NoiseScale) {
	// mouse wheel scale => lerp.
	SceneScaleTrans.x -= ImGui::GetIO().MouseWheel * 0.12f;
	SceneScaleTrans.y += (SceneScaleTrans.x - SceneScaleTrans.y) * 0.032f * timestep * NoiseScale;
	SceneScaleTrans.x = PSAG_IMVEC_CLAMP(SceneScaleTrans.x, 0.4f, 2.0f);
	// mouse middle click => normal scale.
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
		SceneScaleTrans.x = 1.0f;
}

bool ZPGameSceneMain::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// create global physics world.
	PsagActor::OperPhysicalWorld CreatePhysics("ZPGamePhysics", 1);
	SceneFinal.CreatePointer();
	// create manager(s).
	SceneShaders.CreatePointer();
	SceneStatic.CreatePointer();

	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;

	auto ImgBoundaryH = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_wall_h.png"));
	auto ImgBoundaryV = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_wall_v.png"));

	PsagActor::ActorShader* RenderBackground = 
		new PsagActor::ActorShader(SceneBackground, WinSize);

	PsagActor::PresetScript ShaderScript = {};

	// boundary wall [h] shader.
	PsagActor::ActorShader* RenderBH = 
		new PsagActor::ActorShader(ShaderScript.TmpScriptDrawImage("WallH"), WinSize);
	RenderBH->ShaderImageLADD("WallH", DecodeRawImage.DecodeImageRawData(ImgBoundaryH));

	// boundary wall [v] shader.
	PsagActor::ActorShader* RenderBV = 
		new PsagActor::ActorShader(ShaderScript.TmpScriptDrawImage("WallV"), WinSize);
	RenderBV->ShaderImageLADD("WallV", DecodeRawImage.DecodeImageRawData(ImgBoundaryV));

	SceneShaders.Get()->CreateActorShader("Background", RenderBackground);

	SceneShaders.Get()->CreateActorShader("BoundaryH", RenderBH);
	SceneShaders.Get()->CreateActorShader("BoundaryV", RenderBV);
	/*
	PsagManager::StartAnimLOAD SALoader = {};

	SALoader.SettingPlayTime(1.75f);
	SALoader.SettingLerpSpeed(0.85f);

	auto SAI1 = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("psagame2d/PSAGame2D0.png"));
	auto SAI2 = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("psagame2d/PSAGame2D1.png"));
	auto SAI3 = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("psagame2d/PSAGame2D2.png"));

	SALoader.AnimImageADD(DecodeRawImage.DecodeImageRawData(SAI1));
	SALoader.AnimImageADD(DecodeRawImage.DecodeImageRawData(SAI2));
	SALoader.AnimImageADD(DecodeRawImage.DecodeImageRawData(SAI3));
	*/
	CreateSceneStatic();

	// create game global notify system.
	GLO_Notify.CreatePointer("ZPGameNotify");
	return true;
}

void ZPGameSceneMain::LogicCloseFree() {
	// free resource.
	SceneStatic.DeletePointer();
	SceneShaders.DeletePointer();
	SceneFinal.DeletePointer();

	GLO_Notify.DeletePointer();
}

bool ZPGameSceneMain::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	// setting final proc_params.
	RunningState.ShaderParamsFinal->GameSceneBloomRadius = 12;
	RunningState.ShaderParamsFinal->GameSceneFilterAVG   = 0.22f;
	RunningState.ShaderParamsFinal->GameSceneOutVignette = Vector2T<float>(0.7f, 0.28f);
	RunningState.ShaderParamsFinal->GameSceneBloomBlend  = Vector2T<float>(0.92f, 1.72f);
	RunningState.ShaderParamsFinal->GameSceneOutContrast = 1.032f;
	
	//SceneFinal.Get()->GetFinalParamsPonter(RunningState.ShaderParamsFinal);
	//SceneFinal.Get()->RenderDebugParamsPanel("GameFINAL");

	SceneStatic.Get()->RunAllGameEnvmt();

	SceneMouseScaleLerp(RunningState.GameRunTimeSTEP, 1.0f);
	RunningState.CameraParams->MatrixScale 
		= Vector2T<float>(SceneScaleTrans.y, SceneScaleTrans.y);
	return true;
}