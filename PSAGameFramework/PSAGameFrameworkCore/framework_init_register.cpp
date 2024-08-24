// framework_init_register.
#include"CoreFramework/psag_mainevent_support.h"

// include dev game_logic header.
#include "../PSAGameFrameworkDev/TestDemoStar.h"
#include "../PSAGameFrameworkDev/psagame_v1_demo/v1demo_game.h"

// reg_class: 'PSAGCLASS()'
// PSAGCLASS(StarDemoClass);
PSAGCLASS(PsaGameV1Demo);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		//RegisterDevClass::GAMEREF.ReflectionObjectCreate("StarDemoClass", "TestObject");
		RegisterDevClass::GAMEREF.ReflectionObjectCreate("PsaGameV1Demo", "DemoV1Object");
	}
}