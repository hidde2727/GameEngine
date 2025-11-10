#ifndef ENGINE_RENDERER_VULKAN_CONTEXT_H
#define ENGINE_RENDERER_VULKAN_CONTEXT_H

#include "core/PCH.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    #define ENGINE_RENDERER_VULKAN_DEBUG_CALLBACK VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData) 
    enum QueueType {
        None = 0,
        GraphicsQueue = VK_QUEUE_GRAPHICS_BIT,
        ComputeQueue = VK_QUEUE_COMPUTE_BIT,
        TransferQueue = VK_QUEUE_TRANSFER_BIT,
        ParseBindingQueue = VK_QUEUE_SPARSE_BINDING_BIT,
        ProtectedQueue = VK_QUEUE_PROTECTED_BIT,
        // 32 and 64 are reserved for vulkan
        KHRPresentQueue = 0x00000080
    };

    typedef VkPhysicalDeviceFeatures DeviceFeatures;
    class ContextCreationInfo;
    typedef std::optional<uint32_t> QueueFamilyIndices;



// Creates a context with the specified info and for the window
// You cannot use this context with another window when using it with a swapchain (and specified you are doing so in the creationInfo)
    class Context {
    public:

        Context();
        void Init(ContextCreationInfo& info, GLFWwindow* window=nullptr);
        void CleanUp();

        void WaitIdle();
        void WaitQueueIdle(const QueueType type);

        VkDevice GetDevice() const { return _device; }
        VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
        VkQueue GetQueue(const QueueType type) {
            if(!_queues.contains(type)) return VK_NULL_HANDLE;
            return _queues[type]; 
        }

        VkPhysicalDeviceProperties GetPhysicalDeviceProperties();

    private:
        friend class RenderPass;
        friend class Swapchain;
        friend class Pipeline;
        friend class CommandBuffer;
        friend class BaseBuffer;
        friend class EfficientGPUBuffer;
        friend class Texture;
        friend class TextureSampler;

        QueueFamilyIndices GetQueueFamily(const QueueType queueType, const size_t amountQueues);
        int RateDevice(const VkPhysicalDevice device, const ContextCreationInfo info);

        VkInstance _instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device = VK_NULL_HANDLE;

        VkSurfaceKHR _surfaceKHR = VK_NULL_HANDLE;

        std::map<QueueType, VkQueue> _queues;
        std::map<QueueType, uint32_t> _queueFamilies;
        std::vector<VkCommandPool> _commandPools;
        std::map<QueueType, VkCommandPool> _queueTypeToCommandPools;

        VmaAllocator _allocator = VK_NULL_HANDLE;
    };

    class ContextCreationInfo {
    public:
        
        ContextCreationInfo();
        void SetExtensions(const std::initializer_list<const char*> extensions, const bool glfw);
        void SetValidationLayers(const std::initializer_list<const char*> layer);
        void SetDebugCallback(ENGINE_RENDERER_VULKAN_DEBUG_CALLBACK);

        void SetNeccesaryQueues(const std::initializer_list<QueueType> queues);
        void SetDeviceFeatures(const VkPhysicalDeviceFeatures deviceFeatures);
        void SetDeviceExtensions(const std::initializer_list<const char*> extensions);
        void SetNeedsSwapChainSupport();

    private:
        friend Context;

        // Instance + debugging messenger
        VkApplicationInfo _appInfo{};
        VkInstanceCreateInfo _instanceInfo{};
        VkDebugUtilsMessengerCreateInfoEXT _debugMessengerInfo{};
        std::vector<const char*> _extensions;
        std::vector<const char*> _validationLayers;

        // Device
        std::vector<QueueType> _queues;
        VkPhysicalDeviceFeatures _deviceFeatures{};
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT _indexingFeatures{};
        std::vector<const char*> _deviceExtensions;
        bool _needsSwapchain = false;
        VkDeviceCreateInfo _deviceInfo{};
        std::vector<VkDeviceQueueCreateInfo> _queueInfos;
        const float _queuePriority = 1.0f;

    };

}
}
}

#endif