// psag_actor_global_atomic. RCSZ.

#ifndef __PSAG_ACTOR_GLOBAL_ATOMIC_H
#define __PSAG_ACTOR_GLOBAL_ATOMIC_H
// graphics & physics engine => actor_module.
#include "../CoreFramework/GraphicsEngine/psag_graphics_engine.h"
#include "../CoreFramework/PhysicsEngine/psag_physics_engine.h"

namespace ActorSystemAtomic {
	extern std::atomic<size_t> GLOBAL_PARAMS_ACTORS;
	extern std::atomic<size_t> GLOBAL_PARAMS_EVNS;
	extern std::atomic<size_t> GLOBAL_PARAMS_SHADERS;
	extern std::atomic<size_t> GLOBAL_PARAMS_M_ACTORS;
	extern std::atomic<size_t> GLOBAL_PARAMS_M_EVNS;
	extern std::atomic<size_t> GLOBAL_PARAMS_M_SHADERS;
}

#endif