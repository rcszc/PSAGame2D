// psag_actor_global_atomic.
#include "psag_actor_global_atomic.h"

using namespace std;

namespace ActorSystemAtomic {
	atomic<size_t> GLOBAL_PARAMS_ACTORS    = {};
	atomic<size_t> GLOBAL_PARAMS_EVNS      = {};
	atomic<size_t> GLOBAL_PARAMS_SHADERS   = {};
	atomic<size_t> GLOBAL_PARAMS_M_ACTORS  = {};
	atomic<size_t> GLOBAL_PARAMS_M_EVNS    = {};
	atomic<size_t> GLOBAL_PARAMS_M_SHADERS = {};
}