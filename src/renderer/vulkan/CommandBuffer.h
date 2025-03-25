#ifndef ENGINE_RENDERER_VULKAN_COMMANDBUFFER_H
#define ENGINE_RENDERER_VULKAN_COMMANDBUFFER_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Renderpass.h"
#include "renderer/vulkan/Swapchain.h"
#include "renderer/vulkan/Pipeline.h"
#include "renderer/vulkan/Buffers.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    typedef std::vector<VkSemaphore> Semaphore;
    typedef std::vector<VkFence> Fence;
    // Can only be used with one swapchain
    // Can be used with as many different framebuffers as you want
    class CommandBuffer {
    public:

        void Init(Context& context, const QueueType queueType, const uint32_t framesInFlight=1);
        void Cleanup(const Context& context);

        void AcquireNextSwapchainFrame(const Context& context, Swapchain& swapchain, const Semaphore& imageAvailable);

        void StartRecording(const Context& context, const uint32_t specificFrameInFlight=UINT32_MAX, const bool oneTimeUse=false);
        void EndRecording();

        void BeginRenderPass(const RenderPass renderPass, const Swapchain swapChain, const VkClearValue clearColor, bool setViewportAndScissor=false);
        void BeginRenderPass(const RenderPass renderPass, const VkFramebuffer framebuffer, const VkExtent2D extent, const VkClearValue clearColor={{{0.0f, 0.0f, 0.0f, 1.0f}}});
        void EndRenderPass();

        void SetViewport(const VkViewport viewport);
        void SetScissor(const VkRect2D extent);

        void BindGraphicsPipeline(const Pipeline& pipeline);
        void BindVertexBuffer(const VertexBuffer& buffer);

        void CopyBuffer(const VkBuffer& from, const VkBuffer& to, const uint32_t size);

        void Draw(const int vertexCount, const int instanceCount, const int vertexOffset=0, const int instanceOffset=0);

        Semaphore CreateSemaphore(const Context& context);
        Fence CreateFence(const Context& context, const bool signaled=true);

        void WaitFence(const Context& context, const Fence& fence);

        void Submit(Context& context, const std::initializer_list<std::pair<Semaphore&, VkPipelineStageFlags>> waitFor={}, const std::initializer_list<Semaphore> signalSemaphore={}, const Fence& signalFence={});
        void PresentResult(Context& context, const Swapchain& swapchain, const std::initializer_list<Semaphore> waitFor);

    private:
        std::vector<VkCommandBuffer> _commandBuffers;
        uint32_t _framesInFlight=1;
        uint32_t _currentFrame=0;
        QueueType _queue;


        std::vector<Semaphore> _semaphores;
        std::vector<Fence> _fences;
    };

}
}
}

#endif