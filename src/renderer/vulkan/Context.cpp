#include "renderer/vulkan/Context.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

namespace Engine{
namespace Renderer{
namespace Vulkan{

    Context::Context() { }
    void Context::Init(ContextCreationInfo& info, GLFWwindow* window) {
        // Create instance
        VkResult result = vkCreateInstance(&info._instanceInfo, nullptr, &_instance);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan instance");

        if(info._debugMessengerInfo.pfnUserCallback != nullptr) {
            result = CreateDebugUtilsMessengerEXT(_instance, &info._debugMessengerInfo, nullptr, &_debugMessenger);
            ASSERT(result != VK_SUCCESS, "Failed to create vulkan debug messenger");
        }

        if(window) {
            result = glfwCreateWindowSurface(_instance, window, nullptr, &_surfaceKHR);
            ASSERT(result != VK_SUCCESS, "Failed to create vulkan surface for GLFW window");
        }

        // Get all available devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        ASSERT(deviceCount==0, "No vulkan physical devices found");
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

        // Pick the best one
        std::multimap<int, VkPhysicalDevice> candidates;
        for(const VkPhysicalDevice device : physicalDevices) {
            candidates.insert(std::pair<int, VkPhysicalDevice>(RateDevice(device, info), device));
        }
        ASSERT(candidates.rbegin()->first <= 0, "No fitting vulkan physical device found");
        _physicalDevice = candidates.rbegin()->second;

        // Create the logical device
        std::set<uint32_t> uniqueQueueFamilies;
        for(size_t i = 0; i < info._queueInfos.size(); i++) {
            QueueFamilyIndices queueFamily=GetQueueFamily(info._queues[i], 1);
            ASSERT(!queueFamily.has_value(), "No queue family found for a selected vulkan physical device");
            uniqueQueueFamilies.insert(queueFamily.value());
            _queueFamilies[info._queues[i]] = queueFamily.value();
        }
        info._queueInfos.resize(uniqueQueueFamilies.size());
        size_t i = 0;
        for(const uint32_t queueFamily : uniqueQueueFamilies) {
            info._queueInfos[i].queueFamilyIndex = queueFamily;
            i++;
        }
        info._deviceInfo.pQueueCreateInfos = info._queueInfos.data();
        info._deviceInfo.queueCreateInfoCount = (uint32_t)info._queueInfos.size();

        result = vkCreateDevice(_physicalDevice, &info._deviceInfo, nullptr, &_device);
        ASSERT(result, "Failed to create vulkan device");

        // Create the queues
        std::map<uint32_t, VkQueue> queueFamilyToQueue;
        for(const uint32_t queueFamily : uniqueQueueFamilies) {
            VkQueue queue = VK_NULL_HANDLE;
            vkGetDeviceQueue(_device, queueFamily, 0, &queue);
            queueFamilyToQueue[queueFamily] = queue;            
        }
        for(const auto&[type, queueFamily] : _queueFamilies) {
            _queues[type] = queueFamilyToQueue[_queueFamilies[type]];
        }
    }

