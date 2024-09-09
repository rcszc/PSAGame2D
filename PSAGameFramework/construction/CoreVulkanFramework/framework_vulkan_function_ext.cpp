// framework_vulkan_function_ext.
#include "framework_vulkan.hpp"

using namespace std;

namespace VkFunctionLoader {
    RZ_VULKAN_DEFLABEL RZVK_CORE_VALIDATION = "rzvk_validation";

    // func(callback): "vulkan validation debug".
    VKAPI_ATTR VkBool32 VKAPI_CALL VKDebugCallbackPrint(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* ptr_callback_data,
        void* ptr_user_data
    ) {
        constexpr const char* OutLoggerStr = "vk_code: %u, msg: %s";
        LOGLABEL LEVEL = LogInfo;
        switch (message_severity) {
        case(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT):    { LEVEL = LogInfo;    break; }
        case(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT): { LEVEL = LogWarning; break; }
        case(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT):   { LEVEL = LogError;   break; }
        }
        auto Logger = RZ_VULKAN::RzVkLog(LEVEL, RZVK_CORE_VALIDATION, OutLoggerStr, message_type, ptr_callback_data->pMessage);
        RZVK_FUNC_LOGGER(Logger);
        // const: false. [20240828]
        return VK_FALSE;
    }

    // func: "vkCreateDebugUtilsMessengerEXT" 需要手动加载.
    VkResult VKCreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* ptr_create_info,
        const VkAllocationCallbacks* ptr_allocator,
        VkDebugUtilsMessengerEXT* ptr_debug_messenger
    ) {
        auto ExtFunction = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (ExtFunction != nullptr)
            return ExtFunction(instance, ptr_create_info, ptr_allocator, ptr_debug_messenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // func: "vkDestroyDebugUtilsMessengerEXT" 需要手动加载.
    void VKDestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debug_messenger,
        const VkAllocationCallbacks* ptr_allocator
    ) {
        auto ExtFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (ExtFunction != nullptr)
            ExtFunction(instance, debug_messenger, ptr_allocator);
    }
}