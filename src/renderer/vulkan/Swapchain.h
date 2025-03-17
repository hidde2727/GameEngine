#ifndef ENGINE_RENDERER_VULKAN_SWAPCHAIN_H
#define ENGINE_RENDERER_VULKAN_SWAPCHAIN_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class Swapchain {
    public:

        void Init(Context context, const uint32_t width, const uint32_t height);

        void Cleanup(const Context context);

    private:
        VkSwapchainKHR _swapChain;

        uint32_t _queueFamilyIndices[2] = {0,0};
        VkSwapchainCreateInfoKHR _createInfo{};

        std::vector<VkImageView> _imageViews;
    };

}
}
}

#endif