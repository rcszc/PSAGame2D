// psaglow_opengl_resource.
#include "psaglow_opengl.hpp"

namespace PSAG_OGL_RES {
	PsagDebugInvalidKeyCount GLOBAL_DEBUG_COUNT = {};
	// tmu state flags find idle ?
	inline bool TMUstateFlagFind(std::vector<bool>& data, uint32_t& sampler_count) {
		bool IDLEresources = false;
		for (size_t i = 0; i < data.size(); ++i) {
			// texture mapping units slot_flag.
			if (data[i] == false) {
				// slot => open(ture), idle = ture.
				data[i] = true; IDLEresources = true;
				sampler_count = (uint32_t)i;
				break;
			}
		}
		return IDLEresources;
	}

	uint32_t PsagResTexSamplerOGL::AllocTexMapUnitCount() {
		bool FindIdelFlag = false;
		uint32_t ReturnTmuCount = NULL;
		{
			std::lock_guard<std::mutex> Lock(TmuStateMutex);
			FindIdelFlag = TMUstateFlagFind(TmuStateFlag, ReturnTmuCount);
		}
		// 0号(NULL)纹理采样器为系统默认(不使用).
		ReturnTmuCount += 1;
		if (!FindIdelFlag) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "tmu failed alloc idle = 0.");
			return NULL;
		}
		PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "tmu alloc u: %u", ReturnTmuCount);
		return ReturnTmuCount;
	}

	void PsagResTexSamplerOGL::FreeTexMapUnitCount(uint32_t count) {
		std::lock_guard<std::mutex> Lock(TmuStateMutex);
		// alloc tmu > limit.
		if (count >= TmuStateFlag.size()) {
			PsagLowLog(LogError, PSAG_OGLRES_LABEL, "tmu failed delete(space) fc >= max.");
			return;
		}
		if (count != NULL) count -= 1;
		TmuStateFlag[(size_t)count] = false;
		// free -= 1, offset, print += 1.
		PsagLowLog(LogTrace, PSAG_OGLRES_LABEL, "tmu delete(space): %u", count + 1);
	}

	// **************************************** Shader ****************************************

	PsagShader PsagResShadersOGL::ResourceFind(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceShaderMutex);
#if PSAG_DEBUG_MODE
		if (ResourceShaderMap.find(key) == ResourceShaderMap.end())
			++GLOBAL_DEBUG_COUNT.ResourceShader;
