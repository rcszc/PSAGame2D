// psag_actor_module_manager.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameCoreManager {
	// ******************************** game actor_shaders ********************************

	GameActorShaderManager::~GameActorShaderManager() {
		// free valid objects.
		for (auto& ShaderItem : GameShaderDataset) {
			if (ShaderItem.second == nullptr) {
				PushLogger(LogError, PSAGM_CORE_MAG_LABEL, "actor_shader manager: %s nullptr!",
					ShaderItem.first.c_str());
				continue;
			}
			// free actor_shader.
			delete ShaderItem.second;
		}
		PushLogger(LogInfo, PSAGM_CORE_MAG_LABEL, "actor_shader manager system delete.");
		// ATOMIC ENTITIES COUNTER.
		--ActorSystemAtomic::GLOBAL_PARAMS_M_SHADERS;
	}

	bool GameActorShaderManager::CreateActorShader(
		const char* shader_name, GameActorCore::GameActorShader* shader, bool default_circle
	) {
		// shader_pointer = null | name != empty.
		if (shader == nullptr || FindActorShader(shader_name) != nullptr) {
			PushLogger(LogError, PSAGM_CORE_MAG_LABEL,
				"actor_shader(mag) item failed create(nullptr | name_err).");
			return false;
		}
		// create shader.
		shader->CreateShaderResource(default_circle);
		GameShaderDataset[shader_name] = shader;
		return true;
	}

	bool GameActorShaderManager::DeleteActorShader(const char* shader_name) {
		auto it = GameShaderDataset.find(shader_name);
		if (it != GameShaderDataset.end()) {
			// shader_pointer null.
			if (it->second == nullptr) {
				PushLogger(LogError, PSAGM_CORE_MAG_LABEL, "actor_shader(mag) item failed delete.");
				return  false;
			}
			delete it->second;
			GameShaderDataset.erase(it);
			return true;
		}
		return false;
	}

	GameActorCore::GameActorShader* GameActorShaderManager::FindActorShader(const char* shader_name) {
		return GameShaderDataset.find(shader_name) == GameShaderDataset.end() ? nullptr : GameShaderDataset[shader_name];
	}

	// ******************************** game actor_actuator ********************************

	GameActorExecutorManager::~GameActorExecutorManager() {
		// free valid objects.
		for (auto& ActorItem : GameActorDataset)
			if (ActorItem.second != nullptr)
				delete ActorItem.second;
		PushLogger(LogInfo, PSAGM_CORE_MAG_LABEL, "game_actor manager system delete.");
		// ATOMIC ENTITIES COUNTER.
		--ActorSystemAtomic::GLOBAL_PARAMS_M_ACTORS;
	}

	size_t GameActorExecutorManager::CreateGameActor(
		uint32_t actor_code, const GameActorCore::GameActorExecutorDESC& actor_desc
	) {
		GameActorCore::GameActorExecutor* CreateGameActor = 
			new GameActorCore::GameActorExecutor(actor_code, actor_desc);
		// actor pointer = nullptr.
		if (CreateGameActor == nullptr) {
			PushLogger(LogError, PSAGM_CORE_MAG_LABEL, "game_actor(mag) item failed create.");
			return NULL;
		}
		size_t UniqueCode = CreateGameActor->ActorGetPrivate().ActorUniqueCode;
		GameActorDataset[UniqueCode] = CreateGameActor;
		return UniqueCode;
	}

	bool GameActorExecutorManager::DeleteGameActor(size_t unique_code) {
		auto it = GameActorDataset.find(unique_code);
		if (it != GameActorDataset.end()) {
			// find actor_item => push free_list => delete.
			GameActorFreeList.push_back(it->first);
			return true;
		}
		return false;
	}

	GameActorCore::GameActorExecutor* GameActorExecutorManager::FindGameActor(size_t unique_code) {
		return GameActorDataset.find(unique_code) == GameActorDataset.end() ? nullptr : GameActorDataset[unique_code];
	}

	void GameActorExecutorManager::UpdateManagerData() {
		for (auto& FreeItem : GameActorFreeList) {
			auto it = GameActorDataset.find(FreeItem);
			if (it != GameActorDataset.end()) {
				// delete actor_object & erase info.
				if (it->second != nullptr) delete it->second;
				GameActorDataset.erase(it);
			}
		}
		GameActorFreeList.clear();
	}

	void GameActorExecutorManager::RunAllGameActor() {
		for (auto& RunActorItem : GameActorDataset) {
			// update actor state.
			RunActorItem.second->ActorUpdateHealth();
			RunActorItem.second->ActorUpdate();
			RunActorItem.second->ActorRendering();
		}
	}

	// ******************************** game brick_actuator ********************************

	GameBrickExecutorManager::~GameBrickExecutorManager() {
		// free valid objects.
		for (auto& BrickItem : GameBrickDataset)
			if (BrickItem.second != nullptr)
				delete BrickItem.second;
		PushLogger(LogInfo, PSAGM_CORE_MAG_LABEL, "game_brick manager system delete.");
		// ATOMIC ENTITIES COUNTER.
		--ActorSystemAtomic::GLOBAL_PARAMS_M_EVNS;
	}

	size_t GameBrickExecutorManager::CreateGameBrick(const GameBrickCore::GameBrickExecutorDESC& brick_desc) {
		GameBrickCore::GameBrickExecutor* CreateGameBrick = new GameBrickCore::GameBrickExecutor(brick_desc);
		// brick pointer = nullptr.
		if (CreateGameBrick == nullptr) {
			PushLogger(LogError, PSAGM_CORE_MAG_LABEL, "game_brick(mag) item failed create.");
			return NULL;
		}
		size_t UniqueCode = CreateGameBrick->BrickGetUniqueID();
		GameBrickDataset[UniqueCode] = CreateGameBrick;
		return UniqueCode;
	}

	bool GameBrickExecutorManager::DeleteGameBrick(size_t unique_code) {
		auto it = GameBrickDataset.find(unique_code);
		if (it != GameBrickDataset.end()) {
			// brick_pointer null.
			if (it->second == nullptr) {
				PushLogger(LogError, PSAGM_CORE_MAG_LABEL, "game_brick(mag) item failed delete.");
				return false;
			}
			delete it->second;
			GameBrickDataset.erase(it);
			return true;
		}
		return false;
	}

	void GameBrickExecutorManager::RunAllGameBrick() {
		for (auto& RunBrickItem : GameBrickDataset)
			RunBrickItem.second->BrickRendering();
	}
}