    int Context::RateDevice(const VkPhysicalDevice device, const ContextCreationInfo info) {
        _physicalDevice = device;
        for(QueueType queueInfo : info._queues) {
            if(!GetQueueFamily(queueInfo, 1).has_value()) return 0;
        }

        uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		for (const char* extensionName : info._deviceExtensions) {
			bool extensionFound = false;
			for (const auto& extensionProperties : availableExtensions) {
				if (strcmp(extensionName, extensionProperties.extensionName) == 0) {
					extensionFound = true;
					break;
				}
			}
			if (!extensionFound) return 0;
		}

        if(info._needsSwapchain) {
            uint32_t formatCount;
		    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surfaceKHR, &formatCount, nullptr);
            if(formatCount == 0) return 0;
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surfaceKHR, &presentModeCount, nullptr);
            if(presentModeCount==0) return 0;
        }

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
        if(info._deviceFeatures.robustBufferAccess                      && !features.robustBufferAccess) return 0;
        if(info._deviceFeatures.fullDrawIndexUint32                     && !features.fullDrawIndexUint32) return 0;
        if(info._deviceFeatures.imageCubeArray                          && !features.imageCubeArray) return 0;
        if(info._deviceFeatures.independentBlend                        && !features.independentBlend) return 0;
        if(info._deviceFeatures.geometryShader                          && !features.geometryShader) return 0;
        if(info._deviceFeatures.tessellationShader                      && !features.tessellationShader) return 0;
        if(info._deviceFeatures.sampleRateShading                       && !features.sampleRateShading) return 0;
        if(info._deviceFeatures.dualSrcBlend                            && !features.dualSrcBlend) return 0;
        if(info._deviceFeatures.logicOp                                 && !features.logicOp) return 0;
        if(info._deviceFeatures.multiDrawIndirect                       && !features.multiDrawIndirect) return 0;
        if(info._deviceFeatures.drawIndirectFirstInstance               && !features.drawIndirectFirstInstance) return 0;
        if(info._deviceFeatures.depthClamp                              && !features.depthClamp) return 0;
        if(info._deviceFeatures.depthBiasClamp                          && !features.depthBiasClamp) return 0;
        if(info._deviceFeatures.fillModeNonSolid                        && !features.fillModeNonSolid) return 0;
        if(info._deviceFeatures.depthBounds                             && !features.depthBounds) return 0;
        if(info._deviceFeatures.wideLines                               && !features.wideLines) return 0;
        if(info._deviceFeatures.largePoints                             && !features.largePoints) return 0;
        if(info._deviceFeatures.alphaToOne                              && !features.alphaToOne) return 0;
        if(info._deviceFeatures.multiViewport                           && !features.multiViewport) return 0;
        if(info._deviceFeatures.samplerAnisotropy                       && !features.samplerAnisotropy) return 0;
        if(info._deviceFeatures.textureCompressionETC2                  && !features.textureCompressionETC2) return 0;
        if(info._deviceFeatures.textureCompressionASTC_LDR              && !features.textureCompressionASTC_LDR) return 0;
        if(info._deviceFeatures.textureCompressionBC                    && !features.textureCompressionBC) return 0;
        if(info._deviceFeatures.occlusionQueryPrecise                   && !features.occlusionQueryPrecise) return 0;
        if(info._deviceFeatures.pipelineStatisticsQuery                 && !features.pipelineStatisticsQuery) return 0;
        if(info._deviceFeatures.vertexPipelineStoresAndAtomics          && !features.vertexPipelineStoresAndAtomics) return 0;
        if(info._deviceFeatures.fragmentStoresAndAtomics                && !features.fragmentStoresAndAtomics) return 0;
        if(info._deviceFeatures.shaderTessellationAndGeometryPointSize  && !features.shaderTessellationAndGeometryPointSize) return 0;
        if(info._deviceFeatures.shaderImageGatherExtended               && !features.shaderImageGatherExtended) return 0;
        if(info._deviceFeatures.shaderStorageImageExtendedFormats       && !features.shaderStorageImageExtendedFormats) return 0;
        if(info._deviceFeatures.shaderStorageImageMultisample           && !features.shaderStorageImageMultisample) return 0;
        if(info._deviceFeatures.shaderStorageImageReadWithoutFormat     && !features.shaderStorageImageReadWithoutFormat) return 0;
        if(info._deviceFeatures.shaderStorageImageWriteWithoutFormat    && !features.shaderStorageImageWriteWithoutFormat) return 0;
        if(info._deviceFeatures.shaderUniformBufferArrayDynamicIndexing && !features.shaderUniformBufferArrayDynamicIndexing) return 0;
        if(info._deviceFeatures.shaderSampledImageArrayDynamicIndexing  && !features.shaderSampledImageArrayDynamicIndexing) return 0;
        if(info._deviceFeatures.shaderStorageBufferArrayDynamicIndexing && !features.shaderStorageBufferArrayDynamicIndexing) return 0;
        if(info._deviceFeatures.shaderStorageImageArrayDynamicIndexing  && !features.shaderStorageImageArrayDynamicIndexing) return 0;
        if(info._deviceFeatures.shaderClipDistance                      && !features.shaderClipDistance) return 0;
        if(info._deviceFeatures.shaderCullDistance                      && !features.shaderCullDistance) return 0;
        if(info._deviceFeatures.shaderFloat64                           && !features.shaderFloat64) return 0;
        if(info._deviceFeatures.shaderInt64                             && !features.shaderInt64) return 0;
        if(info._deviceFeatures.shaderInt16                             && !features.shaderInt16) return 0;
        if(info._deviceFeatures.shaderResourceResidency                 && !features.shaderResourceResidency) return 0;
        if(info._deviceFeatures.shaderResourceMinLod                    && !features.shaderResourceMinLod) return 0;
        if(info._deviceFeatures.sparseBinding                           && !features.sparseBinding) return 0;
        if(info._deviceFeatures.sparseResidencyBuffer                   && !features.sparseResidencyBuffer) return 0;
        if(info._deviceFeatures.sparseResidencyImage2D                  && !features.sparseResidencyImage2D) return 0;
        if(info._deviceFeatures.sparseResidencyImage3D                  && !features.sparseResidencyImage3D) return 0;
        if(info._deviceFeatures.sparseResidency2Samples                 && !features.sparseResidency2Samples) return 0;
        if(info._deviceFeatures.sparseResidency4Samples                 && !features.sparseResidency4Samples) return 0;
        if(info._deviceFeatures.sparseResidency8Samples                 && !features.sparseResidency8Samples) return 0;
        if(info._deviceFeatures.sparseResidency16Samples                && !features.sparseResidency16Samples) return 0;
        if(info._deviceFeatures.sparseResidencyAliased                  && !features.sparseResidencyAliased) return 0;
        if(info._deviceFeatures.variableMultisampleRate                 && !features.variableMultisampleRate) return 0;
        if(info._deviceFeatures.inheritedQueries                        && !features.inheritedQueries) return 0;

