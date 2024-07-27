// framework_dev_actor. RCSZ. 20240727. 
// dev_support: actor

#ifndef __FRAMEWORK_DEV_ACTOR_H
#define __FRAMEWORK_DEV_ACTOR_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

namespace PsagActor {
	namespace DebugTools = GameDebugGuiWindow;

	using VertexDESC  = GameActorCore::GameActorShaderVerticesDESC;
	using ActorHpDESC = GameActorCore::GameActorHealthDESC;
	using ActorDESC   = GameActorCore::GameActorActuatorDESC;
	using BrickDESC   = GameBrickCore::GameBrickActuatorDESC;

	using ActorUnique = GameActorCore::ActorPrivateINFO;
	using BirckUnique = GameActorCore::ActorPrivateINFO;

	using ActorRender = GameActorCore::GameActorShader;
	using Actor       = GameActorCore::GameActorActuator;
	using Brick       = GameBrickCore::GameBrickActuator;

	using ActorsManager = GameCoreManager::GameActorActuatorManager;
	using BricksManager = GameCoreManager::GameBrickActuatorManager;

	using OperPhysicalWorld = GamePhysicsOper::GamePhysicalWorld;
}

#endif