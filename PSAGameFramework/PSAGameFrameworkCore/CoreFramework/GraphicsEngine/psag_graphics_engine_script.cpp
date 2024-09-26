// psag_graphics_engine_script. RCSZ.
// framework system preset shader_script.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace GraphicsShaderCode {
	
	void GraphicsEngineLayerRes::Set(const GraphicsShadersDESC& desc) {
		{
			lock_guard<mutex> ResourceLock(ResourceMutex);
			ResourecShaders = desc;
		}
		PushLogger(LogInfo, PSAGM_GLENGINE_SHADER_LABEL, "graphics_engine config system_glsl.");
	}

	GraphicsShadersDESC GraphicsEngineLayerRes::Get() {
		lock_guard<mutex> ResourceLock(ResourceMutex);
		return ResourecShaders;
	}

	GraphicsEngineLayerRes GLOBALRES = {};
}