// framework_init_register.
#include"CoreFramework/psag_core_drivers.h"

// include dev game_logic header.
#include "../PSAGameFrameworkDev/GameZeroProject/zero_project_scene.h"
#include "../PSAGameFrameworkDev/GameZeroProject/zero_project_pawn.h"
#include "../PSAGameFrameworkDev/GameZeroProject/zero_project_bullet.h"
#include "../PSAGameFrameworkDev/GameZeroProject/zero_project_gui.h"

#include "../PSAGameFrameworkDev/GameLab/psag_game_lab.h"

// register_class: 'PSAGCLASS()'
PSAGCLASS(ZPGameSceneMain);
PSAGCLASS(ZPGamePawnActor);
PSAGCLASS(ZPGameBulletSystem);
PSAGCLASS(ZPGameGuiMain);

//PSAGCLASS(PSAGameFrameworkLab);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		RegisterDevClass::GAMEREF.ReflectionObjectCreate("ZPGameSceneMain",    "ZPGameScene");
		RegisterDevClass::GAMEREF.ReflectionObjectCreate("ZPGamePawnActor",    "ZPGamePActor");
		RegisterDevClass::GAMEREF.ReflectionObjectCreate("ZPGameBulletSystem", "ZPGameBullet");
		RegisterDevClass::GAMEREF.ReflectionObjectCreate("ZPGameGuiMain",      "ZPGameGui");

		//RegisterDevClass::GAMEREF.ReflectionObjectCreate("PSAGameFrameworkLab", "Lab");
	}
}