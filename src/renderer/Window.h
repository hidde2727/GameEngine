#ifndef ENGINE_RENDERER_WINDOW_H
#define ENGINE_RENDERER_WINDOW_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Renderpass.h"
#include "renderer/vulkan/Swapchain.h"
#include "renderer/vulkan/Pipeline.h"
#include "renderer/vulkan/CommandBuffer.h"

#include "util/Vec2D.h"
#include "util/Vec3D.h"

namespace Engine {
namespace Renderer {

    struct Vertex {
        Vertex(const Utils::Vec2D pos, const Utils::Vec3D color) : pos(pos), color(color) {}
        Utils::Vec2F pos;
        Utils::Vec3F color;
    };

    class Window {
    public:

        Window();
        ~Window();

        bool ShouldClose();
        void Update();
        void Draw();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        static void FramebufferResize(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow* _window = nullptr;
        Vulkan::Context _vkContext;
        Vulkan::RenderPass _vkRenderPass;
        Vulkan::Swapchain _vkSwapchain;
        Vulkan::Pipeline _vkPipeline;
        Vulkan::CommandBuffer _vkCommandBuffer;

        Vulkan::Fence _vkInFlightFence;
        Vulkan::Semaphore _vkImageAvailableSemaphore;
        Vulkan::Semaphore _vkRenderFinishedSemaphore;

        Vulkan::VertexBuffer _vkVertexBuffer;
        Vulkan::TransferBuffer _vkTransferBuffer;

        bool _framebufferResized;
    };

}
}

#endif