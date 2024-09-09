// framework_vulkan_manager.
#include "framework_vulkan.hpp"

using namespace std;

namespace RZ_VULKAN {
	namespace RenderPass {
		RZ_VULKAN_DEFLABEL RZVK_MANAGER_RENDERPASS = "rzvk_manager_render_pass";

		RenderPassManger::~RenderPassManger() {
			// free vulkan render_pass.
			vkDestroyRenderPass(HANDLE_VK_DEVICE_LOGIC, VulkanRenderPass, nullptr);
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_RENDERPASS, "delete render_pass(manager): name: %s", RenderPassTagName));
			// sub render_pass objects counter.
			--ObjectsRenderPass;
		}

		void RenderPassManger::CreateDepthAttachment(VkFormat dep_format, const FmtVector2T<float>& clear) {
			VkAttachmentDescription ColorAttachment = {};
			VkClearValue            ClearAttachment = {};

			ClearAttachment.depthStencil.depth   = clear.vector_x;
			ClearAttachment.depthStencil.stencil = (uint32_t)clear.vector_y;

			ColorAttachment.format  = dep_format;
			ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

			ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			ColorAttachment.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			ColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
			ColorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
			ColorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			AttachmentsClear.AttachmentClearValues.push_back(ClearAttachment);
			DatasetAttachments.push_back(ColorAttachment);
			DepthAttachmentIndex = DatasetAttachments.size() - 1;

			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_RENDERPASS, "enable render_pass(manager): depth_attachment."));
		}

		size_t RenderPassManger::PushCreateAttachment(VkFormat img_format, const FmtVector4T<float>& clear) {
			VkAttachmentDescription ColorAttachment = {};
			VkClearValue            ClearAttachment = {};

			memcpy(ClearAttachment.color.float32, clear.data(), sizeof(float) * 4);

			ColorAttachment.format  = img_format;
			ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

			ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			ColorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			ColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
			ColorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
			ColorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			AttachmentsClear.AttachmentClearValues.push_back(ClearAttachment);
			DatasetAttachments.push_back(ColorAttachment);
			return DatasetAttachments.size() - 1;
		}

		size_t RenderPassManger::PushCreateSubRenderPass(size_t attchment_index) {
			VkAttachmentReference AttachmentReference = {};
			
			AttachmentReference.attachment = (uint32_t)attchment_index;
			AttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			DatasetAttachmentsRef.push_back(AttachmentReference);
			VkSubpassDescription SubRenderPass = {};

			SubRenderPass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
			SubRenderPass.colorAttachmentCount = 1;
			SubRenderPass.pColorAttachments    = &DatasetAttachmentsRef.back();

			if (DepthAttachmentIndex < SIZE_MAX) {
				// depth_index valid => config depth_ref. 
				DepthAttachmentRef.attachment = (uint32_t)DepthAttachmentIndex;
				DepthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				SubRenderPass.pDepthStencilAttachment = &DepthAttachmentRef;
			}
			DatasetSubRenderPass.push_back(SubRenderPass);
			return DatasetSubRenderPass.size() - 1;
		}

		size_t RenderPassManger::PushCreateSubDependency() {
			VkSubpassDependency SubPassDependency = {};

			SubPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			SubPassDependency.dstSubpass = 0;

			SubPassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			SubPassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			SubPassDependency.srcAccessMask = 0;
			SubPassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// depth and color attchment dependency.
			if (DepthAttachmentIndex < SIZE_MAX) {
				SubPassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				SubPassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				SubPassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				SubPassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}
			DatasetSubDependency.push_back(SubPassDependency);
			return DatasetSubDependency.size() - 1;
		}

		void RenderPassManger::CreateRenderPass(const char* tag_name) {
			VkRenderPassCreateInfo RenderPassCreate = {};

			RenderPassCreate.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			RenderPassCreate.attachmentCount = (uint32_t)DatasetAttachments.size();
			RenderPassCreate.pAttachments    = DatasetAttachments.data();
			RenderPassCreate.subpassCount    = (uint32_t)DatasetSubRenderPass.size();
			RenderPassCreate.pSubpasses      = DatasetSubRenderPass.data();

			RenderPassCreate.dependencyCount = (uint32_t)DatasetSubDependency.size();
			RenderPassCreate.pDependencies   = DatasetSubDependency.data();

			ResultInfoGet(
				vkCreateRenderPass(HANDLE_VK_DEVICE_LOGIC, &RenderPassCreate, nullptr, &VulkanRenderPass),
				RzVkLog(LogInfo, RZVK_MANAGER_RENDERPASS, "create render_pass(manager): sub: %u items.", DatasetSubRenderPass.size()),
				RzVkLog(LogError, RZVK_MANAGER_RENDERPASS, "failed create render_pass(manager).")
			);
			RenderPassTagName = tag_name;
		}

		VulkanRenderPassData RenderPassManger::GetRenderPassData() {
			return VulkanRenderPassData((uint32_t)DatasetSubRenderPass.size(), VulkanRenderPass);
		}
		VulkanClearValues RenderPassManger::GetRenderPassClearValues() {
			return AttachmentsClear;
		}
	}

	namespace RenderCommand {
		RZ_VULKAN_DEFLABEL RZVK_MANAGER_COMMAND = "rzvk_manager_render_command";

		RenderCommandManager::~RenderCommandManager() {
			vkDeviceWaitIdle(HANDLE_VK_DEVICE_LOGIC);
			// free_number constant: 1(manager_this).
			vkFreeCommandBuffers(HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_POOL_COMMAND, 1, &VulkanCommandBuffer);
			RZVK_FUNC_LOGGER(
				RzVkLog(LogInfo, RZVK_MANAGER_COMMAND, "delete vulkan command_buffer(manager): name: %s", CommandBufferTagName)
			);
			// sub render_command objects counter.
			--ObjectsRenderCommand;
		}

		void RenderCommandManager::ClearCommand() {
			vkResetCommandBuffer(VulkanCommandBuffer, NULL);
		}

		bool RenderCommandManager::RecordCommandBegin() {
			VkCommandBufferBeginInfo CommandBeginCreate = {};

			CommandBeginCreate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			CommandBeginCreate.flags = 0;

			CommandBeginCreate.pInheritanceInfo = nullptr;
			// dynamic_recording(begin): non-print_log => return state.
			return vkBeginCommandBuffer(VulkanCommandBuffer, &CommandBeginCreate) != VK_SUCCESS;
		}

		bool RenderCommandManager::RecordCommandEnd() {
			// dynamic_recording(end): non-print_log => return state.
			return vkEndCommandBuffer(VulkanCommandBuffer) != VK_SUCCESS;
		}

		void RenderCommandManager::CreateRenderCommandBuffer(const char* tag_name) {
			VkCommandBufferAllocateInfo CommandBufferCreate = {};

			CommandBufferCreate.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			CommandBufferCreate.commandPool        = HANDLE_VK_POOL_COMMAND;
			CommandBufferCreate.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			CommandBufferCreate.commandBufferCount = 1;

			ResultInfoGet(
				vkAllocateCommandBuffers(HANDLE_VK_DEVICE_LOGIC, &CommandBufferCreate, &VulkanCommandBuffer),
				RzVkLog(LogInfo, RZVK_MANAGER_COMMAND, "create vulkan command_buffer(manager)."),
				RzVkLog(LogError, RZVK_MANAGER_COMMAND, "failed create vulkan command_buffer(manager).")
			);
			CommandBufferTagName = tag_name;
		}

		bool RenderCommandManager::SubmitRenderCommandBuffer(const CommandSubmit& params) {
			VkSubmitInfo SubmitParams = params.CommandSubmitConfig;

			SubmitParams.commandBufferCount = 1;
			SubmitParams.pCommandBuffers    = &VulkanCommandBuffer;

			return vkQueueSubmit(HANDLE_VK_QUEUE_GRAPH, 1, &SubmitParams, params.CommandSubmitFence) != VK_SUCCESS;
		}

		void RenderCommandManager::CommandGraphDraw(uint32_t ver, uint32_t off_ver, uint32_t inst, uint32_t off_inst) {
			vkCmdDraw(VulkanCommandBuffer, ver, inst, off_ver, off_inst);
		}

		void RenderCommandManager::CommandBindShader(VkPipeline shader) {
			vkCmdBindPipeline(VulkanCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader);
		}

		void RenderCommandManager::CommandBindDescriptorSet(VkDescriptorSet desc, VkPipelineLayout layout) {
			vkCmdBindDescriptorSets(VulkanCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &desc, 0, nullptr);
		}

		void RenderCommandManager::CommandBindVertexBuffer(const CMDparams_VertexBufferBind& params) {
			vkCmdBindVertexBuffers(
				VulkanCommandBuffer, params.BindingFirst, params.BindingCount,
				&params.UniformBuffer, &params.BufferOffsetBytes
			);
		}

		void RenderCommandManager::CommandViewportScissor(const VkViewport& viewport) {
			VkRect2D Scissor = {};

			Scissor.offset = { (int32_t)viewport.x,      (int32_t)viewport.y       };
			Scissor.extent = { (uint32_t)viewport.width, (uint32_t)viewport.height };

			vkCmdSetViewport(VulkanCommandBuffer, 0, 1, &viewport);
			vkCmdSetScissor (VulkanCommandBuffer, 0, 1, &Scissor);
		}

		void RenderCommandManager::CommandBeginRenderPass(const CMDparams_BeginRenderPass& params) {
			VkRenderPassBeginInfo RenderPassBeginCreate = {};

			RenderPassBeginCreate.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			RenderPassBeginCreate.renderPass  = params.RenderPass;
			RenderPassBeginCreate.framebuffer = params.RenderPassBuffer;

			VkOffset2D OFFSET = { (int32_t)params.RenderRangeSize.BeginCoord.vector_x, (int32_t)params.RenderRangeSize.BeginCoord.vector_y };
			VkExtent2D EXTENT = { (uint32_t)params.RenderRangeSize.EndCoord.vector_x, (uint32_t)params.RenderRangeSize.EndCoord.vector_y };

			RenderPassBeginCreate.renderArea.offset = OFFSET;
			RenderPassBeginCreate.renderArea.extent = EXTENT;

			RenderPassBeginCreate.clearValueCount = (uint32_t)params.ClearValues.AttachmentClearValues.size();
			RenderPassBeginCreate.pClearValues    = params.ClearValues.AttachmentClearValues.data();

			vkCmdBeginRenderPass(VulkanCommandBuffer, &RenderPassBeginCreate, VK_SUBPASS_CONTENTS_INLINE);
		}

		void RenderCommandManager::CommandEndRenderPass() {
			vkCmdEndRenderPass(VulkanCommandBuffer);
		}

		void RenderCommandManager::CommandConvertPresent(VkImage image) {
			VkImageMemoryBarrier BarrierConfig = {};

			BarrierConfig.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

			BarrierConfig.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			BarrierConfig.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			BarrierConfig.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			BarrierConfig.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			BarrierConfig.image               = image; // convert image.

			BarrierConfig.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			BarrierConfig.subresourceRange.baseMipLevel   = 0;
			BarrierConfig.subresourceRange.levelCount     = 1;
			BarrierConfig.subresourceRange.baseArrayLayer = 0;
			BarrierConfig.subresourceRange.layerCount     = 1;

			BarrierConfig.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			BarrierConfig.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			constexpr VkPipelineStageFlagBits Flags[2] = { 
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT 
			};
			vkCmdPipelineBarrier(VulkanCommandBuffer, Flags[0], Flags[1], 0, 0, nullptr, 0, nullptr, 1, &BarrierConfig);
		}
	}

	namespace TextureSamplers {
		RZ_VULKAN_DEFLABEL RZVK_MANAGER_SAMPLERS = "rzvk_manager_samplers";

		vector<VkSampler> TextureSamplersManager::FrameworkConstSampler      = {};
		mutex             TextureSamplersManager::FrameworkConstSamplerMutex = {};

		void TextureSamplersManager::CreateTextureSampler(VkSampler* sampler, VkFilter filter, VkSamplerAddressMode mode) {
			VkSamplerCreateInfo TexSamplerCreate = {};

			TexSamplerCreate.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

			TexSamplerCreate.magFilter = filter;
			TexSamplerCreate.minFilter = filter;

			TexSamplerCreate.addressModeU = mode;
			TexSamplerCreate.addressModeV = mode;
			TexSamplerCreate.addressModeW = mode;

			VkPhysicalDeviceProperties Properties = {};
			vkGetPhysicalDeviceProperties(HANDLE_VK_DEVICE_PHYSICAL, &Properties);

			TexSamplerCreate.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			TexSamplerCreate.mipLodBias = 0.0f;
			TexSamplerCreate.minLod     = 0.0f;
			TexSamplerCreate.maxLod     = 0.0f;

			TexSamplerCreate.anisotropyEnable        = VK_TRUE;
			TexSamplerCreate.maxAnisotropy           = Properties.limits.maxSamplerAnisotropy;
			TexSamplerCreate.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			TexSamplerCreate.unnormalizedCoordinates = VK_FALSE; // pixel_coord sample.

			TexSamplerCreate.compareEnable = VK_FALSE;
			TexSamplerCreate.compareOp     = VK_COMPARE_OP_ALWAYS;

			uint32_t MCODE = (uint32_t)mode;
			ResultInfoGet(
				vkCreateSampler(HANDLE_VK_DEVICE_LOGIC, &TexSamplerCreate, nullptr, sampler),
				RzVkLog(LogInfo, RZVK_MANAGER_SAMPLERS, "create texture_sampler, mode_name: %s", RZVK_SAMPLE_MODENAME[MCODE]),
				RzVkLog(LogError, RZVK_MANAGER_SAMPLERS, "failed create texture_sampler, mode_name: %s", RZVK_SAMPLE_MODENAME[MCODE])
			);
		}

		void TextureSamplersManager::VKFC_TextureSamplersCreate() {
			lock_guard<mutex> Lock(FrameworkConstSamplerMutex);

			size_t SmpModeBegin = (size_t)VK_SAMPLER_ADDRESS_MODE_REPEAT;
			size_t SmpModeEnd   = (size_t)VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

			// FILTER: VK_FILTER_NEAREST. MODE: VK: REPEAT - CLAMP_TO_BORDER.
			for (size_t i = SmpModeBegin; i <= SmpModeEnd; ++i) {
				VkSampler VulkanSamplerTemp = VK_NULL_HANDLE;

				// create perset texture_sampler.
				CreateTextureSampler(&VulkanSamplerTemp, VK_FILTER_NEAREST, (VkSamplerAddressMode)i);
				if (VulkanSamplerTemp == VK_NULL_HANDLE)
					continue;
				FrameworkConstSampler.push_back(VulkanSamplerTemp);
			}
			RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_MANAGER_SAMPLERS, "create texture_samplers filter: nearest."));

			// FILTER: VK_FILTER_LINEAR. MODE: VK: REPEAT - CLAMP_TO_BORDER.
			for (size_t i = SmpModeBegin; i <= SmpModeEnd; ++i) {
				VkSampler VulkanSamplerTemp = VK_NULL_HANDLE;

				// create perset texture_sampler.
				CreateTextureSampler(&VulkanSamplerTemp, VK_FILTER_LINEAR, (VkSamplerAddressMode)i);
				if (VulkanSamplerTemp == VK_NULL_HANDLE)
					continue;
				FrameworkConstSampler.push_back(VulkanSamplerTemp);
			}
			RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_MANAGER_SAMPLERS, "create texture_samplers filter: linear."));
		}

		void TextureSamplersManager::VKFC_TextureSamplersDelete() {
			lock_guard<mutex> Lock(FrameworkConstSamplerMutex);

			for (auto SamplerItem : FrameworkConstSampler) {
				vkDestroySampler(HANDLE_VK_DEVICE_LOGIC, SamplerItem, nullptr);
				SamplerItem = VK_NULL_HANDLE;
			}
			RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_MANAGER_SAMPLERS, "delete texture_samplers nearest & linear."));
		}

		VkSampler TextureSamplersManager::GetPresetTextureSampler(RZVK_SAMPLE_MODE mode) {
			lock_guard<mutex> Lock(FrameworkConstSamplerMutex);
			return FrameworkConstSampler[(size_t)mode];
		}
	}

	// ############################################ Vertex Attribute ############################################
	RZ_VULKAN_DEFLABEL RZVK_MANAGER_ATTRIB = "rzvk_manager_attribute";

	size_t VulkanCreateVertexAttribute::PushSettingAttribute(VulkanVectorType type, const char* tag_name) {
		VkVertexInputAttributeDescription AttribDescription = {};
		VkFormat AttribFormat = {};

		// types: VEC1-FP32, VEC2-FP32, VEC3-FP32, VEC4-FP32.
		// update: 2024_08_30. RCSZ.
		switch (type) {
		case(AttributeVector1): { AttribFormat = VK_FORMAT_R32_SFLOAT;          break; }
		case(AttributeVector2): { AttribFormat = VK_FORMAT_R32G32_SFLOAT;       break; }
		case(AttributeVector3): { AttribFormat = VK_FORMAT_R32G32B32_SFLOAT;    break; }
		case(AttributeVector4): { AttribFormat = VK_FORMAT_R32G32B32A32_SFLOAT; break; }
		}
		AttribDescription.binding  = VertexBufferBinding;
		AttribDescription.location = VertexLocationCount;
		AttribDescription.format   = AttribFormat;
		AttribDescription.offset   = VertexBytesCount;

		++VertexLocationCount;
		VertexBytesCount += (uint32_t)type;

		VertexAttrubutesTemp.AttributeDescriptions.push_back(AttribDescription);
		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_ATTRIB, "push attribute: begin: %u, size: %u, tag: %s", 
			VertexBytesCount, (uint32_t)type, tag_name
		));
		return VertexAttrubutesTemp.AttributeDescriptions.size();
	}

	void VulkanCreateVertexAttribute::CreateAttributes(bool non_attrib) {
		// config params vulkan: "VkPipelineVertexInputStateCreateInfo". 
		uint32_t AttributesSize = (uint32_t)VertexAttrubutesTemp.AttributeDescriptions.size();
		auto     AttributesPtr  = VertexAttrubutesTemp.AttributeDescriptions.data();

		VertexAttrubutesTemp.AttributeBindingBuffer.binding   = VertexBufferBinding;
		VertexAttrubutesTemp.AttributeBindingBuffer.stride    = VertexBytesCount;
		VertexAttrubutesTemp.AttributeBindingBuffer.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VertexAttrubutesTemp.PiplineVertexAttribute.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		if (!non_attrib) {
			VertexAttrubutesTemp.PiplineVertexAttribute.vertexBindingDescriptionCount   = 1;
			VertexAttrubutesTemp.PiplineVertexAttribute.pVertexBindingDescriptions      = &VertexAttrubutesTemp.AttributeBindingBuffer;
			VertexAttrubutesTemp.PiplineVertexAttribute.vertexAttributeDescriptionCount = AttributesSize;
			VertexAttrubutesTemp.PiplineVertexAttribute.pVertexAttributeDescriptions    = AttributesPtr;

			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_ATTRIB, "vertex attribute binding: %u", VertexBufferBinding));
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_ATTRIB, "create vertex attribute."));
			return;
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_ATTRIB, "create vertex empty_attribute."));
	}

	VulkanVertexAttribute VulkanCreateVertexAttribute::__MS_STORAGE(bool* storage_state) {
		// non.attribute_descriptions => storage res err.
		if (VertexAttrubutesTemp.AttributeDescriptions.empty()) {
			*storage_state = false;
		}
		*storage_state = true;
		return VertexAttrubutesTemp;
	}

	// ############################################ Buffer Memory ############################################

	uint32_t CREATE_BUFFER_MEMORY::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties MemProperties = {};
		vkGetPhysicalDeviceMemoryProperties(HANDLE_VK_DEVICE_PHYSICAL, &MemProperties);
		// find memory type_count.
		for (uint32_t i = 0; i < MemProperties.memoryTypeCount; i++)
			if ((type_filter & (1 << i)) && (MemProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		RZVK_FUNC_LOGGER(RzVkLog(LogError, LOG_NAME_MODULE, "failed find suitable memory_type."));
		return NULL;
	}

	void CREATE_BUFFER_MEMORY::CreateVulkanBufferMemory(
		VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& buffer_memory
	) {
		// create buffer => get buffer_memory requirements => alloc => bind.
		VkBufferCreateInfo BufferCraete = {};

		BufferCraete.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCraete.size        = size;
		BufferCraete.usage       = usage;
		BufferCraete.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		ResultInfoGet(
			vkCreateBuffer(HANDLE_VK_DEVICE_LOGIC, &BufferCraete, nullptr, &buffer),
			RzVkLog(LogInfo, LOG_NAME_MODULE, "buffer(create): %s.", LOG_NAME_BUFFER_TAG),
			RzVkLog(LogError, LOG_NAME_MODULE, "failed buffer(create): %s.", LOG_NAME_BUFFER_TAG)
		);

		VkMemoryRequirements MemRequirements = {};
		// 获取设备(logic_device)内存要求信息.
		vkGetBufferMemoryRequirements(HANDLE_VK_DEVICE_LOGIC, buffer, &MemRequirements);

		VkMemoryAllocateInfo MemoryAllocCreate = {};
		
		MemoryAllocCreate.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		MemoryAllocCreate.allocationSize  = MemRequirements.size;
		MemoryAllocCreate.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, properties);

		// allocate_memory => bind_memory.
		ResultInfoGet(
			vkAllocateMemory(HANDLE_VK_DEVICE_LOGIC, &MemoryAllocCreate, nullptr, &buffer_memory),
			RzVkLog(LogInfo, LOG_NAME_MODULE, "buffer(allocate): %s memory: %u bytes.", LOG_NAME_BUFFER_TAG, size),
			RzVkLog(LogError, LOG_NAME_MODULE, "failed buffer(allocate): %s memory.", LOG_NAME_BUFFER_TAG)
		);
		ResultInfoGet(
			vkBindBufferMemory(HANDLE_VK_DEVICE_LOGIC, buffer, buffer_memory, 0),
			RzVkLog(LogInfo, LOG_NAME_MODULE, "buffer(bind): %s memory.", LOG_NAME_BUFFER_TAG),
			RzVkLog(LogError, LOG_NAME_MODULE, "failed buffer(bind): %s memory.", LOG_NAME_BUFFER_TAG)
		);
	}

	// ############################################ SingleTime Commands ############################################
	
	void CREATE_SINGLE_TIME_COMMAND::ContextSingleTimeCommandsBegin() {
		VkCommandBufferAllocateInfo AllocCommandBuffer = {};

		AllocCommandBuffer.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocCommandBuffer.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocCommandBuffer.commandPool = HANDLE_VK_POOL_COMMAND;

		AllocCommandBuffer.commandBufferCount = 1;
		// alloc command_buffer: non-check_result.
		vkAllocateCommandBuffers(HANDLE_VK_DEVICE_LOGIC, &AllocCommandBuffer, &SingleTimeCommandBuffer);

		VkCommandBufferBeginInfo BeginConifg = {};

		BeginConifg.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginConifg.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		// begin command_buffer: non-check_result.
		vkBeginCommandBuffer(SingleTimeCommandBuffer, &BeginConifg);
	}

	void CREATE_SINGLE_TIME_COMMAND::ContextSingleTimeCommandsEnd() {
		VkSubmitInfo SubmitCmdConfig = {};
		// end command_buffer: non-check_result.
		vkEndCommandBuffer(SingleTimeCommandBuffer);

		SubmitCmdConfig.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitCmdConfig.commandBufferCount = 1;
		SubmitCmdConfig.pCommandBuffers    = &SingleTimeCommandBuffer;

		vkQueueSubmit(HANDLE_VK_QUEUE_GRAPH, 1, &SubmitCmdConfig, VK_NULL_HANDLE);
		vkQueueWaitIdle(HANDLE_VK_QUEUE_GRAPH);

		// delete command_buffer: non-check_result.
		vkFreeCommandBuffers(HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_POOL_COMMAND, 1, &SingleTimeCommandBuffer);
	}

	// ############################################ Vertex Buffer ############################################
	RZ_VULKAN_DEFLABEL RZVK_MANAGER_VBO = "rzvk_manager_vbo";

	bool VulkanCreateVertexBuffer::CreateVertexBuffer(const vector<float>& vertex_data, uint32_t binding) {
		if (vertex_data.empty()) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_VBO, "vertex_data err: data_array empty."));
			return false;
		}
		LOG_NAME_MODULE     = RZVK_MANAGER_VBO;
		LOG_NAME_BUFFER_TAG = "vertex_buffer";

		// create memory => upload vert_data.
		void*  MemoryData      = nullptr;
		size_t MemoryDataBytes = vertex_data.size() * sizeof(float);

		CreateVulkanBufferMemory(
			MemoryDataBytes,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			VulkanVertexBufferTemp.VertexBuffer, VulkanVertexBufferTemp.VertexBufferMemory
		);
		// mapped gpu_memory.
		vkMapMemory(HANDLE_VK_DEVICE_LOGIC, VulkanVertexBufferTemp.VertexBufferMemory, 0, MemoryDataBytes, 0, &MemoryData);
		// map_memory =cpu=> copy_memory data =cpu=> unmap_memory => gpu.
		memcpy(MemoryData, vertex_data.data(), MemoryDataBytes);
		vkUnmapMemory(HANDLE_VK_DEVICE_LOGIC, VulkanVertexBufferTemp.VertexBufferMemory);

		VulkanVertexBufferTemp.VertexBufferBinding = binding;
		return true;
	}

	VulkanVertexBuffer VulkanCreateVertexBuffer::__MS_STORAGE(bool* storage_state) {
		// vk_vertex_buffer(ptr) | vk_vertex_buffer_memory(ptr) nullptr => storage res err.
		if (VulkanVertexBufferTemp.VertexBuffer == VK_NULL_HANDLE || VulkanVertexBufferTemp.VertexBufferMemory == VK_NULL_HANDLE) {
			*storage_state = false;
		}
		*storage_state = true;
		return VulkanVertexBufferTemp;
	}

	// ############################################ Uniform Buffer ############################################
	RZ_VULKAN_DEFLABEL RZVK_MANAGER_UBO = "rzvk_manager_ubo";

	bool VulkanCreateUniformBuffer::CreateUniformBuffer() {
		if (UniformMemoryBytes == 0) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_UBO, "uniform_data err: struct_bytes = 0."));
			return false;
		}
		LOG_NAME_MODULE     = RZVK_MANAGER_UBO;
		LOG_NAME_BUFFER_TAG = "uniform_buffer";
		
		CreateVulkanBufferMemory(
			UniformMemoryBytes,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			VulkanUniformBufferTemp.UniformBuffer,
			VulkanUniformBufferTemp.UniformBufferMemory
		);
		// mapped gpu_memory.
		vkMapMemory(
			HANDLE_VK_DEVICE_LOGIC, VulkanUniformBufferTemp.UniformBufferMemory, 
			0, UniformMemoryBytes, 0, &VulkanUniformBufferTemp.UniformBufferMap.MemoryMapPointer
		);
		VulkanUniformBufferTemp.UniformBufferMap.MemoryMapBytes = UniformMemoryBytes;
		// delete_ubo => "vkUnmapMemory".
		return true;
	}

	VulkanUniformBuffer VulkanCreateUniformBuffer::__MS_STORAGE(bool* storage_state) {
		// vk_uniform_buffer(ptr) | vk_uniform_buffer_memory(ptr) nullptr => storage res err.
		if (VulkanUniformBufferTemp.UniformBuffer == VK_NULL_HANDLE || VulkanUniformBufferTemp.UniformBufferMemory == VK_NULL_HANDLE) {
			*storage_state = false;
		}
		*storage_state = true;
		return VulkanUniformBufferTemp;
	}

	// ############################################ Shader(Pipeline) Layout ############################################
	RZ_VULKAN_DEFLABEL RZVK_MANAGER_LEYOUT = "rzvk_manager_layout";

	atomic<uint32_t> VulkanCreateLayout::VulkanShaderLayoutCount = 0;

	size_t VulkanCreateLayout::CreateLayoutUniformBuffer(VulkanShaderType visib) {
		VkDescriptorSetLayoutBinding UBOLayoutBinding = {};

		UBOLayoutBinding.binding         = ShaderUniformBinding;
		UBOLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UBOLayoutBinding.descriptorCount = 1;

		switch (visib) {
			// all_type: graphics_shader, non-compute_shader.
		case(ShaderIsGraphics): { UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS; break; }
		case(ShaderIsVertex):   { UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;   break; }
		case(ShaderIsFragment): { UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; break; }
		}
		UBOLayoutBinding.pImmutableSamplers = nullptr;

		DescriptorSetLayoutConfig.push_back(UBOLayoutBinding);
		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_LEYOUT, "create layout: uniform_buffer."));
		return DescriptorSetLayoutConfig.size();
	}

	size_t VulkanCreateLayout::CreateLayoutUniformSampler() {
		VkDescriptorSetLayoutBinding UBOLayoutBinding = {};

		UBOLayoutBinding.binding         = ShaderSamplerBinding;
		UBOLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		UBOLayoutBinding.descriptorCount = 1;

		UBOLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
		UBOLayoutBinding.pImmutableSamplers = nullptr;

		DescriptorSetLayoutConfig.push_back(UBOLayoutBinding);
		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_LEYOUT, "create layout: uniform_sampler."));
		return DescriptorSetLayoutConfig.size();
	}

	bool VulkanCreateLayout::CreatePipelineShaderLayout() {
		VkDescriptorSetLayoutCreateInfo DescSetCreate = {};

		DescSetCreate.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		DescSetCreate.bindingCount = (uint32_t)DescriptorSetLayoutConfig.size();
		DescSetCreate.pBindings    = DescriptorSetLayoutConfig.data();

		ResultInfoGet(
			vkCreateDescriptorSetLayout(HANDLE_VK_DEVICE_LOGIC, &DescSetCreate, nullptr, &VulkanShaderLayoutTemp.LayoutDescriptorSet),
			RzVkLog(LogInfo, RZVK_MANAGER_LEYOUT, "create pipeline_layout desc_set."),
			RzVkLog(LogError, RZVK_MANAGER_LEYOUT, "failed create pipeline_layout desc_set.")
		);

		VkPipelineLayoutCreateInfo PipelineLayoutCreate = {};

		PipelineLayoutCreate.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutCreate.setLayoutCount         = 1;
		PipelineLayoutCreate.pSetLayouts            = &VulkanShaderLayoutTemp.LayoutDescriptorSet;
		PipelineLayoutCreate.pushConstantRangeCount = 0;
		PipelineLayoutCreate.pPushConstantRanges    = nullptr;

		VkPipelineLayout* PipelineLayout = &VulkanShaderLayoutTemp.LayoutPipeline;
		ResultInfoGet(
			vkCreatePipelineLayout(HANDLE_VK_DEVICE_LOGIC, &PipelineLayoutCreate, nullptr, PipelineLayout),
			RzVkLog(LogInfo, RZVK_MANAGER_LEYOUT, "create pipeline_layout."),
			RzVkLog(LogError, RZVK_MANAGER_LEYOUT, "failed create pipeline_layout.")
		);
		return true;
	}

	VulkanShaderLayout VulkanCreateLayout::__MS_STORAGE(bool* storage_state) {
		// vk_pipeline_layout(ptr) nullptr => storage res err.
		if (VulkanShaderLayoutTemp.LayoutPipeline == VK_NULL_HANDLE) {
			*storage_state = false;
		}
		*storage_state = true;
		VulkanShaderLayoutTemp.ShaderLayoutUnqiue = ++VulkanShaderLayoutCount;
		return VulkanShaderLayoutTemp;
	}

	// ############################################ Shader(Pipeline) ############################################
	RZ_VULKAN_DEFLABEL RZVK_MANAGER_SHADER = "rzvk_manager_shader";

	VulkanShaderModule VulkanCreateShader::CreateShaderModule(const RzVkShaderSPIRV& code, VulkanShaderType type) {
		VkShaderModule           ShaderModule  = nullptr;
		VkShaderModuleCreateInfo SshaderCreate = {};

		if (code.empty()) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_SHADER, "shader_module: null spv_code."));
			return VulkanShaderModule(type, nullptr);
		}
		SshaderCreate.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		SshaderCreate.codeSize = code.size();
		SshaderCreate.pCode    = reinterpret_cast<const uint32_t*>(code.data());

		if (type == NULL) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_SHADER, "shader_module: type = null."));
			return VulkanShaderModule(type, nullptr);
		}

		ResultInfoGet(
			vkCreateShaderModule(HANDLE_VK_DEVICE_LOGIC, &SshaderCreate, nullptr, &ShaderModule),
			RzVkLog(LogInfo, RZVK_MANAGER_SHADER, "create shader_module typecode: %u", (uint32_t)type),
			RzVkLog(LogError, RZVK_MANAGER_SHADER, "failed create shader_module typecode: %u", (uint32_t)type)
		);
		return VulkanShaderModule(type, ShaderModule);
	}

	void VulkanCreateShader::ShaderLoaderVert(const RzVkShaderSPIRV& code) {
		VkPipelineShaderStageCreateInfo VertShaderStageCreate = {};
		// create shader_module: code'spir-v', vertex.
		ShaderModuleVertex = CreateShaderModule(code, ShaderIsVertex);

		VertShaderStageCreate.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		VertShaderStageCreate.stage  = VK_SHADER_STAGE_VERTEX_BIT;
		VertShaderStageCreate.module = ShaderModuleVertex.ShaderModule;
		VertShaderStageCreate.pName  = "main";

		DatasetShadersStage.push_back(VertShaderStageCreate);
	}

	void VulkanCreateShader::ShaderLoaderFrag(const RzVkShaderSPIRV& code) {
		VkPipelineShaderStageCreateInfo FragShaderStageCreate = {};
		// create shader_module: code'spir-v', fragment.
		ShaderModuleFragment = CreateShaderModule(code, ShaderIsFragment);

		FragShaderStageCreate.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		FragShaderStageCreate.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragShaderStageCreate.module = ShaderModuleFragment.ShaderModule;
		FragShaderStageCreate.pName  = "main";

		DatasetShadersStage.push_back(FragShaderStageCreate);
	}

	// 获取设备(GPU)多重采样样本数量.
	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice phy_device) {
		VkPhysicalDeviceProperties PhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(phy_device, &PhysicalDeviceProperties);

		VkSampleCountFlags Counts = 
			PhysicalDeviceProperties.limits.framebufferColorSampleCounts & PhysicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (Counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (Counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (Counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (Counts & VK_SAMPLE_COUNT_8_BIT)  { return VK_SAMPLE_COUNT_8_BIT;  }
		if (Counts & VK_SAMPLE_COUNT_4_BIT)  { return VK_SAMPLE_COUNT_4_BIT;  }
		if (Counts & VK_SAMPLE_COUNT_2_BIT)  { return VK_SAMPLE_COUNT_2_BIT;  }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	bool VulkanCreateShader::CreatePipelineShader(const VulkanCreateShaderConfig& config) {
		VkRect2D Scissor = {};

		Scissor.offset = { 0, 0 };
		Scissor.extent = { (uint32_t)config.ConfigRenderViewport.width, (uint32_t)config.ConfigRenderViewport.height };

		VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreate = {};
		// 绘制图元: 封闭三角形非重用顶点连线.
		InputAssemblyCreate.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssemblyCreate.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		InputAssemblyCreate.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo ViewportStateCreate = {};

		ViewportStateCreate.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportStateCreate.viewportCount = 1;
		ViewportStateCreate.pViewports    = &config.ConfigRenderViewport;
		ViewportStateCreate.scissorCount  = 1;
		ViewportStateCreate.pScissors     = &Scissor;

		VkPipelineRasterizationStateCreateInfo RasterizerCreate = {};

		RasterizerCreate.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizerCreate.depthClampEnable = VK_FALSE;

		RasterizerCreate.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizerCreate.lineWidth   = 1.0f;

		// 绘制图元为: 填充模式 / 线框(边缘)模式.
		if (config.EnableLinesMode) {
			// lines_mode: psag[std]: const draw_line_width: 3.2f (fp32)
			// update: 2024_08_30. RCSZ.
			RasterizerCreate.polygonMode = VK_POLYGON_MODE_LINE;
			RasterizerCreate.lineWidth   = 3.2f;
		}

		// 背面剔除(flags): VK_CULL_MODE_BACK_BIT & VK_FRONT_FACE_CLOCKWISE.
		RasterizerCreate.cullMode  = VK_CULL_MODE_NONE;
		RasterizerCreate.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// vulkan_doc: "常量值或根据片段的斜率偏置深度值来更改深度值"
		// psag[std]: psag_framework is 2d => non-setting.
		// set: false. 2024_08_30. RCSZ.
		RasterizerCreate.depthBiasEnable         = VK_FALSE;
		RasterizerCreate.depthBiasConstantFactor = 0.0f;
		RasterizerCreate.depthBiasClamp          = 0.0f;
		RasterizerCreate.depthBiasSlopeFactor    = 0.0f;

		VkPipelineMultisampleStateCreateInfo MultisamplingCreate = {};

		MultisamplingCreate.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisamplingCreate.sampleShadingEnable   = VK_FALSE;
		MultisamplingCreate.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		MultisamplingCreate.minSampleShading      = 1.0f;
		MultisamplingCreate.pSampleMask           = nullptr;
		MultisamplingCreate.alphaToCoverageEnable = VK_FALSE;
		MultisamplingCreate.alphaToOneEnable      = VK_FALSE;

		VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};

		ColorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		ColorBlendAttachment.blendEnable         = VK_TRUE;
		ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		ColorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
		ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

		// "ColorBlendAttachment" =ptr=> "ColorBlendingCreate".
		VkPipelineColorBlendStateCreateInfo ColorBlendingCreate = {};

		ColorBlendingCreate.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlendingCreate.logicOpEnable   = VK_FALSE;
		ColorBlendingCreate.logicOp         = VK_LOGIC_OP_COPY;
		ColorBlendingCreate.attachmentCount = 1;
		ColorBlendingCreate.pAttachments    = &ColorBlendAttachment;

		memset(ColorBlendingCreate.blendConstants, NULL, sizeof(float) * 4);

		VkPipelineDepthStencilStateCreateInfo DepthStencilCreate = {};

		DepthStencilCreate.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencilCreate.depthTestEnable  = VK_TRUE;
		DepthStencilCreate.depthWriteEnable = VK_TRUE;
		DepthStencilCreate.depthCompareOp   = VK_COMPARE_OP_LESS;

		DepthStencilCreate.depthBoundsTestEnable = VK_FALSE;
		DepthStencilCreate.minDepthBounds        = 0.0f;
		DepthStencilCreate.maxDepthBounds        = 1.0f;

		DepthStencilCreate.stencilTestEnable = VK_FALSE;
		DepthStencilCreate.front             = {};
		DepthStencilCreate.back              = {};

		vector<VkDynamicState> DynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo DynamicStateCreate = {};

		DynamicStateCreate.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicStateCreate.dynamicStateCount = (uint32_t)DynamicStates.size();
		DynamicStateCreate.pDynamicStates    = DynamicStates.data();

		VkGraphicsPipelineCreateInfo PipelineCreate = {};

		// check shader_stage >= 1 ?
		if (DatasetShadersStage.size() < 1) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_SHADER, "shader config: shader_stage < 1."));
			return false;
		}
		PipelineCreate.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		PipelineCreate.stageCount = (uint32_t)DatasetShadersStage.size();
		PipelineCreate.pStages    = DatasetShadersStage.data();

		PipelineCreate.pInputAssemblyState = &InputAssemblyCreate;
		PipelineCreate.pViewportState      = &ViewportStateCreate;
		PipelineCreate.pRasterizationState = &RasterizerCreate;
		PipelineCreate.pMultisampleState   = &MultisamplingCreate;
		PipelineCreate.pDynamicState       = &DynamicStateCreate;
		PipelineCreate.pColorBlendState    = &ColorBlendingCreate;
		PipelineCreate.pDepthStencilState  = &DepthStencilCreate;

		PipelineCreate.pVertexInputState = &config.ConfigVertAttribute.PiplineVertexAttribute;

		PipelineCreate.layout = config.ConfigPipelineLayout.LayoutPipeline;

		// check sub_render_pass >= 1 | bind_count > max ?
		if (config.RenderPass.RenderMaxSubPass < 1 || config.SubRenderPassBinding >= config.RenderPass.RenderMaxSubPass) {
			RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_MANAGER_SHADER, "shader config: sub_render_pass < 1 | bind > max."));
			return false;
		}
		PipelineCreate.renderPass = config.RenderPass.RenderPass;
		PipelineCreate.subpass    = config.SubRenderPassBinding;

		// 当前渲染管道的派生渲染管道, 暂不使用.
		// 2024_09_01. RCSZ.
		PipelineCreate.basePipelineHandle = VK_NULL_HANDLE;
		PipelineCreate.basePipelineIndex  = -1;

		ResultInfoGet(
			vkCreateGraphicsPipelines(HANDLE_VK_DEVICE_LOGIC, VK_NULL_HANDLE, 1, &PipelineCreate, nullptr, &VulkanShaderTemp.ShaderPipeline),
			RzVkLog(LogInfo, RZVK_MANAGER_SHADER, "create shader(pipeline)."),
			RzVkLog(LogError, RZVK_MANAGER_SHADER, "failed create shader(pipeline).")
		);
		// shader layout unique_code. 
		VulkanShaderTemp.IsShaderLayout = config.ConfigPipelineLayout.ShaderLayoutUnqiue;

		// delete shader modules.
		vkDestroyShaderModule(HANDLE_VK_DEVICE_LOGIC, ShaderModuleVertex.ShaderModule,   nullptr);
		vkDestroyShaderModule(HANDLE_VK_DEVICE_LOGIC, ShaderModuleFragment.ShaderModule, nullptr);

		VkDescriptorSetAllocateInfo AllocDescConfig = {};

		AllocDescConfig.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocDescConfig.descriptorPool     = HANDLE_VK_POOL_DESC;
		AllocDescConfig.descriptorSetCount = 1;
		AllocDescConfig.pSetLayouts        = &config.ConfigPipelineLayout.LayoutDescriptorSet;

		ResultInfoGet(
			vkAllocateDescriptorSets(HANDLE_VK_DEVICE_LOGIC, &AllocDescConfig, &VulkanShaderTemp.ShaderDescriptorSet),
			RzVkLog(LogInfo, RZVK_MANAGER_SHADER, "shader(pipeline): allocate descriptor_set."),
			RzVkLog(LogError, RZVK_MANAGER_SHADER, "failed shader(pipeline): allocate descriptor_set.")
		);

		if (config.ConfigUniformBuffer.UniformBuffer == VK_NULL_HANDLE) {
			RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_SHADER, "shader(pipeline): non descriptor_set."));
			return true;
		}

		VkDescriptorBufferInfo BufferInfo = {};

		BufferInfo.buffer = config.ConfigUniformBuffer.UniformBuffer;
		BufferInfo.range  = config.ConfigUniformBuffer.UniformBufferMap.MemoryMapBytes;
		BufferInfo.offset = 0;

		VkWriteDescriptorSet DescriptorWriteConfig = {};

		DescriptorWriteConfig.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWriteConfig.dstSet          = VulkanShaderTemp.ShaderDescriptorSet;
		DescriptorWriteConfig.dstBinding      = ShaderUniformBinding;
		DescriptorWriteConfig.dstArrayElement = 0;

		DescriptorWriteConfig.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		DescriptorWriteConfig.descriptorCount = 1;
		DescriptorWriteConfig.pBufferInfo     = &BufferInfo;

		vkUpdateDescriptorSets(HANDLE_VK_DEVICE_LOGIC, 1, &DescriptorWriteConfig, 0, nullptr);

		// update image_texture desc_set.
		if (!config.ConfigImageTextures.empty()) {
			vector<VkDescriptorImageInfo> ImagesInfo = {};

			for (const auto& TexItem : config.ConfigImageTextures) {
				// tex_image view & sampler => desc.
				VkDescriptorImageInfo ImageInfo = {};

				ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				ImageInfo.imageView   = TexItem.ColorTextureView;
				ImageInfo.sampler     = TexItem.ColorTextureSampler;

				ImagesInfo.push_back(ImageInfo);
			}
			VkWriteDescriptorSet DescriptorWriteConfig = {};

			DescriptorWriteConfig.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			DescriptorWriteConfig.dstSet          = VulkanShaderTemp.ShaderDescriptorSet;
			DescriptorWriteConfig.dstBinding      = ShaderSamplerBinding;
			DescriptorWriteConfig.dstArrayElement = 0;

			DescriptorWriteConfig.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			DescriptorWriteConfig.descriptorCount = (uint32_t)ImagesInfo.size();
			DescriptorWriteConfig.pImageInfo      = ImagesInfo.data();

			vkUpdateDescriptorSets(HANDLE_VK_DEVICE_LOGIC, 1, &DescriptorWriteConfig, 0, nullptr);
		}
		RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_MANAGER_SHADER, "shader(pipeline): update descriptor_set."));
		// non-error, const_flag: true, 20240904. RCSZ. 
		return true;
	}

	VulkanShader VulkanCreateShader::__MS_STORAGE(bool* storage_state) {
		// vk_pipeline(ptr) nullptr => storage res err.
		if (VulkanShaderTemp.ShaderPipeline == VK_NULL_HANDLE) {
			*storage_state = false;
		}
		*storage_state = true;
		return VulkanShaderTemp;
	}
}