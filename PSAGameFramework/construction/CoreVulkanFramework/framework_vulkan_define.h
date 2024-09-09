// framework_vulkan_define.
// files_strcut: framework_vulkan_define.h => framework_vulkan.hpp
#ifndef _FRAMEWORK_VULKAN_DEFINE_H
#define _FRAMEWORK_VULKAN_DEFINE_H

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <optional>
#include <type_traits>

#define DEF_RZVULKAN_NAME "RzVulkanEngine"
constexpr uint32_t DEF_RZVULKAN_VER[3] = { 1,2,1 };

// vector 2d x, y. 
// vulkan memory_std alignment: 8[bytes].
template <typename mvec>
struct alignas(8) FmtVector2T {
	mvec vector_x, vector_y;
	constexpr FmtVector2T() : vector_x{}, vector_y{} {}
	constexpr FmtVector2T(mvec x, mvec y) : vector_x(x), vector_y(y) {}

	mvec* data() { return &vector_x; }
	const mvec* data() const { return &vector_x; }
};

// vector 4d x, y, z, w. 
// vulkan memory_std alignment: 16[bytes].
template <typename mvec>
struct alignas(16) FmtVector4T {
	mvec vector_x, vector_y, vector_z, vector_w;
	constexpr FmtVector4T() : vector_x{}, vector_y{}, vector_z{}, vector_w{} {}
	constexpr FmtVector4T(mvec x, mvec y, mvec z, mvec w) : vector_x(x), vector_y(y), vector_z(z), vector_w(w) {}

	mvec* data() { return &vector_x; }
	const mvec* data() const { return &vector_x; }
};

// rz_vulkan image_pixel src_info,data.
struct RzVkImageSRC {
	uint32_t              ImageChannels = {};
	FmtVector2T<uint32_t> ImageSize = {};
	std::vector<uint8_t>  ImageSrcData = {};
};

// framework_vulkan => psagame2d.
#if (POMELO_STAR_GAME2D_DEF == false)
enum LOGLABEL {
	LogError   = 1 << 1, // 标签 <错误>
	LogWarning = 1 << 2, // 标签 <警告>
	LogInfo    = 1 << 3, // 标签 <信息>
	LogTrace   = 1 << 4, // 标签 <跟踪>
	LogPerfmac = 1 << 5  // 标签 <性能> (performance)
};
#endif

// rzvk low_resource hashmap key_type.
using VKLRES_KEY = uint64_t;

#define RZ_VULKAN_DEFLABEL constexpr const char*
#define RZ_VULKAN_DEFSTD // ...
// rz_vulkan low_framework define, 20240906.
// std_version: 0.12, update: 20240906.
namespace RZ_VULKAN {

	using RzVkShaderSPIRV = std::vector<char>;
	using RzVkIndexCode   = uint32_t;
	using RzVkBindingUi32 = uint32_t;
	using RzVkExtName     = const char*;

#define RZVK_CLEAR_COLOR FmtVector4T<float>(0.0f, 0.0f, 0.0f, 1.0f)
#define RZVK_CLEAR_DEPTH FmtVector2T<float>(1.0f, 0.0f)

	// gpu memory =mapping=> cpu. struct_bytes copy.
	RZ_VULKAN_DEFSTD struct GPUmemoryMapping {
		void*  MemoryMapPointer = nullptr;
		size_t MemoryMapBytes   = NULL;

		template<typename TU, typename = std::enable_if_t<std::is_class<TU>::value>>
		bool StructMemCopy(TU OBJ) {
			if (MemoryMapPointer == nullptr || sizeof(TU) != MemoryMapBytes)
				return false;
			std::memcpy(MemoryMapPointer, &OBJ, MemoryMapBytes);
			return true;
		}
	};

	constexpr const char* RZVK_SAMPLE_MODENAME[4] = {
		"repeat", "mirrored_repeat", "clamp_to_edge", "clamp_to_border"
	};
	// rz_vk framework sampler preset_param.
	enum RZVK_SAMPLE_MODE {
		TEX_NEAREST_AND_REPEAT          = 0,
		TEX_NEAREST_AND_MIRRORED_REPEAT = 1,
		TEX_NEAREST_AND_CLAMP_TO_EDGE   = 2,
		TEX_NEAREST_AND_CLAMP_TO_BORDER = 3,

		TEX_LINEAR_AND_REPEAT          = 4,
		TEX_LINEAR_AND_MIRRORED_REPEAT = 5,
		TEX_LINEAR_AND_CLAMP_TO_EDGE   = 6,
		TEX_LINEAR_AND_CLAMP_TO_BORDER = 7
	};

	enum VulkanShaderType {
		ShaderIsGraphics = 1 << 0,
		ShaderIsVertex   = 1 << 1,
		ShaderIsFragment = 1 << 2
	};
	RZ_VULKAN_DEFSTD struct VulkanShaderModule {
		VulkanShaderType ShaderModuleType;
		VkShaderModule   ShaderModule;

		VulkanShaderModule() : ShaderModuleType((VulkanShaderType)NULL), ShaderModule(nullptr) {}
		VulkanShaderModule(VulkanShaderType type, VkShaderModule m_ptr) : ShaderModuleType(type), ShaderModule(m_ptr) {}
	};

	RZ_VULKAN_DEFSTD struct VulkanClearValues {
		// render_cmd => begin_pass => clear_values.
		std::vector<VkClearValue> AttachmentClearValues = {};
	};

	RZ_VULKAN_DEFSTD struct VulkanVertexAttribute {
		VkPipelineVertexInputStateCreateInfo PiplineVertexAttribute = {};
		// describe vertex data_attibutes & desc(s) & tag_name(s).
		VkVertexInputBindingDescription                AttributeBindingBuffer = {};
		std::vector<VkVertexInputAttributeDescription> AttributeDescriptions  = {};
		std::vector<std::string>                       AttributeTagNameString = {};
	};

	RZ_VULKAN_DEFSTD struct VulkanVertexBuffer {
		VkDeviceMemory VertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer       VertexBuffer       = VK_NULL_HANDLE;
		// global vertex_buffer binding(only).
		RzVkBindingUi32 VertexBufferBinding = NULL;
	};

	RZ_VULKAN_DEFSTD struct VulkanUniformBuffer {
		VkDeviceMemory UniformBufferMemory = VK_NULL_HANDLE;
		VkBuffer       UniformBuffer       = VK_NULL_HANDLE;
		// buffer: gpu_mem_addr => cpu_addr.
		GPUmemoryMapping UniformBufferMap = {};
	};

	RZ_VULKAN_DEFSTD struct VulkanShaderLayout {
		VkDescriptorSetLayout LayoutDescriptorSet = VK_NULL_HANDLE;
		VkPipelineLayout      LayoutPipeline      = VK_NULL_HANDLE;
		// shader_layout unique index.
		RzVkIndexCode ShaderLayoutUnqiue = NULL;
	};

	RZ_VULKAN_DEFSTD struct VulkanShader {
		VkDescriptorSet ShaderDescriptorSet = VK_NULL_HANDLE;
		VkPipeline      ShaderPipeline      = VK_NULL_HANDLE;
		// shader =unique=find=> shader_layout.
		RzVkIndexCode IsShaderLayout = NULL;
	};

	RZ_VULKAN_DEFSTD struct VulkanTextureColor {
		// color_texture data storage. [CPU不可访问]
		VkDeviceMemory TextureBufferMemory = VK_NULL_HANDLE;
		// texture sampler(mem => shader). [optional]
		VkSampler   ColorTextureSampler = VK_NULL_HANDLE;
		VkImage     ColorTexture        = VK_NULL_HANDLE;
		VkImageView ColorTextureView    = VK_NULL_HANDLE;
	};

	RZ_VULKAN_DEFSTD struct VulkanTextureDepth {
		// depth_texture data storage. [CPU不可访问]
		VkDeviceMemory TextureBufferMemory = VK_NULL_HANDLE;
		// texture sampler(mem => shader). [optional]
		VkSampler      DepthTextureSampler = VK_NULL_HANDLE;
		VkImage        DepthTexture        = VK_NULL_HANDLE;
		VkImageView    DepthTextureView    = VK_NULL_HANDLE;
		// depth_texture create format.
		VkFormat DepthTextureFormat = VK_FORMAT_UNDEFINED;
	};

	// rz_vulkan create => resource interface define.
	// psagame2d - vk.std, rcsz, v202409.

	class CreateInterfaceVA { public: virtual VulkanVertexAttribute __MS_STORAGE(bool* storage_state) = 0; };
	class CreateInterfaceVB { public: virtual VulkanVertexBuffer    __MS_STORAGE(bool* storage_state) = 0; };
	class CreateInterfaceUB { public: virtual VulkanUniformBuffer   __MS_STORAGE(bool* storage_state) = 0; };
	class CreateInterfaceSL { public: virtual VulkanShaderLayout    __MS_STORAGE(bool* storage_state) = 0; };
	class CreateInterfaceSP { public: virtual VulkanShader          __MS_STORAGE(bool* storage_state) = 0; };
	class CreateInterfaceCT { public: virtual VulkanTextureColor    __MS_STORAGE(bool* storage_state) = 0; };
	class CreateInterfaceDT { public: virtual VulkanTextureDepth    __MS_STORAGE(bool* storage_state) = 0; };
}

namespace RZ_VULKAN_RESOURCE {
	// rz_vulkan resource interface define.
	// psagame2d - vk.std, rcsz, v202409.
	
	// vertex_attribute resource. INTERFACE. [20240909]
	class ResourceInterfaceVA {
	public:
		virtual RZ_VULKAN::VulkanVertexAttribute ResourceFind(VKLRES_KEY key) = 0;

		virtual bool   ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceVA* res) = 0;
		virtual bool   ResourceDelete(VKLRES_KEY key) = 0;
		virtual size_t ResourceSize() = 0;

		virtual ~ResourceInterfaceVA() = default;
	};

	// vertex_buffer resource. INTERFACE. [20240909]
	class ResourceInterfaceVB {
	public:
		virtual RZ_VULKAN::VulkanVertexBuffer ResourceFind(VKLRES_KEY key) = 0;

		virtual bool   ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceVB* res) = 0;
		virtual bool   ResourceDelete(VKLRES_KEY key) = 0;
		virtual size_t ResourceSize() = 0;

		virtual ~ResourceInterfaceVB() = default;
	};

	// uniform_buffer resource. INTERFACE. [20240909]
	class ResourceInterfaceUB {
	public:
		virtual RZ_VULKAN::VulkanUniformBuffer ResourceFind(VKLRES_KEY key) = 0;

		virtual bool   ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceUB* res) = 0;
		virtual bool   ResourceDelete(VKLRES_KEY key) = 0;
		virtual size_t ResourceSize() = 0;

		virtual ~ResourceInterfaceUB() = default;
	};

	// shader_layout resource. INTERFACE. [20240909]
	class ResourceInterfaceSL {
	public:
		virtual RZ_VULKAN::VulkanShaderLayout ResourceFind(VKLRES_KEY key) = 0;

		virtual bool   ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceSL* res) = 0;
		virtual bool   ResourceDelete(VKLRES_KEY key) = 0;
		virtual size_t ResourceSize() = 0;

		virtual ~ResourceInterfaceSL() = default;
	};

	// shader_pipeline resource. INTERFACE. [20240909]
	class ResourceInterfaceSP {
	public:
		virtual RZ_VULKAN::VulkanShader ResourceFind(VKLRES_KEY key) = 0;

		virtual bool   ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceSP* res) = 0;
		virtual bool   ResourceDelete(VKLRES_KEY key) = 0;
		virtual size_t ResourceSize() = 0;

		virtual ~ResourceInterfaceSP() = default;
	};

	// color_texture resource. INTERFACE. [20240909]
	class ResourceInterfaceCT {
	public:
		virtual RZ_VULKAN::VulkanTextureColor ResourceFind(VKLRES_KEY key) = 0;

		virtual bool   ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceCT* res) = 0;
		virtual bool   ResourceDelete(VKLRES_KEY key) = 0;
		virtual size_t ResourceSize() = 0;

		virtual ~ResourceInterfaceCT() = default;
	};

	// depth_texture resource. INTERFACE. [20240909]
	class ResourceInterfaceDT {
	public:
		virtual RZ_VULKAN::VulkanTextureDepth ResourceFind(VKLRES_KEY key) = 0;

		virtual bool   ResourceStorage(VKLRES_KEY* key, RZ_VULKAN::CreateInterfaceDT* res) = 0;
		virtual bool   ResourceDelete(VKLRES_KEY key) = 0;
		virtual size_t ResourceSize() = 0;

		virtual ~ResourceInterfaceDT() = default;
	};
}

#endif