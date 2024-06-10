// ModuleDebug.
#include "ModuleDebug.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace TActor = GameActorCore::Type;

bool ModuleDebugClass::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
    
    return true;
}

void ModuleDebugClass::LogicCloseFree() {

}

bool ModuleDebugClass::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

    return true;
}

bool ModuleDebugClass::LogicEventLoopGui(GameLogic::FrameworkParams& RunningState) {

    GameDebugGuiWindow::DebugWindowGuiFPS("PSA-Game2D ModuleDebug", FramerateParams);
    
    return true;
}