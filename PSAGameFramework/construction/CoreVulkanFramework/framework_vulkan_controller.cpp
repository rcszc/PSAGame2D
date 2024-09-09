// framework_vulkan_controller.
#include "framework_vulkan.hpp"

using namespace std;

namespace RZ_VULKAN {
	namespace CONTROLLER {
		RZ_VULKAN_DEFLABEL RZVK_CONTROLLER = "rzvk_controller";

		atomic<size_t> VulkanCoreControllerCounter::ObjectsRenderPass    = NULL;
		atomic<size_t> VulkanCoreControllerCounter::ObjectsRenderCommand = NULL;

		template<typename T>
		bool DELETE_MANAGER_POINTER(T* object, const char* tag) {
			if (object == nullptr) {
				RZVK_FUNC_LOGGER(RzVkLog(LogError, RZVK_CONTROLLER, "delete resource_map(%s) obj = nullptr.", tag));
				return false;
			}
			delete object; object = nullptr;
			return true;
		}

		void VulkanCoreController::CreateDynamicResourceManager(size_t init_map) {
			if (init_map < 16) {
				RZVK_FUNC_LOGGER(RzVkLog(LogWarning, RZVK_CONTROLLER, "set resource_map bucket < 16."));
				init_map = 16;
			}
			VulkanFrameworkResource.PointerVAR = new RZ_VULKAN_RESOURCE::ResourceVkVertexAttribute(init_map);
			VulkanFrameworkResource.PointerVBR = new RZ_VULKAN_RESOURCE::ResourceVkVertexBuffer   (init_map);
			VulkanFrameworkResource.PointerUBR = new RZ_VULKAN_RESOURCE::ResourceVkUniformBuffer  (init_map);
			VulkanFrameworkResource.PointerSLR = new RZ_VULKAN_RESOURCE::ResourceVkShaderLayout   (init_map);
			VulkanFrameworkResource.PointerSPR = new RZ_VULKAN_RESOURCE::ResourceVkShader         (init_map);
			VulkanFrameworkResource.PointerCTR = new RZ_VULKAN_RESOURCE::ResourceVkColorTexture   (init_map);
			VulkanFrameworkResource.PointerDTR = new RZ_VULKAN_RESOURCE::ResourceVkDepthTexture   (init_map);

			RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_CONTROLLER, "create resource_maps."));
			VulkanFrameworkResourceValidFlag = true;
		}

		bool VulkanCoreController::DeleteDynamicResourceManager() {
			bool DeleteFailed = false;

			DeleteFailed |= !DELETE_MANAGER_POINTER(VulkanFrameworkResource.PointerVAR, "va");
			DeleteFailed |= !DELETE_MANAGER_POINTER(VulkanFrameworkResource.PointerVBR, "vb");
			DeleteFailed |= !DELETE_MANAGER_POINTER(VulkanFrameworkResource.PointerUBR, "ub");
			DeleteFailed |= !DELETE_MANAGER_POINTER(VulkanFrameworkResource.PointerSLR, "sl");
			DeleteFailed |= !DELETE_MANAGER_POINTER(VulkanFrameworkResource.PointerSPR, "sp");
			DeleteFailed |= !DELETE_MANAGER_POINTER(VulkanFrameworkResource.PointerCTR, "ct");
			DeleteFailed |= !DELETE_MANAGER_POINTER(VulkanFrameworkResource.PointerDTR, "dt");

			RZVK_FUNC_LOGGER(RzVkLog(LogTrace, RZVK_CONTROLLER, "delete resource_maps."));
			VulkanFrameworkResourceValidFlag = false;
			return !DeleteFailed;
		}

		ControllerStateQuery VulkanCoreController::QueryRzVkFrameworkInfo() {
			ControllerStateQuery StateTemp = {};

			StateTemp.ObjectsRenderPass    = ObjectsRenderPass;
			StateTemp.ObjectsRenderCommand = ObjectsRenderCommand;

			// format framework version_str.
			ostringstream OSS;
			OSS << DEF_RZVULKAN_NAME << " v " << DEF_RZVULKAN_VER[0] << "." << DEF_RZVULKAN_VER[1] << "." << DEF_RZVULKAN_VER[2];
			StateTemp.RzFrameworkVersion = OSS.str();

			// resource_maps size.
			if (VulkanFrameworkResourceValidFlag) {
				StateTemp.OnlineResourceItems["VAR"] = VulkanFrameworkResource.PointerVAR->ResourceSize();
				StateTemp.OnlineResourceItems["VBR"] = VulkanFrameworkResource.PointerVBR->ResourceSize();
				StateTemp.OnlineResourceItems["UBR"] = VulkanFrameworkResource.PointerUBR->ResourceSize();
				StateTemp.OnlineResourceItems["SLR"] = VulkanFrameworkResource.PointerSLR->ResourceSize();
				StateTemp.OnlineResourceItems["SPR"] = VulkanFrameworkResource.PointerSPR->ResourceSize();
				StateTemp.OnlineResourceItems["CTR"] = VulkanFrameworkResource.PointerCTR->ResourceSize();
				StateTemp.OnlineResourceItems["DTR"] = VulkanFrameworkResource.PointerDTR->ResourceSize();
			}
			return StateTemp;
		}

		ControllerPointerFamily* VulkanCoreController::GetRzVkFrameworkResourcePtr() {
			return &VulkanFrameworkResource;
		}
	}
}