#endif
		return ResourceShaderMap.find(key) == ResourceShaderMap.end() ? 
			OPENGL_INVALID_HANDEL : ResourceShaderMap[key];
	}

	bool PsagResShadersOGL::ResourceStorage(ResUnique key, PsagGLmanagerShader* res) {
		std::lock_guard<std::mutex> Lock(ResourceShaderMutex);

		auto it = ResourceShaderMap.find(key);
		if (it != ResourceShaderMap.end()) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "shader: failed storage duplicate_key: %u", key);
			return DEF_PSAGSTAT_FAILED;
		}
		ResourceFlag ResFlag = {};
		auto ResStorage = res->_MS_GETRES(ResFlag);

		// check resource = normal ?
		if (ResFlag == DEFRES_FLAG_NORMAL) {
			ResourceShaderMap[key] = ResStorage;
			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "shader: storage key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "shader: failed storage, key: %u, code: %i", key, ResFlag);
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagResShadersOGL::ResourceDelete(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceShaderMutex);

		auto it = ResourceShaderMap.find(key);
		if (it != ResourceShaderMap.end()) {
			// delete shader program => clear map_item.
			glDeleteProgram(it->second);
			ResourceShaderMap.erase(it);

			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "shader: delete key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "shader: failed delete, not found key.");
		return DEF_PSAGSTAT_FAILED;
	}

	// **************************************** Texture ****************************************

	PsagTextureAttribute PsagResTextureOGL::ResourceFind(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceTextureMutex);
#if PSAG_DEBUG_MODE
		if (ResourceTextureMap.find(key) == ResourceTextureMap.end())
			++GLOBAL_DEBUG_COUNT.ResourceTexture;
#endif
		return ResourceTextureMap.find(key) == ResourceTextureMap.end() ? 
			PsagTextureAttribute() : ResourceTextureMap[key];
	}

	bool PsagResTextureOGL::ResourceStorage(ResUnique key, PsagGLmangerTextureStorage* res) {
		std::lock_guard<std::mutex> Lock(ResourceTextureMutex);

		auto it = ResourceTextureMap.find(key);
		if (it != ResourceTextureMap.end()) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "texture: failed storage duplicate_key: %u", key);
			return DEF_PSAGSTAT_FAILED;
		}
		ResourceFlag ResFlag = {};
		auto ResStorage = res->_MS_GETRES(ResFlag);

		// check resource = normal ?
		if (ResFlag == DEFRES_FLAG_NORMAL) {
			ResourceTextureMap[key] = ResStorage;
			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "texture: storage key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "texture: failed storage, key: %u, code: %i", key, ResFlag);
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagResTextureOGL::ResourceDelete(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceTextureMutex);

		auto it = ResourceTextureMap.find(key);
		if (it != ResourceTextureMap.end()) {
			// delete texture2d handle => clear map_item.
			glDeleteTextures(1, &it->second.Texture);
			ResourceTextureMap.erase(it);

			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "texture: delete key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "texture: failed delete, not found key.");
		return DEF_PSAGSTAT_FAILED;
	}

	// **************************************** VertexBuffer ****************************************

	PsagVertexBufferAttribute PsagResVertexBufferOGL::ResourceFind(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceVertexBufferMutex);
#if PSAG_DEBUG_MODE
		if (ResourceVertexBufferMap.find(key) == ResourceVertexBufferMap.end())
			++GLOBAL_DEBUG_COUNT.ResourceVBO;
#endif
		return ResourceVertexBufferMap.find(key) == ResourceVertexBufferMap.end() ? 
			PsagVertexBufferAttribute() : ResourceVertexBufferMap[key];
	}

	bool PsagResVertexBufferOGL::ResourceStorage(ResUnique key, PsagGLmanagerVertex* res) {
		std::lock_guard<std::mutex> Lock(ResourceVertexBufferMutex);

		auto it = ResourceVertexBufferMap.find(key);
		if (it != ResourceVertexBufferMap.end()) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "vbo: failed storage duplicate_key: %u", key);
			return DEF_PSAGSTAT_FAILED;
		}
		ResourceFlag ResFlag = {};
		auto ResStorage = res->_MS_GETRES(ResFlag);

		// check resource = normal ?
		if (ResFlag == DEFRES_FLAG_NORMAL) {
			ResourceVertexBufferMap[key] = ResStorage;
			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "vbo: storage key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "vbo: failed storage, key: %u, code: %i", key, ResFlag);
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagResVertexBufferOGL::ResourceDelete(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceVertexBufferMutex);
		
		auto it = ResourceVertexBufferMap.find(key);
		if (it != ResourceVertexBufferMap.end()) {
			// delete vbo handle => clear map_item. vao is mapping.
			glDeleteBuffers(1, &it->second.DataBuffer);
			ResourceVertexBufferMap.erase(it);

			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "vbo: delete key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "vbo: failed delete, not found key.");
		return DEF_PSAGSTAT_FAILED;
	}

	PsagVertexBufferAttribute* PsagResVertexBufferOGL::ExtResourceMapping(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceVertexBufferMutex);
		return (ResourceVertexBufferMap.find(key) != ResourceVertexBufferMap.end()) ? &ResourceVertexBufferMap[key] : nullptr;
	}

	// **************************************** VertexAttribute ****************************************

	PsagVertexAttribute PsagResVertexAttribOGL::ResourceFind(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceVertexAttrMutex);
#if PSAG_DEBUG_MODE
		if (ResourceVertexAttrMap.find(key) == ResourceVertexAttrMap.end())
			++GLOBAL_DEBUG_COUNT.ResourceVAO;
#endif
		return ResourceVertexAttrMap.find(key) == ResourceVertexAttrMap.end() ? 
			PsagVertexAttribute() : ResourceVertexAttrMap[key];
	}

	bool PsagResVertexAttribOGL::ResourceStorage(ResUnique key, PsagVertexAttribute res) {
		std::lock_guard<std::mutex> Lock(ResourceVertexAttrMutex);

		auto it = ResourceVertexAttrMap.find(key);
		if (it != ResourceVertexAttrMap.end()) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "vao: failed storage duplicate_key: %u", key);
			return DEF_PSAGSTAT_FAILED;
		}
		ResourceVertexAttrMap[key] = res;
		PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "vao: storage key: %u", key);
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagResVertexAttribOGL::ResourceDelete(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceVertexAttrMutex);

		auto it = ResourceVertexAttrMap.find(key);
		if (it != ResourceVertexAttrMap.end()) {
			// delete vao handle => clear map_item.
			glDeleteVertexArrays(1, &it->second);
			ResourceVertexAttrMap.erase(it);

			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "vao: delete key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "vao: failed delete, not found key.");
		return DEF_PSAGSTAT_FAILED;
	}

	// **************************************** FrameBuffer ****************************************

	PsagFrameBuffer PsagResFrameBufferOGL::ResourceFind(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceFrameBufferMutex);
#if PSAG_DEBUG_MODE
		if (ResourceFrameBufferMap.find(key) == ResourceFrameBufferMap.end())
			++GLOBAL_DEBUG_COUNT.ResourceFBO;
