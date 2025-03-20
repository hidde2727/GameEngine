#ifndef ENGINE_RENDERER_WINDOW_H
#define ENGINE_RENDERER_WINDOW_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Renderpass.h"
#include "renderer/vulkan/Swapchain.h"
#include "renderer/vulkan/Pipeline.h"

namespace Engine{
namespace Renderer{

    class Window {
    public:

        Window();
        ~Window();

        bool ShouldClose();
        void Update();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    private:
        GLFWwindow* _window = nullptr;
        Vulkan::Context _vkContext;
        Vulkan::RenderPass _vkRenderPass;
        Vulkan::Swapchain _vkSwapchain;
        Vulkan::Pipeline _vkPipeline;
    };

}
}

#endif