		return 1;
    }

    QueueFamilyIndices Context::GetQueueFamily(const QueueType queueType, const size_t amountQueues) {
        uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if(queueType==QueueType::KHRPresentQueue) {
                ASSERT(_surfaceKHR==VK_NULL_HANDLE, "Vulkan surface not set");
                VkBool32 KHRPresentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surfaceKHR, &KHRPresentSupport);
                if(KHRPresentSupport) return QueueFamilyIndices(i);
            }
            else if (queueFamily.queueFlags & queueType && queueFamily.queueCount >= amountQueues) {
                return QueueFamilyIndices(i);
            }
            i++;
        }
        return QueueFamilyIndices();
    }

    void Context::CleanUp() {
        vkDestroyDevice(_device, nullptr);
        vkDestroySurfaceKHR(_instance, _surfaceKHR, nullptr);
        if(_debugMessenger != VK_NULL_HANDLE) DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        vkDestroyInstance(_instance, nullptr);
    }






    ContextCreationInfo::ContextCreationInfo() {
        _appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        _appInfo.pApplicationName = "Hidde's game engine";
        _appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        _appInfo.pEngineName = "No Engine";
        _appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        _appInfo.apiVersion = VK_API_VERSION_1_0;

        _instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        _instanceInfo.pApplicationInfo = &_appInfo;
        _instanceInfo.enabledExtensionCount = 0;
        _instanceInfo.ppEnabledExtensionNames = nullptr;
        _instanceInfo.enabledLayerCount = 0;

        _debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        _debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        _debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        _debugMessengerInfo.pfnUserCallback = nullptr;
        _debugMessengerInfo.pUserData = nullptr;
 
        _deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        _deviceInfo.pQueueCreateInfos = nullptr;
        _deviceInfo.queueCreateInfoCount = 0;
        _deviceInfo.pEnabledFeatures = &_deviceFeatures;
        _deviceInfo.ppEnabledExtensionNames = nullptr;
        _deviceInfo.enabledExtensionCount = 0;
    }

    void ContextCreationInfo::SetExtensions(const std::initializer_list<const char*> extensions, const bool glfw) {
        _extensions = extensions;

        if(glfw) {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            _extensions.insert(_extensions.end(), glfwExtensions, glfwExtensions+glfwExtensionCount);
        }

        _instanceInfo.enabledExtensionCount = (uint32_t)_extensions.size();
        _instanceInfo.ppEnabledExtensionNames = _extensions.data();
    }

    void ContextCreationInfo::SetValidationLayers(const std::initializer_list<const char*> layer, ENGINE_RENDERER_VULKAN_DEBUG_CALLBACK) {
        _validationLayers = layer;

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        bool notFound=false;
        for(const char* layer : layer) {
            bool found=false;
            for(const VkLayerProperties available : availableLayers) {
                if(strcmp(layer, available.layerName)) { found=true; break; }
            }
            if(!found) { notFound=true; break; }
        }
        ASSERT(notFound, "Requested validation layer not available")

        _instanceInfo.enabledLayerCount = (uint32_t)_validationLayers.size();
        _instanceInfo.ppEnabledLayerNames = _validationLayers.data();
        _deviceInfo.enabledLayerCount = (uint32_t)_validationLayers.size();
        _deviceInfo.ppEnabledLayerNames = _validationLayers.data();

        _debugMessengerInfo.pfnUserCallback = debugCallback;

        // Add to extensions
        _extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        _instanceInfo.enabledExtensionCount = (uint32_t)_extensions.size();
        _instanceInfo.ppEnabledExtensionNames = _extensions.data();
    }

    void ContextCreationInfo::SetDeviceFeatures(const VkPhysicalDeviceFeatures deviceFeatures) {
        _deviceFeatures = deviceFeatures;
    }

    void ContextCreationInfo::SetDeviceExtensions(const std::initializer_list<const char*> extensions) {
        _deviceExtensions = extensions;
        _deviceInfo.ppEnabledExtensionNames = _deviceExtensions.data();
        _deviceInfo.enabledExtensionCount = (uint32_t)_deviceExtensions.size();
    }

    void ContextCreationInfo::SetNeccesaryQueues(const std::initializer_list<QueueType> queues) {
        _queues = queues;
        _queueInfos.reserve(_queues.size());
        for(const QueueType info : _queues) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = 0;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &_queuePriority;
            _queueInfos.push_back(queueCreateInfo);
        }
        _deviceInfo.pQueueCreateInfos = _queueInfos.data();
        _deviceInfo.queueCreateInfoCount = (uint32_t)_queueInfos.size();
    }

    void ContextCreationInfo::SetNeedsSwapChainSupport() {
        _needsSwapchain = true;
    }

}
}
}