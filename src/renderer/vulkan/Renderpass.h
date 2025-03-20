#ifndef ENGINE_RENDERER_VULKAN_RENDERPASS_H
#define ENGINE_RENDERER_VULKAN_RENDERPASS_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Swapchain.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class Swapchain;
    class RenderPass {
    public:

        void Init(const Context& context, const Swapchain& swapchain);
        // Init with a framebuffer

        void Cleanup(const Context& context);

    private:
        friend class Pipeline;

        VkRenderPass _renderPass;
    };

}
}
}

#endif