#ifndef ENGINE_RENDERER_VULKAN_SWAPCHAIN_H
#define ENGINE_RENDERER_VULKAN_SWAPCHAIN_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Renderpass.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class Swapchain {
    public:

        // After calling Init also call Resize to fully setup the Swapchain
        void Init(Context& context, const uint32_t width, const uint32_t height);
        void Resize(Context& context, const RenderPass& renderPass, const uint32_t width, const uint32_t height);

        void Cleanup(const Context& context);

    private:
        friend class RenderPass;
        friend class CommandBuffer;

        VkSwapchainKHR _swapChain;

        uint32_t _queueFamilyIndices[2] = {0,0};
        VkSwapchainCreateInfoKHR _createInfo{};

        std::vector<VkImageView> _imageViews;

        std::vector<VkFramebuffer> _framebuffers;
        uint32_t _nextFramebuffer=0;
    };

}
}
}

#endif