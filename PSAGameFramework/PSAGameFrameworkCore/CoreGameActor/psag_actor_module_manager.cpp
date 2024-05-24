// psag_actor_module_manager.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameActorManager {
	GameActorActuatorManager::~GameActorActuatorManager() {
		for (auto& ActorItem : GameActorDataset)
			if (ActorItem.second != nullptr)
				delete ActorItem.second;
		PushLogger(LogInfo, PSAGM_ACTOR_MAG_LABEL, "game_actor manager system delete.");
	}

	size_t GameActorActuatorManager::CreateGameActor(
		uint32_t actor_code, const GameActorCore::GameActorActuatorDESC& actor_desc
	) {
		GameActorCore::GameActorActuator* CreateGameActor = 
			new GameActorCore::GameActorActuator(actor_code, actor_desc);
		// actor pointer = nullptr.
		if (CreateGameActor == nullptr) {
			PushLogger(LogError, PSAGM_ACTOR_MAG_LABEL, "game_actor(mag) item failed create.");
			return NULL;
		}
		size_t UniqueCode = CreateGameActor->ActorGetPrivate().ActorUniqueCode;
		GameActorDataset[UniqueCode] = CreateGameActor;
		return UniqueCode;
	}

	bool GameActorActuatorManager::DeleteGameActor(size_t unique_code) {
		auto it = GameActorDataset.find(unique_code);
		if (it != GameActorDataset.end()) {
			// find actor_item => push free_list => delete.
			GameActorFreeList.push_back(it->first);
			return true;
		}
		return false;
	}

	GameActorCore::GameActorActuator* GameActorActuatorManager::FindGameActor(size_t unique_code) {
		return (GameActorDataset.find(unique_code) != GameActorDataset.end()) ? GameActorDataset[unique_code] : nullptr;
	}

	void GameActorActuatorManager::UpdateManagerData() {
		for (auto& FreeItem : GameActorFreeList) {
			auto it = GameActorDataset.find(FreeItem);
			if (it != GameActorDataset.end()) {
				// delete actor_object & erase info.
				if (it->second != nullptr)
					delete it->second;
				GameActorDataset.erase(it);
			}
		}
		GameActorFreeList.clear();
	}

	void GameActorActuatorManager::RunAllGameActor() {
		for (auto& RunActorItem : GameActorDataset) {
			// update actor state.
			RunActorItem.second->ActorUpdateHealth();
			RunActorItem.second->ActorUpdate();
			RunActorItem.second->ActorRendering();
		}
	}
}