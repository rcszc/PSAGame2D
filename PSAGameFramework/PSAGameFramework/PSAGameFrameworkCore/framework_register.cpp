// framework_register.
#include"CoreFramework/psag_mainevent_support.h"
// include dev game_logic header.
#include "../PSAGameFrameworkDev/FrameworkTest.h"

// reg_class: 'REDVCLASS()'
REDVCLASS(DevTestClass);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		RegisterDevClass::GAMEREF.ReflectionObjectCreate("DevTestClass", "TestObject");
	}
}