#endif
		return ResourceFrameBufferMap.find(key) == ResourceFrameBufferMap.end() ? 
			OPENGL_INVALID_HANDEL : ResourceFrameBufferMap[key];
	}

	bool PsagResFrameBufferOGL::ResourceStorage(ResUnique key, PsagGLmanagerFrameBuffer* res) {
		std::lock_guard<std::mutex> Lock(ResourceFrameBufferMutex);

		auto it = ResourceFrameBufferMap.find(key);
		if (it != ResourceFrameBufferMap.end()) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "fbo: failed storage duplicate_key: %u", key);
			return DEF_PSAGSTAT_FAILED;
		}
		ResourceFlag ResFlag = {};
		auto ResStorage = res->_MS_GETRES(ResFlag);

		// check resource = normal ?
		if (ResFlag == DEFRES_FLAG_NORMAL) {
			ResourceFrameBufferMap[key] = ResStorage;
			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "fbo: storage key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "fbo: failed storage, key: %u, code: %i", key, ResFlag);
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagResFrameBufferOGL::ResourceDelete(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceFrameBufferMutex);
		
		auto it = ResourceFrameBufferMap.find(key);
		if (it != ResourceFrameBufferMap.end()) {
			// delete fbo handle => clear map_item.
			glDeleteFramebuffers(1, &it->second);
			ResourceFrameBufferMap.erase(it);

			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "fbo: delete key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "fbo: failed delete, not found key.");
		return DEF_PSAGSTAT_FAILED;
	}

	// **************************************** RenderBuffer ****************************************

	PsagRenderBufferAttribute PsagResRenderBufferOGL::ResourceFind(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceRenderBufferMutex);
#if PSAG_DEBUG_MODE
		if (ResourceRenderBufferMap.find(key) == ResourceRenderBufferMap.end())
			++GLOBAL_DEBUG_COUNT.ResourceRBO;
#endif
		return ResourceRenderBufferMap.find(key) == ResourceRenderBufferMap.end() ? 
			PsagRenderBufferAttribute() : ResourceRenderBufferMap[key];
	}

	bool PsagResRenderBufferOGL::ResourceStorage(ResUnique key, PsagGLmanagerRenderBuffer* res) {
		std::lock_guard<std::mutex> Lock(ResourceRenderBufferMutex);

		auto it = ResourceRenderBufferMap.find(key);
		if (it != ResourceRenderBufferMap.end()) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "rbo: failed storage duplicate_key: %u", key);
			return DEF_PSAGSTAT_FAILED;
		}
		ResourceFlag ResFlag = {};
		auto ResStorage = res->_MS_GETRES(ResFlag);

		// check resource = normal ?
		if (ResFlag == DEFRES_FLAG_NORMAL) {
			ResourceRenderBufferMap[key] = ResStorage;
			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "rbo: storage key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "rbo: failed storage, key: %u, code: %i", key, ResFlag);
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagResRenderBufferOGL::ResourceDelete(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceRenderBufferMutex);

		auto it = ResourceRenderBufferMap.find(key);
		if (it != ResourceRenderBufferMap.end()) {
			// delete rbo handle => clear map_item.
			glDeleteFramebuffers(1, &it->second.RenderBufferSize);
			ResourceRenderBufferMap.erase(it);

			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "rbo: delete key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "rbo: failed delete, not found key.");
		return DEF_PSAGSTAT_FAILED;
	}

	// **************************************** UniformBuffer ****************************************

	PsagUniformBuffer PsagResUniformBufferOGL::ResourceFind(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceRenderBufferMutex);
#if PSAG_DEBUG_MODE
		if (ResourceRenderBufferMap.find(key) == ResourceRenderBufferMap.end())
			++GLOBAL_DEBUG_COUNT.ResourceUBO;
#endif
		return ResourceRenderBufferMap.find(key) == ResourceRenderBufferMap.end() ?
			PsagUniformBuffer() : ResourceRenderBufferMap[key];
	}

	bool PsagResUniformBufferOGL::ResourceStorage(ResUnique key, PsagGLmanagerUniformBuffer* res) {
		std::lock_guard<std::mutex> Lock(ResourceRenderBufferMutex);

		auto it = ResourceRenderBufferMap.find(key);
		if (it != ResourceRenderBufferMap.end()) {
			PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "ubo: failed storage duplicate_key: %u", key);
			return DEF_PSAGSTAT_FAILED;
		}
		ResourceFlag ResFlag = {};
		auto ResStorage = res->_MS_GETRES(ResFlag);

		// check resource = normal ?
		if (ResFlag == DEFRES_FLAG_NORMAL) {
			ResourceRenderBufferMap[key] = ResStorage;
			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "ubo: storage key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "ubo: failed storage, key: %u, code: %i", key, ResFlag);
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagResUniformBufferOGL::ResourceDelete(ResUnique key) {
		std::lock_guard<std::mutex> Lock(ResourceRenderBufferMutex);

		auto it = ResourceRenderBufferMap.find(key);
		if (it != ResourceRenderBufferMap.end()) {
			// delete ubo handle => clear map_item.
			glDeleteFramebuffers(1, &it->second);
			ResourceRenderBufferMap.erase(it);

			PsagLowLog(LogInfo, PSAG_OGLRES_LABEL, "ubo: delete key: %u", key);
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogWarning, PSAG_OGLRES_LABEL, "ubo: failed delete, not found key.");
		return DEF_PSAGSTAT_FAILED;
	}
}