// zero_project_pawn. RCSZ.
// game module(desc): pawn actor & npc.

#ifndef __ZERO_PROJECT_PAWN
#define __ZERO_PROJECT_PAWN
#include "base_define_header.hpp"

extern ShaderCode PawnActorNormal;

class PlayerActorAttribute {
protected:
    float StateMaxHealth = 122000.0f;
};

class ZPGamePawnActor :public GameLogic::INTERFACE_DEVCLASS_GAME,
    public PlayerActorAttribute {
protected:
    Mptr::DefinePointer<PsagActor::ShaderManager> ActorShaders = {};

    // player pawn actor_entity.
    Mptr::DefinePointer<PsagActor::Actor> PlayerActorEntity = {};
    Mptr::DefinePointer<PsagManager::Tools::Camera::GamePlayerCameraMP> PlayerActorCamera = {};
    Mptr::DefinePointer<PsagManager::Tools::Pawn::GamePlayerPawn> PlayerActorPawn = {};
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif