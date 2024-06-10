// framework_register.
#include"CoreFramework/psag_mainevent_support.h"
// include dev game_logic header.
#include "../PSAGameFrameworkDev/ModuleDebug.h"
#include "../PSAGameFrameworkDev/TestDemoStar.h"

// reg_class: 'PSAGCLASS()'
PSAGCLASS(ModuleDebugClass);
PSAGCLASS(StarDemoClass);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		RegisterDevClass::GAMEREF.ReflectionObjectCreate("StarDemoClass", "TestObject");
	}
}