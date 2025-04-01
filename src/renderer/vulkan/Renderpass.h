#ifndef ENGINE_RENDERER_VULKAN_RENDERPASS_H
#define ENGINE_RENDERER_VULKAN_RENDERPASS_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class Swapchain;
    class RenderPass {
    public:
        // Subpasses are defined as a pass on the output framebuffer, no special depthPasses etc supported
        void Init(const Context& context, const Swapchain& swapchain, const uint32_t amountSubpasses);
        // Subpasses are defined as a pass on the output framebuffer, no special depthPasses etc supported
        void Init(const Context& context, const VkFormat format, const uint32_t amountSubpasses);

        void Cleanup(const Context& context);

    private:
        friend class Swapchain;
        friend class Pipeline;
        friend class CommandBuffer;

        VkRenderPass _renderPass;
    };

}
}
}

#endif