// framework_register.
#include"CoreFramework/psag_mainevent_support.h"
// include dev game_logic header.
#include "../PSAGameFrameworkDev/TestDemoStar.h"

// reg_class: 'PSAGCLASS()'
PSAGCLASS(TestClassStar);

namespace PsagFrameworkCore {
	void PSAGame2DFramework::InitializeRegistrationDev() {
		// use: 'RegDev::GAMEREF.ReflectionObjectCreate'
		// register,create class,object...

		RegisterDevClass::GAMEREF.ReflectionObjectCreate("TestClassStar", "TestObject");
	}
}