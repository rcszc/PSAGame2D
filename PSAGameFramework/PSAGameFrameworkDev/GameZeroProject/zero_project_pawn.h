// zero_project_pawn. RCSZ.
// game module(desc): pawn actor & npc.

#ifndef __ZERO_PROJECT_PAWN
#define __ZERO_PROJECT_PAWN
#include "base_define_header.hpp"

extern ShaderCode PawnActorNormal;

struct PlayerActorAttribute {
    float StateMaxHealth = 122000.0f;
};

struct NpcActorAttribute {
    float StateMaxHealth = 12000.0f;
};

class ZPGamePawnActor :public GameLogic::INTERFACE_DEVCLASS_GAME,
    public ZPGameGlobale {
protected:
    Mptr::DefinePointer<PsagActor::ShaderManager> ActorShaders = {};

    PlayerActorAttribute PPActorParams = {};
    NpcActorAttribute NpcActorParams = {};

    // player pawn actor_entity.
    Mptr::DefinePointer<PsagActor::Actor> PlayerActorEntity = {};
    Mptr::DefinePointer<PsagManager::Notify::StationSystem> PlayerActorStation = {};
    Mptr::DefinePointer<PsagManager::Tools::Camera::GamePlayerCameraMP> PlayerActorCamera = {};
    Mptr::DefinePointer<PsagManager::Tools::Pawn::GamePlayerPawn> PlayerActorPawn = {};

    // npc pawn actor_entities.
    Mptr::DefinePointer<PsagActor::ActorsManager> NpcActorEntities = {};

    Mptr::DefinePointer<PsagActor::DebugTools::DebugGamePANEL> DEBUG_GUI = {};

    void CreateNpcActor(size_t number);
    void CreateNpcActorEntity(const Vector2T<float>& position);
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif