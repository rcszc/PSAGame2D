// framework_vulkan_manager_tex.
#include "framework_vulkan.hpp"

using namespace std;

namespace RZ_VULKAN {
	RZ_VULKAN_DEFLABEL RZVK_MANAGER_TEXTURE = "rzvk_manager_texture";

	void CREATE_TEXTURE_VIEW::CreateTextureImage(
		uint32_t width, uint32_t height, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkImage* image, VkDeviceMemory* image_memory
	) {
		VkImageCreateInfo ImageCreate = {};

		ImageCreate.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

		ImageCreate.imageType     = VK_IMAGE_TYPE_2D;
		ImageCreate.extent.width  = width;
		ImageCreate.extent.height = height;
		ImageCreate.extent.depth  = 1;
		ImageCreate.mipLevels     = 1;
		ImageCreate.arrayLayers   = 1;
		ImageCreate.format        = format;
		ImageCreate.tiling        = tiling;
		ImageCreate.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageCreate.usage         = usage;
		ImageCreate.samples       = VK_SAMPLE_COUNT_1_BIT;
		ImageCreate.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

		ResultInfoGet(
			vkCreateImage(HANDLE_VK_DEVICE_LOGIC, &ImageCreate, nullptr, image),
			RzVkLog(LogInfo, LOG_TEXTURE_MODULE, "create %s_texture.", LOG_TXETURE_TYPE),
			RzVkLog(LogError, LOG_TEXTURE_MODULE, "failed create %s_texture.", LOG_TXETURE_TYPE)
		);

		VkMemoryRequirements MemRequirements = {};
		vkGetImageMemoryRequirements(HANDLE_VK_DEVICE_LOGIC, *image, &MemRequirements);

		VkMemoryAllocateInfo AllocConfig = {};

		AllocConfig.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocConfig.allocationSize  = MemRequirements.size;
		AllocConfig.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// create alloc bind texture_memory.
		ResultInfoGet(
			vkAllocateMemory(HANDLE_VK_DEVICE_LOGIC, &AllocConfig, nullptr, image_memory),
			RzVkLog(LogInfo, LOG_TEXTURE_MODULE, "create %s_texture memory: size: %u x %u", LOG_TXETURE_TYPE, width, height),
			RzVkLog(LogError, LOG_TEXTURE_MODULE, "failed create %s_texture memory.", LOG_TXETURE_TYPE)
		);
		ResultInfoGet(
			vkBindImageMemory(HANDLE_VK_DEVICE_LOGIC, *image, *image_memory, 0),
			RzVkLog(LogInfo, LOG_TEXTURE_MODULE, "%s_texture bind memory.", LOG_TXETURE_TYPE),
			RzVkLog(LogError, LOG_TEXTURE_MODULE, "failed %s_texture bind memory.", LOG_TXETURE_TYPE)
		);
	}

	void CREATE_TEXTURE_VIEW::CreateTextureImageView(
		VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageView* image_view
	) {
		VkImageViewCreateInfo ImageViewCreate = {};

		ImageViewCreate.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		ImageViewCreate.image    = image;
		ImageViewCreate.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreate.format   = format;

		ImageViewCreate.subresourceRange.aspectMask     = aspect;
		ImageViewCreate.subresourceRange.baseMipLevel   = 0;
		ImageViewCreate.subresourceRange.levelCount     = 1;
		ImageViewCreate.subresourceRange.baseArrayLayer = 0;
		ImageViewCreate.subresourceRange.layerCount     = 1;

		ResultInfoGet(
			vkCreateImageView(HANDLE_VK_DEVICE_LOGIC, &ImageViewCreate, nullptr, image_view),
			RzVkLog(LogInfo, LOG_TEXTURE_MODULE, "create %s_texture view.", LOG_TXETURE_TYPE),
			RzVkLog(LogError, LOG_TEXTURE_MODULE, "failed create %s_texture view.", LOG_TXETURE_TYPE)
		);
	}

	void VulkanCreateColorTexture::TextureSourceDataUpload(const RzVkImageSRC& load_image) {
		VkDeviceSize ImageDataSize = size_t(load_image.ImageSize.vector_x * load_image.ImageSize.vector_y * load_image.ImageChannels);

		LOG_NAME_MODULE     = RZVK_MANAGER_TEXTURE;
		LOG_NAME_BUFFER_TAG = "texture_upload_memory";

		CreateVulkanBufferMemory(
			ImageDataSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			TextureUploadBuffer, TextureUploadBufferMem
		);
		void* MemoryData = nullptr;

		// mapped gpu_memory, upload image_texture data. 
		vkMapMemory(HANDLE_VK_DEVICE_LOGIC, TextureUploadBufferMem, 0, ImageDataSize, 0, &MemoryData);
		memcpy(MemoryData, load_image.ImageSrcData.data(), ImageDataSize);
		vkUnmapMemory(HANDLE_VK_DEVICE_LOGIC, TextureUploadBufferMem);
	}

	void VulkanCreateColorTexture::TransitionImageLayout(
		VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout
	) {
		VkImageMemoryBarrier BarrierConfig = {};

		BarrierConfig.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		BarrierConfig.oldLayout = old_layout;
		BarrierConfig.newLayout = new_layout;

		BarrierConfig.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		BarrierConfig.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		BarrierConfig.srcAccessMask = 0;
		BarrierConfig.dstAccessMask = 0;

		BarrierConfig.image                           = image;
		BarrierConfig.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		BarrierConfig.subresourceRange.baseMipLevel   = 0;
		BarrierConfig.subresourceRange.levelCount     = 1;
		BarrierConfig.subresourceRange.baseArrayLayer = 0;
		BarrierConfig.subresourceRange.layerCount     = 1;

		VkPipelineStageFlags StageSource      = {};
		VkPipelineStageFlags StageDestination = {};

		// vulkan_doc: "未定义的 => 传输目标" & "传输目标 => 着色器读取".
		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			BarrierConfig.srcAccessMask = 0;
			BarrierConfig.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			StageSource      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			StageDestination = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			BarrierConfig.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			BarrierConfig.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			StageSource      = VK_PIPELINE_STAGE_TRANSFER_BIT;
			StageDestination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// pipeline barrier command.
		ContextSingleTimeCommandsBegin();
		vkCmdPipelineBarrier(GetCommandBuffer(), StageSource, StageDestination, 0, 0, nullptr, 0, nullptr, 1, &BarrierConfig);
		ContextSingleTimeCommandsEnd();
	}

	void VulkanCreateColorTexture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		ContextSingleTimeCommandsBegin();
		{
			VkBufferImageCopy RegionConfig = {};

			RegionConfig.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			RegionConfig.imageSubresource.mipLevel       = 0;
			RegionConfig.imageSubresource.baseArrayLayer = 0;
			RegionConfig.imageSubresource.layerCount     = 1;

			RegionConfig.bufferOffset      = 0;
			RegionConfig.bufferRowLength   = 0;
			RegionConfig.bufferImageHeight = 0;

			RegionConfig.imageOffset = { 0, 0, 0 };
			RegionConfig.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(GetCommandBuffer(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &RegionConfig);
		}
		ContextSingleTimeCommandsEnd();
	}

	bool VulkanCreateColorTexture::CreateImageTexture(const RzVkImageSRC& image_data, RZVK_SAMPLE_MODE sampler) {
		constexpr VkFormat IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
		// image_texture size > 32 x 32. [psag_std].
		if (image_data.ImageSize.vector_x < 32 || image_data.ImageSize.vector_y < 32) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_TEXTURE, "texture_image info: size < 32."));
			return false;
		}
		LOG_TXETURE_TYPE   = "color(image)";
		LOG_TEXTURE_MODULE = RZVK_MANAGER_TEXTURE;

		TextureSourceDataUpload(image_data);
		CreateTextureImage(
			image_data.ImageSize.vector_x, image_data.ImageSize.vector_y, 
			IMAGE_FORMAT, VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			&VulkanTextureImageTemp.ColorTexture, 
			&VulkanTextureImageTemp.TextureBufferMemory
		);

		// 上传布局 => 拷贝 => 着色器读取布局.
		TransitionImageLayout(
			VulkanTextureImageTemp.ColorTexture, IMAGE_FORMAT, 
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);
		CopyBufferToImage(
			TextureUploadBuffer, VulkanTextureImageTemp.ColorTexture, 
			image_data.ImageSize.vector_x, image_data.ImageSize.vector_y
		);
		TransitionImageLayout(
			VulkanTextureImageTemp.ColorTexture, IMAGE_FORMAT, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		// create texture_image view & sampler.
		CreateTextureImageView(
			VulkanTextureImageTemp.ColorTexture, IMAGE_FORMAT, 
			VK_IMAGE_ASPECT_COLOR_BIT, 
			&VulkanTextureImageTemp.ColorTextureView
		);

		// get framework preset sampler => texture.
		TextureSamplers::TextureSamplersManager GetSampler;
		auto TempSampler = GetSampler.GetPresetTextureSampler(sampler);
		if (TempSampler == VK_NULL_HANDLE) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_TEXTURE, "color_texture(image) info: invalid sampler."));
			return false;
		}
		// type framework preset_sampler.
		VulkanTextureImageTemp.ColorTextureSampler = TempSampler;

		// delete temp resource.
		vkDestroyBuffer(HANDLE_VK_DEVICE_LOGIC, TextureUploadBuffer, nullptr);
		vkFreeMemory(HANDLE_VK_DEVICE_LOGIC, TextureUploadBufferMem, nullptr);

		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_TEXTURE, "create color_texture(image) resource."));
		return true;
	}

	bool VulkanCreateColorTexture::CreateColorTexture(const FmtVector2T<uint32_t>& tex_size, RZVK_SAMPLE_MODE sampler) {
		constexpr VkFormat IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
		// image_texture size > 32 x 32. [psag_std].
		if (tex_size.vector_x < 32 || tex_size.vector_y < 32) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_TEXTURE, "color_texture info: size < 32."));
			return false;
		}
		LOG_TXETURE_TYPE   = "color";
		LOG_TEXTURE_MODULE = RZVK_MANAGER_TEXTURE;

		CreateTextureImage(
			tex_size.vector_x, tex_size.vector_y,
			IMAGE_FORMAT, VK_IMAGE_TILING_OPTIMAL,
			// shader_to_shader use.
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&VulkanTextureImageTemp.ColorTexture,
			&VulkanTextureImageTemp.TextureBufferMemory
		);

		TransitionImageLayout(
			VulkanTextureImageTemp.ColorTexture, IMAGE_FORMAT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		// create color_texture view & sampler.
		CreateTextureImageView(
			VulkanTextureImageTemp.ColorTexture, IMAGE_FORMAT, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			&VulkanTextureImageTemp.ColorTextureView
		);

		// get framework preset sampler => texture.
		TextureSamplers::TextureSamplersManager GetSampler;
		auto TempSampler = GetSampler.GetPresetTextureSampler(sampler);
		if (TempSampler == VK_NULL_HANDLE) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_TEXTURE, "color_texture info: invalid sampler."));
			return false;
		}
		// type framework preset_sampler.
		VulkanTextureImageTemp.ColorTextureSampler = TempSampler;

		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_TEXTURE, "create color_texture resource."));
		return true;
	}

	VulkanTextureColor VulkanCreateColorTexture::__MS_STORAGE(bool* storage_state) {
		// vk_image(ptr) | vk_image_view(ptr) | vk_buffer_memory(ptr) | vk_sampler(ptr) nullptr => storage res err.
		if (VulkanTextureImageTemp.TextureBufferMemory == VK_NULL_HANDLE || 
			VulkanTextureImageTemp.ColorTexture        == VK_NULL_HANDLE ||
			VulkanTextureImageTemp.ColorTextureSampler == VK_NULL_HANDLE ||
			VulkanTextureImageTemp.ColorTextureView    == VK_NULL_HANDLE
		) {
			*storage_state = false;
		}
		*storage_state = true;
		return VulkanTextureImageTemp;
	}

	RZ_VULKAN_DEFLABEL RZVK_MANAGER_TEXTURE_DEP = "rzvk_manager_texture_dep";

	VkFormat VulkanCreateDepthTexture::FindSupportedFormat(
		const vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features
	) {
		for (VkFormat FormatItem : candidates) {
			VkFormatProperties Properties = {};

			vkGetPhysicalDeviceFormatProperties(HANDLE_VK_DEVICE_PHYSICAL, FormatItem, &Properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (Properties.linearTilingFeatures & features) == features)
				return FormatItem;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (Properties.optimalTilingFeatures & features) == features)
				return FormatItem;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_TEXTURE_DEP, "texture_depth format not found."));
		return VK_FORMAT_UNDEFINED;
	}

	bool VulkanCreateDepthTexture::CreateDepthTexture(const FmtVector2T<uint32_t>& tex_size) {
		// depth_texture size > 128 x 128. [psag_std].
		if (tex_size.vector_x < 128 || tex_size.vector_y < 128) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_TEXTURE_DEP, "texture_depth info: size < 128."));
			return false;
		}
		VkFormat FindDepthFormat = FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
		// depth format check: 32bit-sfloat | 24bit-unsigned.
		// Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;

		LOG_TXETURE_TYPE   = "depth";
		LOG_TEXTURE_MODULE = RZVK_MANAGER_TEXTURE_DEP;

		CreateTextureImage(
			tex_size.vector_x, tex_size.vector_y, 
			FindDepthFormat, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			&VulkanTextureDepthTemp.DepthTexture, &VulkanTextureDepthTemp.TextureBufferMemory
		);
		CreateTextureImageView(
			VulkanTextureDepthTemp.DepthTexture, FindDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT,
			&VulkanTextureDepthTemp.DepthTextureView
		);
		VulkanTextureDepthTemp.DepthTextureFormat = FindDepthFormat;
		// ... 深度图像无需显示过度, 渲染通道处理.
		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_TEXTURE_DEP, "create texture_depth resource."));
		return true;
	}

	VulkanTextureDepth VulkanCreateDepthTexture::__MS_STORAGE(bool* storage_state) {
		// vk_image(ptr) | vk_image_view(ptr) | vk_buffer_memory(ptr) nullptr => storage res err.
		if (VulkanTextureDepthTemp.TextureBufferMemory == VK_NULL_HANDLE ||
			VulkanTextureDepthTemp.DepthTexture       == VK_NULL_HANDLE ||
			VulkanTextureDepthTemp.DepthTextureView   == VK_NULL_HANDLE
			) {
			*storage_state = false;
		}
		*storage_state = true;
		return VulkanTextureDepthTemp;
	}
}