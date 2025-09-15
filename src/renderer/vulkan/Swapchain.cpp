#include "Swapchain.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    void Swapchain::Init(Context& context, const uint32_t width, const uint32_t height) {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(context._physicalDevice, context._surfaceKHR, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(context._physicalDevice, context._surfaceKHR, &formatCount, formats.data());
        VkSurfaceFormatKHR selectedFormat{};
        for (const auto& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                selectedFormat = availableFormat;
            }
        }
        if(selectedFormat.format == VK_FORMAT_UNDEFINED) selectedFormat = formats[0];

        // Garantueed to be available
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context._physicalDevice, context._surfaceKHR, &capabilities);

        VkExtent2D extent{};
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            extent = capabilities.currentExtent;
        } else {    
            extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        _createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        _createInfo.surface = context._surfaceKHR;
        _createInfo.minImageCount = imageCount;
        _createInfo.imageFormat = selectedFormat.format;
        _createInfo.imageColorSpace = selectedFormat.colorSpace;
        _createInfo.imageExtent = extent;
        _createInfo.imageArrayLayers = 1;
        _createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        _queueFamilyIndices[0] = context._queueFamilies[QueueType::GraphicsQueue];
        _queueFamilyIndices[1] = context._queueFamilies[QueueType::KHRPresentQueue];
        if(_queueFamilyIndices[0] == _queueFamilyIndices[1]) {
            _createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        } else {
            _createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            _createInfo.queueFamilyIndexCount = 2;
            _createInfo.pQueueFamilyIndices = _queueFamilyIndices;
        }

        _createInfo.preTransform = capabilities.currentTransform;
        _createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        _createInfo.presentMode = presentMode;
        _createInfo.clipped = VK_TRUE;
        VkSwapchainKHR oldSwapchain = _swapChain;
        _createInfo.oldSwapchain = oldSwapchain;
    }

    void Swapchain::Resize(Context& context, const RenderPass& renderPass, const uint32_t width, const uint32_t height) {
        vkDeviceWaitIdle(context._device);
        if(_swapChain != VK_NULL_HANDLE) Cleanup(context);

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context._physicalDevice, context._surfaceKHR, &capabilities);
        VkExtent2D extent{};
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            extent = capabilities.currentExtent;
        } else {    
            extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }
        _createInfo.imageExtent = extent;

        //VkSwapchainKHR oldSwapchain = _swapChain;
        //_createInfo.oldSwapchain = oldSwapchain;

        VkResult result = vkCreateSwapchainKHR(context._device, &_createInfo, nullptr, &_swapChain);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan swapchain");

        std::vector<VkImage> images;
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(context._device, _swapChain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(context._device, _swapChain, &imageCount, images.data());

        _imageViews.resize(imageCount);
        _framebuffers.resize(imageCount);
        for(size_t i = 0; i < imageCount; i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = _createInfo.imageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(context._device, &createInfo, nullptr, &_imageViews[i]);
            ASSERT(result != VK_SUCCESS, "Failed to create vulkan swapchain image view");

            VkImageView attachments[] = {
                _imageViews[i]
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass._renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = _createInfo.imageExtent.width;
            framebufferInfo.height = _createInfo.imageExtent.height;
            framebufferInfo.layers = 1;

            result = vkCreateFramebuffer(context._device, &framebufferInfo, nullptr, &_framebuffers[i]);
            ASSERT(result != VK_SUCCESS, "Failed to create vulkan swapchain framebuffers");
        
        }
    }

    void Swapchain::Cleanup(const Context& context) {
        for (auto framebuffer : _framebuffers) {
            vkDestroyFramebuffer(context._device, framebuffer, nullptr);
        }
        for (auto imageView : _imageViews) {
            vkDestroyImageView(context._device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(context._device, _swapChain, nullptr);
    }

}
}
}