#include "renderer/window.h"

namespace Engine {
namespace Renderer {
    
    Window::Window() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, FramebufferResize);

        Vulkan::ContextCreationInfo contextInfo;
        contextInfo.SetExtensions({}, true);
        contextInfo.SetValidationLayers({"VK_LAYER_KHRONOS_validation"}, &DebugCallback);
        contextInfo.SetNeccesaryQueues({
            Vulkan::QueueType::GraphicsQueue,
            Vulkan::QueueType::KHRPresentQueue
        });
        contextInfo.SetDeviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        _vkContext.Init(contextInfo, _window);
        

        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);
        _vkSwapchain.Init(_vkContext, (uint32_t)width, (uint32_t)height);

        _vkRenderPass.Init(_vkContext, _vkSwapchain);

        _vkSwapchain.Resize(_vkContext, _vkRenderPass, width, height);

        Vulkan::PipelineCreator pipelineInfo;
        pipelineInfo.SetShaders({ "resources/engine/shaders/shader.vert", "resources/engine/shaders/shader.frag" });
        pipelineInfo.SetVertexInput({ Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec3 });
        pipelineInfo.SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        _vkPipeline.Init(pipelineInfo, _vkContext, _vkRenderPass, 0);

        _vkCommandBuffer.Init(_vkContext, Vulkan::QueueType::GraphicsQueue, 2);
        _vkInFlightFence = _vkCommandBuffer.CreateFence(_vkContext, true);
        _vkImageAvailableSemaphore = _vkCommandBuffer.CreateSemaphore(_vkContext);
        _vkRenderFinishedSemaphore = _vkCommandBuffer.CreateSemaphore(_vkContext);

        _vkVertexBuffer.Init(_vkContext, 1, true);
        _vkTransferBuffer.Init(_vkContext, sizeof(Vertex)*3);
        const std::vector<Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        _vkTransferBuffer.SetData(_vkContext, vertices);
        _vkTransferBuffer.CopyTo(_vkContext, &_vkVertexBuffer);
    }
    Window::~Window() {
        _vkContext.WaitIdle();

        _vkVertexBuffer.Cleanup(_vkContext);
        _vkTransferBuffer.Cleanup(_vkContext);

        _vkCommandBuffer.Cleanup(_vkContext);
        _vkPipeline.Cleanup(_vkContext);
        _vkSwapchain.Cleanup(_vkContext);
        _vkRenderPass.Cleanup(_vkContext);
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
    void Window::Draw() {
        if(_framebufferResized) {
            _framebufferResized = false; 
            int width = 0, height = 0;
            glfwGetFramebufferSize(_window, &width, &height);
            if(width==0 && height==0) return;
            _vkSwapchain.Resize(_vkContext, _vkRenderPass, width, height);
        }

        _vkCommandBuffer.AcquireNextSwapchainFrame(_vkContext, _vkSwapchain, _vkImageAvailableSemaphore);
        _vkCommandBuffer.WaitFence(_vkContext, _vkInFlightFence);

        _vkCommandBuffer.StartRecording(_vkContext);
        _vkCommandBuffer.BeginRenderPass(_vkRenderPass, _vkSwapchain, {{{0,0,0,1.f}}}, true);
        _vkCommandBuffer.BindGraphicsPipeline(_vkPipeline);
        _vkCommandBuffer.BindVertexBuffer(_vkVertexBuffer);
        _vkCommandBuffer.Draw(3, 1);
        _vkCommandBuffer.EndRenderPass();
        _vkCommandBuffer.EndRecording();

        _vkCommandBuffer.Submit(
            _vkContext, 
            { {_vkImageAvailableSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT} }, 
            {_vkRenderFinishedSemaphore}, 
            _vkInFlightFence
        );
        _vkCommandBuffer.PresentResult(_vkContext, _vkSwapchain, { _vkRenderFinishedSemaphore });
        _vkContext.WaitIdle();
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

    void Window::FramebufferResize(GLFWwindow* window, int width, int height) {
        auto windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        windowPtr->_framebufferResized = true;
    }

}
}