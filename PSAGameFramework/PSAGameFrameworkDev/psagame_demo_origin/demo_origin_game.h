// demo_origin_game. 20241106 RCSZ.
// version 1.0.0 release demo: "Origin".

#ifndef __DEMO_ORIGIN_GAME_H
#define __DEMO_ORIGIN_GAME_H

// => include framework core header. [actor][manager]
#include "../../PSAGameFrameworkCore/framework_header_actor.h"
#include "../../PSAGameFrameworkCore/framework_header_manager.h"

#define SHADER_CODE const char*

extern SHADER_CODE ActorPawnShaderFrag;
extern SHADER_CODE ActorPawnFxShaderFrag;
extern SHADER_CODE ActorNpcShaderFrag;

// NPC 最大血量 & 最大受击伤害.
constexpr float NpcMaxHealth   = 5000.0f;
constexpr float NpcMaxAffected = 320.0f;

// Pawn 最大血量 Lv1, Lv2, Lv3
constexpr float PawnMaxHealth[3] = {
    2880000.0f, 4360000, 1320000
};

class NpcActorLogic :public PsagActor::ActorLogicBase {
protected:
	float ThisNPCTimetrOffset = 0.0f;

	PsagActor::Actor* PawnActorPointer = nullptr;

	PsagManager::Tools::Timer::GameCycleTimer FireTimer = {};
	std::function<void(const Vector2T<float>&, float)> BulletFireFunc = {};
public:
    NpcActorLogic(
		PsagActor::Actor* ptr, std::function<void(const Vector2T<float>&, float)> bullet_func) :
		PawnActorPointer(ptr), BulletFireFunc(bullet_func)
	{
		ThisNPCTimetrOffset += 50.0f;
	};

    void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override;
	void HealthRendering(PsagActor::Actor* actor_object);
};

// 对象管理器.
class DemoGameOriginManager {
protected:
    PsagActor::ShaderManager* DemoShaders = nullptr;
    PsagActor::BricksManager* DemoStatic  = nullptr;
    PsagActor::ActorsManager* DemoActors  = nullptr;
};

class DemoGameOriginInit :public DemoGameOriginManager {
protected:
    PsagActor::PresetScript ShaderCode = {};

    void GameCreatePawnActor(ResUnique* u_pawn, ResUnique* u_fx);
    void GameCreateNpcActor(ResUnique u_pawn, const Vector2T<float>& position);

    void GameCreateShaderResource(PsagActor::ShaderManager* manager, Vector2T<uint32_t> win_size);
    void GameCreateStaticScene();
};

class DemoGameOrigin :public DemoGameOriginInit, public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    PsagActor::ActorShader* PawnActorShader   = nullptr;
    PsagActor::ActorShader* PawnActorFxShader = nullptr;

    ResUnique PawnActorCode = NULL, PawnActorFxCode = NULL;

    // vector_x: value, vector_y: limit.
    Vector2T<float> PawnActorMove = {}, PawnActorRotate = {};
    float PawnActorFxLightBarPosition[2] = { 1.2f, 1.2f };
    float PawnActorFigure = 0.98f;

    //PsagManager::Tools::Camera::GamePlayerCameraMP* PlayerCamera = nullptr;
    PsagManager::Tools::Camera::GamePlayerCameraGM* PlayerCamera = nullptr;

    ImVec2 CameraScaleLerp = ImVec2(1.0f, 1.0f);

    std::unique_ptr<PsagManager::Notify::NotifySystem> TestNotify = {};
    
    std::unique_ptr<PsagManager::Notify::StationSystem> StationTestA = {};
    std::unique_ptr<PsagManager::Notify::StationSystem> StationTestB = {};

    void GameActorPawnTrans(float time_step);
    void GamePostProcessing(GameLogic::FrameworkParams& params);

    void GameRenderGui();

public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif