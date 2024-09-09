// framework_vulkan_init. RCSZ.
#define NOMINMAX
#include <cstdint>
#include <set>
#include <limits>
#include <algorithm>
#include "framework_vulkan.hpp"

using namespace std;

std::function<void(const RZ_VULKAN::RzVkLog&)> RZVK_FUNC_LOGGER  = {};
std::function<void(int)>                       RZVK_FUNC_EXIT    = {};
std::function<int64_t()>                       RZVK_FUNC_TIMENOW = {};

namespace RZ_VULKAN {
    std::vector<RzVkExtName> VulkanCoreExtensions::ExtensionEnableInstance = {};
    std::vector<RzVkExtName> VulkanCoreExtensions::ExtensionEnableDevice   = {};

    VkInstance VulkanCoreStaticResource::HANDLE_VK_INSTANCE = VK_NULL_HANDLE;

    VkPhysicalDevice VulkanCoreStaticResource::HANDLE_VK_DEVICE_PHYSICAL = VK_NULL_HANDLE;
    VkDevice         VulkanCoreStaticResource::HANDLE_VK_DEVICE_LOGIC    = VK_NULL_HANDLE;

    VkQueue VulkanCoreStaticResource::HANDLE_VK_QUEUE_GRAPH   = VK_NULL_HANDLE;
    VkQueue VulkanCoreStaticResource::HANDLE_VK_QUEUE_PRESENT = VK_NULL_HANDLE;

    VkSurfaceKHR   VulkanCoreStaticResource::HANDLE_VK_SURFACE   = VK_NULL_HANDLE;
    VkSwapchainKHR VulkanCoreStaticResource::HANDLE_VK_SWAPCHAIN = VK_NULL_HANDLE;

    VkCommandPool    VulkanCoreStaticResource::HANDLE_VK_POOL_COMMAND = VK_NULL_HANDLE;
    VkDescriptorPool VulkanCoreStaticResource::HANDLE_VK_POOL_DESC    = VK_NULL_HANDLE;

    // rz_vulkan framework extensions.
    // update: 2024_08_30. RCSZ.
    VulkanCoreExtensions::VulkanCoreExtensions() {
#if defined(GLFW_VERSION_MAJOR)
        // glfw => get vulkan instance exten(s).
        uint32_t GLFW_ExtensionCount = NULL;
        const char**  GLFW_Extensions = glfwGetRequiredInstanceExtensions(&GLFW_ExtensionCount);

        // push_add glfw_extensions.
        for (size_t i = 0; i < GLFW_ExtensionCount; ++i)
            ExtensionEnableInstance.push_back(GLFW_Extensions[i]);
#endif
        // push_add instance_level extensions.
        ExtensionEnableInstance.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        // push_add device_level extensions.
        ExtensionEnableDevice.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    void VulkanCoreError::ResultInfoGet(VkResult value, RzVkLog succ_info, RzVkLog info) {
        if (value == VK_SUCCESS) {
            RZVK_FUNC_LOGGER(succ_info);
            return;
        }
        RzVkResultErrorList[RZVK_FUNC_TIMENOW()] = RzVkError(info, (int64_t)value);
    }

    inline QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR* vs_khr = nullptr) {
        QueueFamilyIndices Indices = {};
        uint32_t QueueFamilyCount = 0, Count = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, nullptr);
        vector<VkQueueFamilyProperties> QueueFamiliesTemp(QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, QueueFamiliesTemp.data());
        
        // 查询图形队列簇.
        for (const auto& QueueFamily : QueueFamiliesTemp) {
            if (QueueFamily.queueCount > 0 && QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                Indices.GraphicsFamily = Count;
            // graph value_valid => break.
            if (Indices.GraphicsFamily.has_value()) break;
            ++Count;
        }
        if (vs_khr == nullptr)
            return Indices;

        // 查询显示队列簇.
        VkBool32 PresentSupport = false;
        Count = NULL;
        for (const auto& QueueFamily : QueueFamiliesTemp) {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, Count, *vs_khr, &PresentSupport);
            if (PresentSupport)
                Indices.PresentFamily = Count;
            // graph value_valid & present value_valid => break.
            if (Indices.FamilyComplete()) break;
            ++Count;
        }
        return Indices;
    }

    inline bool DeviceIsSuitableGPU(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties DeviceProperties = {};
        VkPhysicalDeviceFeatures   DeviceFeatures   = {};

        vkGetPhysicalDeviceProperties(device, &DeviceProperties);
        vkGetPhysicalDeviceFeatures(device, &DeviceFeatures);

        return (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && 
            DeviceFeatures.geometryShader &&
            DeviceFeatures.samplerAnisotropy;
    }

    RZ_VULKAN_DEFLABEL RZVK_CORE_EXTEN = "rzvk_extension";
    inline bool DeviceCheckExtensionSupport(
        VkPhysicalDevice device, const vector<RzVkExtName>& d_dxtension
    ) {
        uint32_t ExtensionCount = NULL;

        vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, nullptr);
        vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, AvailableExtensions.data());

        set<string> RequiredExtensions(d_dxtension.begin(), d_dxtension.end());
        for (const auto& Item : d_dxtension)
            RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_EXTEN, "extensions set item: %s", Item));

        // erase valid_extension.
        for (const auto& Extension : AvailableExtensions) {
            RequiredExtensions.erase(Extension.extensionName);
        }
        for (const auto& NonExten : RequiredExtensions)
            RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CORE_EXTEN, "not supported exten(s): %s", NonExten.c_str()));
        // true(empty): success.
        return RequiredExtensions.empty();
    }

    inline bool DeviceIsSuitableExt(
        VkPhysicalDevice device, VkSurfaceKHR* vs_khr, const vector<RzVkExtName>& d_dxtension
    ) {
        QueueFamilyIndices Indices = FindQueueFamilies(device, vs_khr);
        // 检查设备是否支持所有vk扩展.
        bool ExtensionsSupported = DeviceCheckExtensionSupport(device, d_dxtension);
        return Indices.FamilyComplete() && ExtensionsSupported;
    }

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& available_formats) {
        for (const auto& AvailableFormat : available_formats) {
            if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return AvailableFormat;
        }
        // failed => default_formats.
        return available_formats[0];
    }

    // 设置寻找最佳交换模式.
    VkPresentModeKHR ChooseSwapPresentMode(const vector<VkPresentModeKHR>& available_pesent_modes) {
        for (const auto& AvailablePresentMode : available_pesent_modes) {
            if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return AvailablePresentMode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

#if defined(GLFW_VERSION_MAJOR)
    // 设置交换链范围(图像像素).
    inline VkExtent2D GLFWchooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        // GLFW API => get frame_buffer size.
        FmtVector2T<int> GetBufferSize = {};
        glfwGetFramebufferSize(window, &GetBufferSize.vector_x, &GetBufferSize.vector_y);

        VkExtent2D ActualExtent = {
            (uint32_t)GetBufferSize.vector_x,
            (uint32_t)GetBufferSize.vector_y
        };
        ActualExtent.width  = clamp(ActualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        ActualExtent.height = clamp(ActualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return ActualExtent;
    }
#endif

    const vector<const char*> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };
    VkDebugUtilsMessengerCreateInfoEXT VK_DUMCIEXT_NULL = {};

    void VulkanCore::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        SystemWindowSize = FmtVector2T<uint32_t>((uint32_t)width, (uint32_t)height);
    }
    FmtVector2T<uint32_t> VulkanCore::SystemWindowSize = {};

    RZ_VULKAN_DEFLABEL RZVK_CORE_INIT = "rzvk_core_init";
    void VulkanCore::VulkanCreateValidation(RZVK_VKDUMCIEXT* debug_info) {
        // vk_struct ptr => internal ptr.
        VkDebugUtilsMessengerCreateInfoEXT* InternalPointer = &VK_DUMCIEXT_NULL;
        if (debug_info != nullptr)
            InternalPointer = debug_info;

        uint32_t LayersCount = NULL;
        
        vkEnumerateInstanceLayerProperties(&LayersCount, nullptr);
        vector<VkLayerProperties> AvailableLayers((size_t)LayersCount);
        // get vk_instance_layer dataset.
        vkEnumerateInstanceLayerProperties(&LayersCount, AvailableLayers.data());

        // 检查验证层是否全部有效.
        for (const char* LayerName : VALIDATION_LAYERS) {
            bool LayerFound = false;
            for (const auto& LayerProperties : AvailableLayers) {
                if (strcmp(LayerName, LayerProperties.layerName) == 0) {
                    // layer found_flag set:true => get layer_name.
                    LayerFound = true; break;
                }
            }
            if (!LayerFound) {
                RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_CORE_INIT, "invalid validation layers: %s", LayerName));
                continue;
            }
            RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_INIT, "validation layers: %s", LayerName));
        }

        // 配置验证层扩展.
        InternalPointer->sType = 
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        InternalPointer->messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        InternalPointer->messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        // debug printlog function pointer.
        InternalPointer->pfnUserCallback = VkFunctionLoader::VKDebugCallbackPrint;
        InternalPointer->pUserData       = nullptr;

        VkResult ReturnCode = VkFunctionLoader::VKCreateDebugUtilsMessengerEXT(
            HANDLE_VK_INSTANCE, InternalPointer, nullptr, &DebugInstance
        );
        ResultInfoGet(ReturnCode,
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create verification layer(s)."),
            RzVkLog(LogWarning, RZVK_CORE_INIT, "failed create verification layer(s): %d", (int)ReturnCode)
        );
    }

	void VulkanCore::VulkanCreateInstance() {
        // vulkan application & create handle.
        VkApplicationInfo    InfoApplication = {};
        VkInstanceCreateInfo InfoCreate      = {};

        uint32_t ExtendCount = NULL;
        
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtendCount, nullptr);
        ArrayExtension.resize((size_t)ExtendCount);
        // get vk_extension dataset.
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtendCount, ArrayExtension.data());

        // source: "VkDebugUtilsMessengerCreateInfoEXT".
        RZVK_VKDUMCIEXT DebugConfigInfo = {};

        InfoCreate.enabledLayerCount = NULL;
        InfoCreate.pNext = nullptr;

        if (FlagEnableVerificationLayer) {
            VulkanCreateValidation(&DebugConfigInfo);

            InfoCreate.enabledLayerCount   = (uint32_t)VALIDATION_LAYERS.size();
            InfoCreate.ppEnabledLayerNames = VALIDATION_LAYERS.data();
            InfoCreate.pNext               = (RZVK_VKDUMCIEXT*)&DebugConfigInfo;
        }

        InfoApplication.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        InfoApplication.pApplicationName   = "RZVK";
        InfoApplication.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        InfoApplication.pEngineName        = DEF_RZVULKAN_NAME;
        InfoApplication.engineVersion      = VK_MAKE_VERSION(DEF_RZVULKAN_VER[0], DEF_RZVULKAN_VER[1], DEF_RZVULKAN_VER[2]);
        InfoApplication.apiVersion         = VK_API_VERSION_1_0;

        InfoCreate.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        InfoCreate.pApplicationInfo        = &InfoApplication;
        InfoCreate.enabledExtensionCount   = (uint32_t)ExtensionEnableInstance.size();
        InfoCreate.ppEnabledExtensionNames = ExtensionEnableInstance.data();

        ResultInfoGet(
            vkCreateInstance(&InfoCreate, nullptr, &HANDLE_VK_INSTANCE),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan instanec."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan instanec.")
        );
	}

    void VulkanCore::VulkanCreateWindowSurface() {
        // create window surface | wind32 surface.
#if defined(GLFW_VERSION_MAJOR) // glfw_library create.
        if (glfwCreateWindowSurface(HANDLE_VK_INSTANCE, WindowObjectPtr, nullptr, &HANDLE_VK_SURFACE) != VK_SUCCESS) {
            RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan window surface."));
            return;
        }
#elif defined(_WIN32) // windows system create.
        if (vkCreateWin32SurfaceKHR(HANDLE_VK_INSTANCE, &VulkanWindowConfig, nullptr, &HANDLE_VK_SURFACE) != VK_SUCCESS) {
            RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan win32 surface."));
            return;
        }
#endif
        RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan window surface."));
    }

    bool VulkanCore::VulkanDeviceExtensions() {
        if (DeviceIsSuitableExt(HANDLE_VK_DEVICE_PHYSICAL, &HANDLE_VK_SURFACE, ExtensionEnableDevice)) {
            // enable extensions.
            DeviceCreateConfig.enabledExtensionCount   = (uint32_t)ExtensionEnableDevice.size();
            DeviceCreateConfig.ppEnabledExtensionNames = ExtensionEnableDevice.data();

            RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_INIT, "enable vulkan extensions."));
            return true;
        }
        RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CORE_INIT, "failed enable vulkan extensions."));
        return false;
    }

#define RZVK_SYSTEM_DEVICE_MIN 1
    bool VulkanCore::VulkanCreateDevice() {
        // vulkan device.
        uint32_t DeviceCount = NULL;
        ResultInfoGet(
            vkEnumeratePhysicalDevices(HANDLE_VK_INSTANCE, &DeviceCount, nullptr),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "get vulkan physical devices."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed get vulkan physical devices.")
        );

        if (DeviceCount < RZVK_SYSTEM_DEVICE_MIN) {
            RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CORE_INIT, "fatal error: device_count < min."));
            exit(-1024);
        }
        ArrayPhysicalDevice.resize(DeviceCount);
        vkEnumeratePhysicalDevices(HANDLE_VK_INSTANCE, &DeviceCount, ArrayPhysicalDevice.data());

        if (FlagEnableDeviceTest) {
            for (const auto& Item : ArrayPhysicalDevice) {
                // get physics device name.
                VkPhysicalDeviceProperties DeviceProperties = {};

                vkGetPhysicalDeviceProperties(Item, &DeviceProperties);
                string PhysicalDeviceName = DeviceProperties.deviceName;

                // device_test: state: available?
                bool DeviceValid = DeviceIsSuitableGPU(Item);
                RZVK_FUNC_LOGGER(RzVkLog(LogPerfmac, RZVK_CORE_INIT, "physical device flag: %d, name: %s", 
                    (int)DeviceValid, PhysicalDeviceName.c_str()
                ));
            }
        }
        RZVK_FUNC_LOGGER(RzVkLog(LogPerfmac, RZVK_CORE_INIT, "physical device valid_num: %u", DeviceCount));
        PHYD_INDEX = PHYD_INDEX >= DeviceCount ? DeviceCount - 1 : PHYD_INDEX;
        // type physical device count.
        HANDLE_VK_DEVICE_PHYSICAL = ArrayPhysicalDevice[PHYD_INDEX];

        // create window_suface.
        VulkanCreateWindowSurface();
        
        // ******************************** config device(logic)_queue_family *******************************
        constexpr float QueuePriority = 1.0f;

        QueueFamilyIndices Indices = FindQueueFamilies(HANDLE_VK_DEVICE_PHYSICAL, &HANDLE_VK_SURFACE);
        // storage family_indices info.
        StorageFamilyIndices = Indices;

        vector<VkDeviceQueueCreateInfo> QueueCreateInfos = {};
        set<uint32_t> UniqueQueueFamilies = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

        for (uint32_t QueueFamily : UniqueQueueFamilies) {
            VkDeviceQueueCreateInfo QueueCreateInfo = {};

            QueueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            QueueCreateInfo.queueFamilyIndex = QueueFamily;
            QueueCreateInfo.queueCount       = 1;
            QueueCreateInfo.pQueuePriorities = &QueuePriority;

            QueueCreateInfos.push_back(QueueCreateInfo);
        }
        DeviceCreateConfig.queueCreateInfoCount = (uint32_t)QueueCreateInfos.size();
        DeviceCreateConfig.pQueueCreateInfos    = QueueCreateInfos.data();

        // create device(logic)_extensions.
        bool ENABLE_ALL_EXT = VulkanDeviceExtensions();
        
        VkPhysicalDeviceFeatures DeviceFeaturesConfig = {};

        DeviceFeaturesConfig.fillModeNonSolid  = VK_TRUE;
        DeviceFeaturesConfig.wideLines         = VK_TRUE;
        DeviceFeaturesConfig.samplerAnisotropy = VK_TRUE;

        DeviceCreateConfig.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        DeviceCreateConfig.pEnabledFeatures      = &DeviceFeaturesConfig;
        DeviceCreateConfig.enabledExtensionCount = (uint32_t)ExtensionEnableDevice.size();
        DeviceCreateConfig.enabledLayerCount     = 0;

        if (FlagEnableVerificationLayer) {
            DeviceCreateConfig.ppEnabledLayerNames = VALIDATION_LAYERS.data();
            DeviceCreateConfig.enabledLayerCount   = (uint32_t)VALIDATION_LAYERS.size();
        }

        ResultInfoGet(
            vkCreateDevice(HANDLE_VK_DEVICE_PHYSICAL, &DeviceCreateConfig, nullptr, &HANDLE_VK_DEVICE_LOGIC),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan logic device(s)."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan logic device(s).")
        );
        return ENABLE_ALL_EXT;
    }

    void VulkanCore::VulkanCreateDeviceQueue() {
        // create graphics render queue.
        vkGetDeviceQueue(HANDLE_VK_DEVICE_LOGIC, StorageFamilyIndices.GraphicsFamily.value(), 0, &HANDLE_VK_QUEUE_GRAPH);
        RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan device queue(graphics)."));

        // create window presentation queue.
        vkGetDeviceQueue(HANDLE_VK_DEVICE_LOGIC, StorageFamilyIndices.PresentFamily.value(), 0, &HANDLE_VK_QUEUE_PRESENT);
        RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan device queue(presentation)."));
    }

    bool VulkanCore::VulkanSwapChainSupport(bool swapchain) {
        // 获取交换链支持信息: capabilities, formats, present_mode.
        // get surface capabilities.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            HANDLE_VK_DEVICE_PHYSICAL,
            HANDLE_VK_SURFACE,
            &SwapChainDetails.SCSD_Capabilities
        );

        uint32_t FormatCount = NULL, ModeCount = NULL;
        vkGetPhysicalDeviceSurfaceFormatsKHR(HANDLE_VK_DEVICE_PHYSICAL, HANDLE_VK_SURFACE, &FormatCount, nullptr);
        if (FormatCount > 0) {
            SwapChainDetails.SCSD_Formats.resize(FormatCount);

            // get surface formats.
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                HANDLE_VK_DEVICE_PHYSICAL,
                HANDLE_VK_SURFACE, 
                &FormatCount, SwapChainDetails.SCSD_Formats.data()
            );
        }

        vkGetPhysicalDeviceSurfacePresentModesKHR(HANDLE_VK_DEVICE_PHYSICAL, HANDLE_VK_SURFACE, &ModeCount, nullptr);
        if (ModeCount > 0) {
            SwapChainDetails.SCSD_PresentModes.resize(ModeCount);

            // get surface present_mode.
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                HANDLE_VK_DEVICE_PHYSICAL,
                HANDLE_VK_SURFACE, 
                &ModeCount, SwapChainDetails.SCSD_PresentModes.data()
            );
        }
        bool SUP_FLAG = swapchain && !SwapChainDetails.SCSD_Formats.empty() && !SwapChainDetails.SCSD_PresentModes.empty();
        // extensions & details supported.
        if (SUP_FLAG) {
            RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_INIT, "params vulkan swapchain support."));
            return true;
        }
        RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CORE_INIT, "params vulkan swapchain not-supported."));
        return false;
    }

    void VulkanCore::VulkanCreateSwapchain() {
        // format(color_space), present_mode, entent(buffer_size).
        VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainDetails.SCSD_Formats);
        VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainDetails.SCSD_PresentModes);
        SwapChainExtent = GLFWchooseSwapExtent(SwapChainDetails.SCSD_Capabilities, WindowObjectPtr);

        uint32_t SwapImageCount = SwapChainDetails.SCSD_Capabilities.minImageCount + 1;
        if (SwapChainDetails.SCSD_Capabilities.maxImageCount > 0) {
            // 交换链图片数量: 取 min,max 中位数. RCSZ [20240829].
            SwapImageCount = SwapChainDetails.SCSD_Capabilities.maxImageCount - SwapChainDetails.SCSD_Capabilities.minImageCount + 1;
            SwapImageCount = SwapImageCount / 2 + SwapChainDetails.SCSD_Capabilities.minImageCount;
        }
        SystemWindowSize.vector_x = SwapChainExtent.width;
        SystemWindowSize.vector_y = SwapChainExtent.height;
        
        VkSwapchainCreateInfoKHR SwapchainCreate = {};

        SwapchainCreate.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        SwapchainCreate.surface          = HANDLE_VK_SURFACE;
        SwapchainCreate.minImageCount    = SwapImageCount;
        SwapchainCreate.imageFormat      = SurfaceFormat.format;
        SwapchainCreate.imageColorSpace  = SurfaceFormat.colorSpace;
        SwapchainCreate.imageExtent      = SwapChainExtent;
        SwapchainCreate.imageArrayLayers = 1;
        SwapchainCreate.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        SwapchainCreate.pNext            = nullptr;
        SwapchainCreate.flags            = 0;

        uint32_t QueueFamilyIndices[2] = { StorageFamilyIndices.GraphicsFamily.value(), StorageFamilyIndices.PresentFamily.value() };
        bool ConfigModeFlag = StorageFamilyIndices.GraphicsFamily != StorageFamilyIndices.PresentFamily;

        if (ConfigModeFlag) {
            SwapchainCreate.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            SwapchainCreate.queueFamilyIndexCount = 2;
            SwapchainCreate.pQueueFamilyIndices   = QueueFamilyIndices;
        }
        else {
            SwapchainCreate.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            SwapchainCreate.queueFamilyIndexCount = 0;
            SwapchainCreate.pQueueFamilyIndices   = nullptr;
        }

        SwapchainCreate.preTransform   = SwapChainDetails.SCSD_Capabilities.currentTransform;
        SwapchainCreate.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        SwapchainCreate.presentMode    = PresentMode;
        SwapchainCreate.clipped        = VK_TRUE;
        SwapchainCreate.oldSwapchain   = VK_NULL_HANDLE;

        ResultInfoGet(
            vkCreateSwapchainKHR(HANDLE_VK_DEVICE_LOGIC, &SwapchainCreate, nullptr, &HANDLE_VK_SWAPCHAIN),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan swapchain."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan swapchain.")
        );
    }

    void VulkanCore::VulkanCreateSwapchainImages() {
        uint32_t ImageCount = NULL;

        vkGetSwapchainImagesKHR(HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_SWAPCHAIN, &ImageCount, nullptr);
        SwapChainImages.resize(ImageCount);
        vkGetSwapchainImagesKHR(HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_SWAPCHAIN, &ImageCount, SwapChainImages.data());

        RZVK_FUNC_LOGGER(RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan swapchain images."));
    }

    void VulkanCore::VulkanCreateSwapchainImageViews() {
        // image_views size = images. 
        SwapChainImageViews.resize(SwapChainImages.size());
        // swapchain_image_format = surface_format.
        VkSurfaceFormatKHR SwapChainImageFormat = ChooseSwapSurfaceFormat(SwapChainDetails.SCSD_Formats);

        for (size_t i = 0; i < SwapChainImages.size(); i++) {
            VkImageViewCreateInfo ImageViewCreate = {};

            ImageViewCreate.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ImageViewCreate.image    = SwapChainImages[i];
            ImageViewCreate.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ImageViewCreate.format   = SwapChainImageFormat.format;

            memset(&ImageViewCreate.components.a, VK_COMPONENT_SWIZZLE_IDENTITY, sizeof(VkComponentSwizzle) * 4);

            ImageViewCreate.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            ImageViewCreate.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            ImageViewCreate.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            ImageViewCreate.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            ImageViewCreate.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            ImageViewCreate.subresourceRange.baseMipLevel   = 0;
            ImageViewCreate.subresourceRange.levelCount     = 1;
            ImageViewCreate.subresourceRange.baseArrayLayer = 0;
            ImageViewCreate.subresourceRange.layerCount     = 1;

            ResultInfoGet(
                vkCreateImageView(HANDLE_VK_DEVICE_LOGIC, &ImageViewCreate, nullptr, &SwapChainImageViews[i]),
                RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan swapchain image_view: %u", i),
                RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan swapchain image_view: %u", i)
            );
        }
    }

    void VulkanCore::VulkanCreateDescriptorPool(uint32_t max_desc) {
        VkDescriptorPoolCreateInfo DescPoolCreate = {};

#define DESC_POOLSIZES 2
        VkDescriptorPoolSize PoolSizesConfig[DESC_POOLSIZES] = {};

        PoolSizesConfig[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        PoolSizesConfig[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        // setting max_num: UNIFORM_BUFFER.
        PoolSizesConfig[0].descriptorCount = max_desc;
        // setting max_num: COMBINED_IMAGE_SAMPLER.
        PoolSizesConfig[1].descriptorCount = max_desc / 2;

        DescPoolCreate.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        DescPoolCreate.poolSizeCount = DESC_POOLSIZES;
        DescPoolCreate.pPoolSizes    = PoolSizesConfig;
        DescPoolCreate.maxSets       = max_desc;

        ResultInfoGet(
            vkCreateDescriptorPool(HANDLE_VK_DEVICE_LOGIC, &DescPoolCreate, nullptr, &HANDLE_VK_POOL_DESC),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan descriptor_pool."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan descriptor_pool.")
        );
    }

    void VulkanCore::VulkanCreateFrameBuffer(VkRenderPass render_pass, bool enable_depth) {
        // frame_buffers size = image_views size.
        SwapChainFrameBuffers.resize(SwapChainImageViews.size());

        // craete swapchain buffers.
        for (size_t i = 0; i < SwapChainImageViews.size(); i++) {

            VkFramebufferCreateInfo FrameBufferCreate = {};
            if (enable_depth) {
                VkImageView Attachments[2] = { SwapChainImageViews[i], SwapChainDepthTex.DepthTextureView };
                FrameBufferCreate.attachmentCount = 2;
                FrameBufferCreate.pAttachments    = Attachments;
            }
            else {
                FrameBufferCreate.attachmentCount = 1;
                FrameBufferCreate.pAttachments    = &SwapChainImageViews[i];
            }
            FrameBufferCreate.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            FrameBufferCreate.renderPass = render_pass;
            FrameBufferCreate.width      = SwapChainExtent.width;
            FrameBufferCreate.height     = SwapChainExtent.height;
            FrameBufferCreate.layers     = 1;

            ResultInfoGet(
                vkCreateFramebuffer(HANDLE_VK_DEVICE_LOGIC, &FrameBufferCreate, nullptr, &SwapChainFrameBuffers[i]),
                RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan swapchain frame_buffer: %u", i),
                RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan swapchain frame_buffer: %u", i)
            );
        }
    }

    void VulkanCore::VulkanCreateCommandPool() {
        QueueFamilyIndices QueueFamilyIndices = FindQueueFamilies(HANDLE_VK_DEVICE_PHYSICAL);
        VkCommandPoolCreateInfo CommandPoolCreate = {};

        CommandPoolCreate.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        CommandPoolCreate.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        CommandPoolCreate.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();
        
        ResultInfoGet(
            vkCreateCommandPool(HANDLE_VK_DEVICE_LOGIC, &CommandPoolCreate, nullptr, &HANDLE_VK_POOL_COMMAND),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan command_pool."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan command_pool.")
        );
    }

    void VulkanCore::VulkanCreateSyncObjects() {
        VkSemaphoreCreateInfo SemaphoreCreate = {};
        SemaphoreCreate.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        ResultInfoGet(
            vkCreateSemaphore(HANDLE_VK_DEVICE_LOGIC, &SemaphoreCreate, nullptr, &SemaphoreImageAvailable),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan image_semaphore."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan image_semaphore.")
        );
        ResultInfoGet(
            vkCreateSemaphore(HANDLE_VK_DEVICE_LOGIC, &SemaphoreCreate, nullptr, &SemaphoreRenderFinished),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan render_semaphore."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan render_semaphore.")
        );

        VkFenceCreateInfo FenceCreate = {};

        FenceCreate.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FenceCreate.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        ResultInfoGet(
            vkCreateFence(HANDLE_VK_DEVICE_LOGIC, &FenceCreate, nullptr, &FenceInFlight),
            RzVkLog(LogInfo, RZVK_CORE_INIT, "create vulkan in_flight_fence."),
            RzVkLog(LogError, RZVK_CORE_INIT, "failed create vulkan in_flight_fence.")
        );
    }

    void VulkanCore::DynamicRecreateSwapChain() {
        vkDeviceWaitIdle(HANDLE_VK_DEVICE_LOGIC);

        VulkanFreeDeleteSwapChain();
        SwapChainExtent = { SystemWindowSize.vector_x, SystemWindowSize.vector_y };

        VulkanCreateSwapchain();
        VulkanCreateSwapchainImages();
        VulkanCreateSwapchainImageViews();
        VulkanCreateFrameBuffer(PresentationRenderPass, SwapChainDepthTexFlag);
    }

    void VulkanCore::VulkanFreeDeleteSwapChain() {
        // delete swapchain frame_buffer.
        for (auto FrameBuffer : SwapChainFrameBuffers)
            vkDestroyFramebuffer(HANDLE_VK_DEVICE_LOGIC, FrameBuffer, nullptr);
        // delete swapchain image_views.
        for (auto ImageView : SwapChainImageViews)
            vkDestroyImageView(HANDLE_VK_DEVICE_LOGIC, ImageView, nullptr);

        // delete swapchain & surface.
        vkDestroySwapchainKHR(HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_SWAPCHAIN, nullptr);
    }

    void VulkanCore::VulkanFreeDelete() {
        TextureSamplers::TextureSamplersManager CreateSamplers;
        // delete texture_samplers.
        CreateSamplers.VKFC_TextureSamplersDelete();
        // controller => delete resource_maps.
        DeleteDynamicResourceManager();

        vkDeviceWaitIdle(HANDLE_VK_DEVICE_LOGIC);

        vkDestroySemaphore(HANDLE_VK_DEVICE_LOGIC, SemaphoreImageAvailable, nullptr);
        vkDestroySemaphore(HANDLE_VK_DEVICE_LOGIC, SemaphoreRenderFinished, nullptr);

        vkDestroyFence(HANDLE_VK_DEVICE_LOGIC, FenceInFlight, nullptr);

        vkDestroyDescriptorPool(HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_POOL_DESC, nullptr);
        vkDestroyCommandPool(HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_POOL_COMMAND, nullptr);

        // delete swapchain & surface.
        VulkanFreeDeleteSwapChain();
        vkDestroySurfaceKHR(HANDLE_VK_INSTANCE, HANDLE_VK_SURFACE, nullptr);
        // delete logic_device.
        vkDestroyDevice(HANDLE_VK_DEVICE_LOGIC, nullptr);
        // delete validation_layer(debug).
        VkFunctionLoader::VKDestroyDebugUtilsMessengerEXT(HANDLE_VK_INSTANCE, DebugInstance, nullptr);
        // delete instance.
        vkDestroyInstance(HANDLE_VK_INSTANCE, nullptr);

        RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_CORE_INIT, "rz_vulkan framework free delete."));
    }

    void VulkanCore::RzVulkanGetWindowHD(HWND window_hd, HINSTANCE module_hd) {
        VulkanWindowConfig.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        VulkanWindowConfig.hwnd      = window_hd;
        VulkanWindowConfig.hinstance = module_hd;
    }

#if defined(_WIN32) && defined(GLFW_VERSION_MAJOR)
    void VulkanCore::RzVulkanWin32GLFWwindowHD(GLFWwindow* window) {
        WindowObjectPtr = window;
        // windows: get module_hd, glfw_libray: get window_hd.
        VulkanWindowConfig.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        VulkanWindowConfig.hwnd      = glfwGetWin32Window(window);
        VulkanWindowConfig.hinstance = GetModuleHandle(nullptr);
        // register callback_function.
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
    }

    void VulkanCore::RzVulkanWin32FLFWwindowResize(FmtVector2T<uint32_t> new_size) {
        glfwSetWindowSize(WindowObjectPtr, (int)new_size.vector_x, (int)new_size.vector_y);
        DynamicRecreateSwapChain();
    }
#endif

    FmtVector2T<VkFormat> VulkanCore::RzVulkanFrameworkInit(bool depth, uint32_t max_desc) {
        VulkanCreateInstance();
        // create device: 0'VulkanCreateWindowSurface' + 1'VulkanCreatePresentation'
        // find device => 0 -> 1 => create device.
        bool EXT_FLAG = VulkanCreateDevice();
        VulkanCreateDeviceQueue();

        VulkanSwapChainSupport(EXT_FLAG);
        VulkanCreateSwapchain();
        VulkanCreateSwapchainImages();
        VulkanCreateSwapchainImageViews();
        VulkanCreateDescriptorPool(max_desc);

        TextureSamplers::TextureSamplersManager CreateSamplers;
        // create texture_samplers.
        CreateSamplers.VKFC_TextureSamplersCreate();

        VulkanCreateDepthTexture CreateDepthTex;
        bool CreateState = false;

        CreateDepthTex.CreateDepthTexture(SystemWindowSize);
        SwapChainDepthTex = CreateDepthTex.__MS_STORAGE(&CreateState);

        // controller => create resource_maps.
        CreateDynamicResourceManager(128);

        VKLRES_KEY TempKey = NULL;
        // 核心框架深度贴图: 贯穿生命周期 => 由 RES 管理器自动销毁.
        GetRzVkFrameworkResourcePtr()->PointerDTR->ResourceStorage(&TempKey, &CreateDepthTex);

        // create depth_texture error.
        if (!CreateState) {
            RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_CORE_INIT, "rz_vulkan framework depth failed create."));
            // depth_texture format invaild. [VK_FORMAT_UNDEFINED].
            return FmtVector2T<VkFormat>(ChooseSwapSurfaceFormat(SwapChainDetails.SCSD_Formats).format, VK_FORMAT_UNDEFINED);
        }
        SwapChainDepthTexFlag = depth;

        RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_CORE_INIT, "rz_vulkan framework init_complete."));
        return FmtVector2T<VkFormat>(
            ChooseSwapSurfaceFormat(SwapChainDetails.SCSD_Formats).format,
            SwapChainDepthTex.DepthTextureFormat
        );
    }

    bool VulkanCore::RzVulkanFrameworkPassLoad(VkRenderPass presentation_pass) {
        if (presentation_pass == VK_NULL_HANDLE) {
            RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CORE_INIT, "rz_vulkan framework load_pass = null."));
            return false;
        }
        PresentationRenderPass = presentation_pass;
        return true;
    }

    void VulkanCore::RzVulkanFrameworkStart() {
        VulkanCreateFrameBuffer(PresentationRenderPass, SwapChainDepthTexFlag);
        VulkanCreateCommandPool();
        VulkanCreateSyncObjects();
        RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_CORE_INIT, "rz_vulkan framework create."));
    }

    // semaphores: out window => color_attachment.
    constexpr VkPipelineStageFlags WaitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    RenderCommand::CommandSubmit VulkanCore::RzVulkanGetOutRenderCmdSubmitConfig() {
        RenderCommand::CommandSubmit SubmitCreate = {};

        SubmitCreate.CommandSubmitConfig.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        SubmitCreate.CommandSubmitConfig.waitSemaphoreCount = 1;
        SubmitCreate.CommandSubmitConfig.pWaitSemaphores    = &SemaphoreImageAvailable;
        SubmitCreate.CommandSubmitConfig.pWaitDstStageMask  = &WaitStages;

        SubmitCreate.CommandSubmitConfig.signalSemaphoreCount = 1;
        SubmitCreate.CommandSubmitConfig.pSignalSemaphores    = &SemaphoreRenderFinished;

        SubmitCreate.CommandSubmitFence = FenceInFlight;
        return SubmitCreate;
    }

    RZVK_RESOURCE* VulkanCore::RzVulkanGetResourcePtr() {
        return GetRzVkFrameworkResourcePtr();
    }

    VkFramebuffer VulkanCore::RZVK_DrawFrameContextBegin() {
        vkWaitForFences(HANDLE_VK_DEVICE_LOGIC, 1, &FenceInFlight, VK_TRUE, UINT64_MAX);
        DrawImageIndexTemp = NULL;

        VkResult Result = vkAcquireNextImageKHR(
            HANDLE_VK_DEVICE_LOGIC, HANDLE_VK_SWAPCHAIN,
            UINT64_MAX, SemaphoreImageAvailable, VK_NULL_HANDLE, &DrawImageIndexTemp
        );
        vkResetFences(HANDLE_VK_DEVICE_LOGIC, 1, &FenceInFlight);
        return SwapChainFrameBuffers[DrawImageIndexTemp];
    }

    void VulkanCore::RZVK_DrawFrameContextEnd() {
        VkPresentInfoKHR PresentCreate = {};

        PresentCreate.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        PresentCreate.waitSemaphoreCount = 1;
        PresentCreate.pWaitSemaphores    = &SemaphoreRenderFinished;

        PresentCreate.swapchainCount = 1;
        PresentCreate.pSwapchains    = &HANDLE_VK_SWAPCHAIN;
        PresentCreate.pImageIndices  = &DrawImageIndexTemp;
        PresentCreate.pResults       = nullptr;

        vkQueuePresentKHR(HANDLE_VK_QUEUE_PRESENT, &PresentCreate);
    }
}