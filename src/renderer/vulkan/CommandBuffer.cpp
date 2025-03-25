#include "renderer/vulkan/CommandBuffer.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    void CommandBuffer::Init(Context& context, const QueueType queueType, const uint32_t framesInFlight) {
        _queue = queueType;
        _framesInFlight = framesInFlight;
        _commandBuffers.resize(_framesInFlight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = context._queueTypeToCommandPools[queueType];
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = _framesInFlight;

        const VkResult result = vkAllocateCommandBuffers(context._device, &allocInfo, _commandBuffers.data());
        ASSERT(result!=VK_SUCCESS, "Failed to create an vulkan command buffer");
    }
    void CommandBuffer::Cleanup(const Context& context) {
        for(const Fence& fences : _fences) {
            for(const VkFence fence : fences) {
                vkDestroyFence(context._device, fence, nullptr);
            }
        }
        for(const Semaphore& semaphores : _semaphores) {
            for(const VkSemaphore semaphore : semaphores) {
                vkDestroySemaphore(context._device, semaphore, nullptr);
            }
        }
    }

    void CommandBuffer::AcquireNextSwapchainFrame(const Context& context, Swapchain& swapchain, const Semaphore& imageAvailable) {
        _currentFrame = (_currentFrame + 1) % _framesInFlight;
        vkAcquireNextImageKHR(context._device, swapchain._swapChain, UINT64_MAX, imageAvailable[_currentFrame], VK_NULL_HANDLE, &swapchain._nextFramebuffer);
    }

    void CommandBuffer::StartRecording(const Context& context, const uint32_t specificFrameInFlight, const bool oneTimeUse) {
        _currentFrame = specificFrameInFlight==UINT32_MAX || specificFrameInFlight>=_framesInFlight ? _currentFrame : specificFrameInFlight;
        vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = oneTimeUse? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
        beginInfo.pInheritanceInfo = nullptr;
        
        const VkResult result = vkBeginCommandBuffer(_commandBuffers[_currentFrame], &beginInfo);
        ASSERT(result!=VK_SUCCESS, "Failed to start recording of an vulkan command buffer")
    }
    void CommandBuffer::EndRecording() {
        const VkResult result = vkEndCommandBuffer(_commandBuffers[_currentFrame]);
        ASSERT(result!=VK_SUCCESS, "Failed to end recording of an vulkan command buffer")
    }

    void CommandBuffer::BeginRenderPass(const RenderPass renderPass, const Swapchain swapChain, const VkClearValue clearColor, bool setViewportAndScissor) {
        BeginRenderPass(renderPass, swapChain._framebuffers[swapChain._nextFramebuffer], swapChain._createInfo.imageExtent, clearColor);
        if(setViewportAndScissor) {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChain._createInfo.imageExtent.width);
            viewport.height = static_cast<float>(swapChain._createInfo.imageExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            SetViewport(viewport);
            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChain._createInfo.imageExtent;
            SetScissor(scissor);
        }
    }
    void CommandBuffer::BeginRenderPass(const RenderPass renderPass, const VkFramebuffer framebuffer, const VkExtent2D extent, const VkClearValue clearColor) {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass._renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(_commandBuffers[_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    void CommandBuffer::EndRenderPass() {
        vkCmdEndRenderPass(_commandBuffers[_currentFrame]);
    }

    void CommandBuffer::SetViewport(const VkViewport viewport) {
        vkCmdSetViewport(_commandBuffers[_currentFrame], 0, 1, &viewport);
    }
    void CommandBuffer::SetScissor(const VkRect2D extent) {
        vkCmdSetScissor(_commandBuffers[_currentFrame], 0, 1, &extent);
    }

    void CommandBuffer::BindGraphicsPipeline(const Pipeline& pipeline) {
        vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipeline);
    }
    void CommandBuffer::BindVertexBuffer(const VertexBuffer& buffer) {
        VkBuffer vertexBuffers[] = {buffer._buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(_commandBuffers[_currentFrame], 0, 1, vertexBuffers, offsets);
    }

    void CommandBuffer::CopyBuffer(const VkBuffer& from, const VkBuffer& to, const uint32_t size) {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(_commandBuffers[_currentFrame], from, to, 1, &copyRegion);
    }

    void CommandBuffer::Draw(const int vertexCount, const int instanceCount, const int vertexOffset, const int instanceOffset) {
        vkCmdDraw(_commandBuffers[_currentFrame], vertexCount, instanceCount, vertexOffset, instanceOffset);
    }
    
    Semaphore CommandBuffer::CreateSemaphore(const Context& context) {
        Semaphore semaphore(_framesInFlight);
        for(uint32_t i = 0; i < _framesInFlight; i++) {
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkCreateSemaphore(context._device, &semaphoreInfo, nullptr, &semaphore[i]);
        }
        _semaphores.push_back(semaphore);
        return semaphore;
    }
    Fence CommandBuffer::CreateFence(const Context& context, const bool signaled) {
        Fence fence(_framesInFlight);
        for(uint32_t i = 0; i < _framesInFlight; i++) {
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = signaled?VK_FENCE_CREATE_SIGNALED_BIT:0;
            vkCreateFence(context._device, &fenceInfo, nullptr, &fence[i]);
        }
        _fences.push_back(fence);
        return fence;
    }

    void CommandBuffer::WaitFence(const Context& context, const Fence& fence) {
        vkWaitForFences(context._device, 1, &fence[_currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(context._device, 1, &fence[_currentFrame]);
    }

    void CommandBuffer::Submit(
        Context& context, 
        const std::initializer_list<std::pair<Semaphore&, VkPipelineStageFlags>> waitFor, 
        const std::initializer_list<Semaphore> signalSemaphore,
        const Fence& signalFence
    ) {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::vector<VkSemaphore> waitSemaphores;
        waitSemaphores.reserve(waitFor.size());
        std::vector<VkPipelineStageFlags> waitStages;
        waitStages.reserve(waitFor.size());
        for(const std::pair<Semaphore&, VkPipelineStageFlags> semaphore : waitFor) { 
            waitSemaphores.push_back(semaphore.first[_currentFrame]); 
            waitStages.push_back(semaphore.second);
        }

        submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

        std::vector<VkSemaphore> signalSemaphores;
        signalSemaphores.reserve(signalSemaphore.size());
        for(const Semaphore& semaphore : signalSemaphore) { signalSemaphores.push_back(semaphore[_currentFrame]); }
        submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        const VkResult result = vkQueueSubmit(context.GetQueue(_queue), 1, &submitInfo, (signalFence.size()>0 ? signalFence[_currentFrame] : VK_NULL_HANDLE));
        ASSERT(result!=VK_SUCCESS, "Failed to submit vulkan command buffer to an queue");

    }

    void CommandBuffer::PresentResult(Context& context, const Swapchain& swapchain, const std::initializer_list<Semaphore> waitFor) {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        std::vector<VkSemaphore> waitSemaphores;
        waitSemaphores.reserve(waitFor.size());
        for(const Semaphore& semaphore : waitFor) { waitSemaphores.push_back(semaphore[_currentFrame]); }
        presentInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
        presentInfo.pWaitSemaphores = waitSemaphores.data();

        VkSwapchainKHR swapChains[] = {swapchain._swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &swapchain._nextFramebuffer;
        presentInfo.pResults = nullptr;

        const VkResult result = vkQueuePresentKHR(context.GetQueue(QueueType::KHRPresentQueue), &presentInfo);
        ASSERT(result!=VK_SUCCESS, "Failed to present to KHRPresentQueue")
    }

}
}
}