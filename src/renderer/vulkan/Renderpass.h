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
        void Init(const Context& context, const VkFormat format);

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