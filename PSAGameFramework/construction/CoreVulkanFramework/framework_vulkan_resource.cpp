// framework_vulkan_resource.
#include "framework_vulkan.hpp"

using namespace std;
using namespace RZ_VULKAN;

namespace RZ_VULKAN_RESOURCE {
	template<typename T_MAP, typename T_OBJ>
	inline bool __RESOURCE_STORAGE(VKLRES_KEY* key, const char* tag, atomic<VKLRES_KEY>* res_counter, T_MAP& res_map, T_OBJ res_storage) {
		// get object storage_res, get storage_res flag.
		bool ResourceState = false;
		auto ResStorage = res_storage->__MS_STORAGE(&ResourceState);
		// resource is valid ?
		if (ResourceState) {
			*key = ++(*res_counter);
			res_map[*key] = ResStorage;
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(%s): storage key: %u", tag, *key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(%s): failed storage.", tag));
		return false;
	}

	VulkanVertexAttribute ResourceVkVertexAttribute::ResourceFind(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		return ResourceHashMap.find(key) == ResourceHashMap.end() ? VulkanVertexAttribute() : ResourceHashMap[key];
	}

	bool ResourceVkVertexAttribute::ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceVA* res) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// resource storage: vertex_attribute.
		return __RESOURCE_STORAGE(key, "va", &ResourceCounter, ResourceHashMap, res);
	}

	bool ResourceVkVertexAttribute::ResourceDelete(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// find key ? => erase item.
		auto it = ResourceHashMap.find(key);
		if (it != ResourceHashMap.end()) {
			// vertex_attribute not_handle, clear map_item.
			ResourceHashMap.erase(it);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(va): delete key: %u", key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(va): failed delete, not found key."));
		return false;
	}

	VulkanVertexBuffer ResourceVkVertexBuffer::ResourceFind(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		return ResourceHashMap.find(key) == ResourceHashMap.end() ? VulkanVertexBuffer() : ResourceHashMap[key];
	}

	bool ResourceVkVertexBuffer::ResourceStorage(VKLRES_KEY* key, CreateInterfaceVB* res) {
		// resource storage: vertex_buffer.
		return __RESOURCE_STORAGE(key, "vb", &ResourceCounter, ResourceHashMap, res);
	}

	bool ResourceVkVertexBuffer::ResourceDelete(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// find key ? => erase item.
		auto it = ResourceHashMap.find(key);
		if (it != ResourceHashMap.end()) {
			// vertex_buffer: free_buffer => free_device_mem => erase_item.
			DeleteVkBuffer(it->second.VertexBuffer);
			DeleteVkDeviceMemory(it->second.VertexBufferMemory);

			ResourceHashMap.erase(it);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(vb): delete key: %u", key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(vb): failed delete, not found key."));
		return false;
	}

	VulkanUniformBuffer ResourceVkUniformBuffer::ResourceFind(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		return ResourceHashMap.find(key) == ResourceHashMap.end() ? VulkanUniformBuffer() : ResourceHashMap[key];
	}

	bool ResourceVkUniformBuffer::ResourceStorage(VKLRES_KEY* key, CreateInterfaceUB* res) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// resource storage: uniform_buffer.
		return __RESOURCE_STORAGE(key, "ub", &ResourceCounter, ResourceHashMap, res);
	}

	bool ResourceVkUniformBuffer::ResourceDelete(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// find key ? => erase item.
		auto it = ResourceHashMap.find(key);
		if (it != ResourceHashMap.end()) {
			// uniform_buffer: unmap_mem => free_buffer => free_device_mem => erase_item.
			UnmapVkDeviceMemory(it->second.UniformBufferMemory);
			DeleteVkBuffer(it->second.UniformBuffer);
			DeleteVkDeviceMemory(it->second.UniformBufferMemory);

			ResourceHashMap.erase(it);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(ub): delete key: %u", key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(ub): failed delete, not found key."));
		return false;
	}

	VulkanShaderLayout ResourceVkShaderLayout::ResourceFind(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		return ResourceHashMap.find(key) == ResourceHashMap.end() ? VulkanShaderLayout() : ResourceHashMap[key];
	}

	bool ResourceVkShaderLayout::ResourceStorage(VKLRES_KEY* key, CreateInterfaceSL* res) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// resource storage: shader(pipeline)_layout.
		return __RESOURCE_STORAGE(key, "sl", &ResourceCounter, ResourceHashMap, res);
	}

	bool ResourceVkShaderLayout::ResourceDelete(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// find key ? => erase item.
		auto it = ResourceHashMap.find(key);
		if (it != ResourceHashMap.end()) {
			// shader(pipeline)_layout: free_desc_set_layout => free_shader_layout => erase_item.
			DeleteVkDescriptorSetLayout(it->second.LayoutDescriptorSet);
			DeleteVkPipelineLayout(it->second.LayoutPipeline);

			ResourceHashMap.erase(it);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(sl): delete key: %u", key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(sl): failed delete, not found key."));
		return false;
	}

	VulkanShader ResourceVkShader::ResourceFind(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		return ResourceHashMap.find(key) == ResourceHashMap.end() ? VulkanShader() : ResourceHashMap[key];
	}

	bool ResourceVkShader::ResourceStorage(VKLRES_KEY* key, CreateInterfaceSP* res) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// resource storage: shader(pipeline).
		return __RESOURCE_STORAGE(key, "sp", &ResourceCounter, ResourceHashMap, res);
	}

	bool ResourceVkShader::ResourceDelete(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// find key ? => erase item.
		auto it = ResourceHashMap.find(key);
		if (it != ResourceHashMap.end()) {
			// shader(pipeline): free_desc_set_layout => free_shader_layout => erase_item.
				// "item.second.ShaderDescriptorSet" => desc_pool.
			DeleteVkPipeline(it->second.ShaderPipeline);

			ResourceHashMap.erase(it);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(sp): delete key: %u", key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(sp): failed delete, not found key."));
		return false;
	}

	VulkanTextureColor ResourceVkColorTexture::ResourceFind(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		return ResourceHashMap.find(key) == ResourceHashMap.end() ? VulkanTextureColor() : ResourceHashMap[key];
	}

	bool ResourceVkColorTexture::ResourceStorage(VKLRES_KEY* key, CreateInterfaceCT* res) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// resource storage: texture_image.
		return __RESOURCE_STORAGE(key, "ct", &ResourceCounter, ResourceHashMap, res);
	}

	bool ResourceVkColorTexture::ResourceDelete(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// find key ? => erase item.
		auto it = ResourceHashMap.find(key);
		if (it != ResourceHashMap.end()) {
			// texture_image: free_texview => free_tex => free_device_mem => erase_item.
			// sampler: manager_free.
			DeleteVkTextureImageView(it->second.ColorTextureView);
			DeleteVkTextureImage(it->second.ColorTexture);
			DeleteVkDeviceMemory(it->second.TextureBufferMemory);

			ResourceHashMap.erase(it);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(ct): delete key: %u", key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(ct): failed delete, not found key."));
		return false;
	}

	VulkanTextureDepth ResourceVkDepthTexture::ResourceFind(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		return ResourceHashMap.find(key) == ResourceHashMap.end() ? VulkanTextureDepth() : ResourceHashMap[key];
	}

	bool ResourceVkDepthTexture::ResourceStorage(VKLRES_KEY* key, CreateInterfaceDT* res) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// resource storage: texture_depth.
		return __RESOURCE_STORAGE(key, "dt", &ResourceCounter, ResourceHashMap, res);
	}

	bool ResourceVkDepthTexture::ResourceDelete(VKLRES_KEY key) {
		lock_guard<mutex> Lock(ResourceThreadMutex);
		// find key ? => erase item.
		auto it = ResourceHashMap.find(key);
		if (it != ResourceHashMap.end()) {
			// texture_depthe: free_texview => free_tex => free_device_mem => erase_item.
			// sampler: manager_free.
			DeleteVkTextureImageView(it->second.DepthTextureView);
			DeleteVkTextureImage(it->second.DepthTexture);
			DeleteVkDeviceMemory(it->second.TextureBufferMemory);

			ResourceHashMap.erase(it);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_RESOURCE_TAG, "res_data(dt): delete key: %u", key));
			return true;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_RESOURCE_TAG, "res_data(dt): failed delete, not found key."));
		return false;
	}
}