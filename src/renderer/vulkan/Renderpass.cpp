#include "renderer/vulkan/Renderpass.h"

#include "renderer/vulkan/Swapchain.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    void RenderPass::Init(const Context& context, const Swapchain& swapchain, const uint32_t amountSubpasses) {
        Init(context, swapchain._createInfo.imageFormat, amountSubpasses);
    }

    void RenderPass::Init(const Context& context, const VkFormat format, const uint32_t amountSubpasses) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        std::vector<VkSubpassDescription> subpasses(amountSubpasses);
        std::vector<VkSubpassDependency> dependencies(amountSubpasses);
        for(uint32_t i = 0; i < amountSubpasses; i++) {
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpasses[i] = subpass;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = i==0? VK_SUBPASS_EXTERNAL : i-1;
            dependency.dstSubpass = i;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[i] = dependency;
        }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = amountSubpasses;
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = amountSubpasses;
        renderPassInfo.pDependencies = dependencies.data();

        VkResult result = vkCreateRenderPass(context._device, &renderPassInfo, nullptr, &_renderPass);
        ASSERT(result==VK_SUCCESS, "[Vulkan::Renderpass] Failed to create vulkan render pass");

    }

    void RenderPass::Cleanup(const Context& context) {
        vkDestroyRenderPass(context._device, _renderPass, nullptr);
    }

}
}
}