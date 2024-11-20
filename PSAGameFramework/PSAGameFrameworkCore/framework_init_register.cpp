// framework_init_register.
#include"CoreFramework/psag_mainevent_support.h"

// include dev game_logic header.
#include "../PSAGameFrameworkDev/psagame_v1_demo/v1demo_game.h"
#include "../PSAGameFrameworkDev/psagame_dev_studio/psagame_dev_test.h"
#include "../PSAGameFrameworkDev/psagame_demo_origin/demo_origin_game.h"

// register_class: 'PSAGCLASS()'
PSAGCLASS(DemoGameOrigin);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		RegisterDevClass::GAMEREF.ReflectionObjectCreate("DemoGameOrigin", "DemoGame'ORIGIN'");
	}
}