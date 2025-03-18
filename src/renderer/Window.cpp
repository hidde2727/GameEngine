#include "renderer/window.h"

namespace Engine{
namespace Renderer{
    
    Window::Window() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

        Vulkan::ContextCreationInfo contextInfo;
        contextInfo.SetExtensions({}, true);
        contextInfo.SetValidationLayers({"VK_LAYER_KHRONOS_validation"}, &DebugCallback);
        contextInfo.SetNeccesaryQueues({
            Vulkan::QueueType::GraphicsQueue,
            Vulkan::QueueType::KHRPresentQueue,
            Vulkan::QueueType::TransferQueue
        });
        contextInfo.SetDeviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        _vkContext.Init(contextInfo, _window);
        
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);
        _vkSwapchain.Init(_vkContext, (uint32_t)width, (uint32_t)height);

        Vulkan::PipelineCreator pipelineInfo;
        pipelineInfo.SetShaders({ "/resources/engine/shaders/shader.vert", "/resources/engine/shaders/shader.frag" });
        _vkPipeline.Init(pipelineInfo, _vkContext);
    }
    Window::~Window() {
        _vkPipeline.Cleanup(_vkContext);
        _vkSwapchain.Cleanup(_vkContext);
        _vkContext.CleanUp();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(_window);
    }
    void Window::Update() {
        glfwPollEvents();
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Window::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
    
        if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            //LOG(pCallbackData->pMessage);
        } else if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            INFO(pCallbackData->pMessage);
        } else if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            ERROR(pCallbackData->pMessage);
        } else if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            ERROR(pCallbackData->pMessage);
        }
    
        return VK_FALSE;
    }

}
}