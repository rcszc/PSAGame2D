// framework_init_register.
#include"CoreFramework/psag_mainevent_support.h"

// include dev game_logic header.
#include "../PSAGameFrameworkDev/psagame_v1_demo/v1demo_game.h"
#include "../PSAGameFrameworkDev/psagame_dev_studio/psagame_dev_test.h"

// reg_class: 'PSAGCLASS()'
PSAGCLASS(PsaGameV1Demo);
PSAGCLASS(PsaGameDevTest);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		//RegisterDevClass::GAMEREF.ReflectionObjectCreate("PsaGameV1Demo", "DemoV1Object");
		RegisterDevClass::GAMEREF.ReflectionObjectCreate("PsaGameDevTest", "TestObject0");
	}
}