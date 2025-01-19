// zero_project_scene. RCSZ.
// game module(desc): main scene.

#ifndef __ZERO_PROJECT_SCENE
#define __ZERO_PROJECT_SCENE
#include "base_define_header.hpp"

extern ShaderCode SceneBackground;

class ZPGameSceneMain :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    Mptr::DefinePointer<PsagActor::ShaderManager> SceneShaders = {};
    Mptr::DefinePointer<PsagActor::BricksManager> SceneStatic  = {};

    Mptr::DefinePointer<PsagManager::SceneFinal::GameFinalProcessing> SceneFinal = {};
    void CreateSceneStatic();

    // x: mouse wheel out, y: value lerp.
    ImVec2 SceneScaleTrans = ImVec2(1.0f, 1.0f);
    void SceneMouseScaleLerp(float timestep, float scale);
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif