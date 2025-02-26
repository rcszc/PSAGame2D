// psag_manager_module_files.
#include "psag_manager_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameManagerCore {
	namespace GameFileSystemPath {

		bool GamePackgaeResourcePath::ADD_SystemResPath(
			const string& name, const string& res_path
		) {
			unique_lock Lock(ResourcePathMutex);
			auto it = ResourcePath.find(name);
			if (it == ResourcePath.end()) {
				// add resource path key_value.
				ResourcePath.emplace(name, res_path);
				PushLogger(LogInfo, PSAGM_MANAGER_FINAL_LABEL, 
					"add game resource name: %s, path: %s", name.c_str(), res_path.c_str()
				);
				return true;
			}
			PushLogger(LogError, PSAGM_MANAGER_FINAL_LABEL, 
				"failed add game resource name: %s", name.c_str()
			);
			return false;
		}

		bool GamePackgaeResourcePath::DELETE_SystemResPath(const string& name) {
			unique_lock Lock(ResourcePathMutex);
			auto it = ResourcePath.find(name);
			if (it != ResourcePath.end()) {
				string PathTemp = it->second;
				// delete resource path key_value.
				ResourcePath.erase(it);
				PushLogger(LogInfo, PSAGM_MANAGER_FINAL_LABEL,
					"delete game resource name: %s, path: %s", name.c_str(), PathTemp.c_str()
				);
				return true;
			}
			PushLogger(LogError, PSAGM_MANAGER_FINAL_LABEL,
				"failed delete game resource name: %s", name.c_str()
			);
			return false;
		}

		string GamePackgaeResourcePath::FIND_SystemResPath(const string& name) {
			shared_lock Lock(ResourcePathMutex);
			return ResourcePath.find(name) == ResourcePath.end() ? 
				string() : ResourcePath[name];
		}

		uint64_t GamePackgaeResourcePath::FIND_SystemResPHash(const string& name) {
			string PathTemp = FIND_SystemResPath(name);
			if (PathTemp.empty()) return NULL;
			// resource path => hash code type: uint64_t.
			hash<string> StrHasher;
			return StrHasher(PathTemp);
		}
	}
}