#include "renderer/vulkan/Texture.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    void Texture::Init(Context& context, const Utils::Vec2U32 size, const VkFormat format) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(size.x);
        imageInfo.extent.height = static_cast<uint32_t>(size.y);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = 0;

        VmaAllocationInfo allocationInfo;
        VkResult result = vmaCreateImage(context._allocator, &imageInfo, &allocInfo, &_image, &_allocation, &allocationInfo);
        ASSERT(result != VK_SUCCESS, "Failed to create a vulkan image with VMA")
        _rectSize = size;
        _size = allocationInfo.size;

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        result = vkCreateImageView(context._device, &viewInfo, nullptr, &_imageView);
        ASSERT(result != VK_SUCCESS, "Failed to create a vulkan image view for texture")
    }
    void Texture::Cleanup(Context& context) {
        vkDestroyImageView(context._device, _imageView, nullptr);
        vmaDestroyImage(context._allocator, _image, _allocation);
    }

    void Texture::SetData(Context& context, const void* data, const uint32_t length) {
        StartTransferingData(context);
        AddData(data, length);
        EndTransferingData(context);
    }
    void Texture::StartTransferingData(const Context& context, const bool resetOffset, const uint32_t overrideSize) {
        _transferBuffer.Init(context, (uint32_t)_size);
        _transferBuffer.StartTransferingData(context, resetOffset, overrideSize);
    }
    void Texture::AddData(const void* data, const uint32_t length) {
        _transferBuffer.AddData(data, length);
    }
    uint8_t* Texture::GetTransferLocation() {
        return reinterpret_cast<uint8_t*>(_transferBuffer._mappedData);
    }
    void Texture::EndTransferingData(Context& context) {
        CommandBuffer commandBuffer;
        QueueType type = context.GetQueue(QueueType::TransferQueue)==VK_NULL_HANDLE? QueueType::GraphicsQueue : QueueType::TransferQueue;
        commandBuffer.Init(context, type, 1);
        commandBuffer.StartRecording(context, UINT32_MAX, true);
        EndTransferingData(context, commandBuffer);
        commandBuffer.EndRecording();
        commandBuffer.Submit(context);
        context.WaitQueueIdle(type);
        TransferCompleteOnCommandBuffer(context);
    }
    void Texture::EndTransferingData(Context& context, CommandBuffer& copyCommandBuffer) {
        _transferBuffer.EndTransferingData(context);
        copyCommandBuffer.TransferImageLayout(_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        _transferBuffer.CopyTo(context, copyCommandBuffer, _image, _rectSize.x, _rectSize.y);
        copyCommandBuffer.TransferImageLayout(_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    void Texture::TransferCompleteOnCommandBuffer(Context& context) {
        _transferBuffer.Cleanup(context);
    }





    void TextureSampler::Init(const Context& context, const VkFilter magFilter, const VkFilter minFilter) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = magFilter;
        samplerInfo.minFilter = minFilter;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        const VkResult result = vkCreateSampler(context._device, &samplerInfo, nullptr, &_sampler);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan image sampler")
    }
    void TextureSampler::Cleanup(const Context& context) {
        vkDestroySampler(context._device, _sampler, nullptr);
    }

}
}
}