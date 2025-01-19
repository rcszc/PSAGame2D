// framework_init_register.
#include"CoreFramework/psag_mainevent_support.h"

// include dev game_logic header.
#include "../PSAGameFrameworkDev/GameZeroProject/zero_project_scene.h"
#include "../PSAGameFrameworkDev/GameZeroProject/zero_project_pawn.h"

// register_class: 'PSAGCLASS()'
PSAGCLASS(ZPGameSceneMain);
PSAGCLASS(ZPGamePawnActor);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		RegisterDevClass::GAMEREF.ReflectionObjectCreate("ZPGameSceneMain", "ZPGameScene");
		RegisterDevClass::GAMEREF.ReflectionObjectCreate("ZPGamePawnActor", "ZPGamePActor");
	}
}