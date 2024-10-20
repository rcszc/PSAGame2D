// framework_dev_actor. RCSZ. 20240727. 
// dev_support: actor

#ifndef __FRAMEWORK_DEV_ACTOR_H
#define __FRAMEWORK_DEV_ACTOR_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

namespace PsagActorType = GameActorCore::Type;
namespace PsagActor {
	namespace DebugTools = GameDebugGuiWindow;

	using VertexDESC      = GameActorCore::GameActorShaderVerticesDESC;
	using ActorHpDESC     = GameActorCore::GameActorHealthDESC;
	using ActorDESC       = GameActorCore::GameActorExecutorDESC;
	using BrickDESC       = GameBrickCore::GameBrickExecutorDESC;
	using ActorSensorDESC = GameActorCore::GameActorCircleSensorDESC;

	using ActorUnique = GameComponents::ActorPrivateINFO;
	using BirckUnique = size_t;

	using ActorShader = GameActorCore::GameActorShader;
	using Actor       = GameActorCore::GameActorExecutor;
	using Brick       = GameBrickCore::GameBrickExecutor;
	using ActorSensor = GameActorCore::GameActorCircleSensor;

	using ActorHP        = GameComponents::GameActorHP;
	using ActorLogicBase = GameComponents::ActorActionLogicBase;
	using ActorCollision = GameActorCore::GameCollisionPAIR;

	using ShaderManager = GameCoreManager::GameActorShaderManager;
	using ActorsManager = GameCoreManager::GameActorExecutorManager;
	using BricksManager = GameCoreManager::GameBrickExecutorManager;

	// physical_world operate: create, delete. 
	using OperPhysicalWorld = GamePhysicsOper::GamePhysicalWorld;
	// system preset actor shader_script.
	using PresetScript = GameActorCore::GameActorPresetScript;
}
#define ACTOR_COLL_FUNC [&](PsagActor::Actor* ThisActor)

#endif