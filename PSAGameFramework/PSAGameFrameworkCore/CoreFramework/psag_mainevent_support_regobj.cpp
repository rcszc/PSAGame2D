// psag_mainevent_support_regobj.
#include "psag_mainevent_support.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace RegisterDevClass {
	
	void PsagReflectionSYS::ReflectionObjectCreate(const char* ClassName, const char* UniqueName) {
		auto& Registry = ReflectionFactory::GetRegistry();
		// find unique_name => register.
		auto it = Registry.find(ClassName);
		if (it != Registry.end()) {
			PsagFrameworkStart::System::GLOBAL_FMC_OBJECT->GameRegistrationObject(ClassName, UniqueName, it->second());
			return;
		}
		PushLogger(LogError, PSAGM_FRAME_DEVREG_LABEL, "failed: class not found in the registry.");
	}

	void PsagReflectionSYS::ReflectionObjectDelete(const char* UniqueName) {
		PsagFrameworkStart::System::GLOBAL_FMC_OBJECT->GameDeleteObject(UniqueName);
	}
	PsagReflectionSYS GAMEREF;
}