#include "renderer/vulkan/Buffers.h"
#include "renderer/vulkan/CommandBuffer.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    void BaseBuffer::Init(const Context& context, const VkBufferUsageFlagBits usage, const uint32_t size, const VkMemoryPropertyFlags memoryProperties) {
        ASSERT(size<=0, "Size must be greater than zero for a vulkan buffer")
        _usage = usage;
        _memoryProperties = memoryProperties;
        ResizeInternal(context, size);
    }

    void BaseBuffer::ResizeInternal(const Context& context, const uint32_t size) {
        ASSERT(size<=0, "Size must be greater than zero for a vulkan buffer")
        if(_buffer != VK_NULL_HANDLE) Cleanup(context);
        _size = size;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = _usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
        VkResult result = vkCreateBuffer(context._device, &bufferInfo, nullptr, &_buffer);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan buffer");
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(context._device, _buffer, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(context, memRequirements.memoryTypeBits, _memoryProperties);

        result = vkAllocateMemory(context._device, &allocInfo, nullptr, &_bufferMemory);
        ASSERT(result != VK_SUCCESS, "Failed to allocate vulkan memory for a buffer");

        vkBindBufferMemory(context._device, _buffer, _bufferMemory, 0);
    }
    
    void BaseBuffer::Cleanup(const Context& context) {
        vkDestroyBuffer(context._device, _buffer, nullptr);
        _buffer = VK_NULL_HANDLE;
        vkFreeMemory(context._device, _bufferMemory, nullptr);
        _bufferMemory = VK_NULL_HANDLE;
    }

    uint32_t BaseBuffer::FindMemoryType(const Context& context, const uint32_t typeFilter, const VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(context._physicalDevice, &memProperties);
        uint32_t suitable = UINT32_MAX;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                if(memProperties.memoryHeaps[memProperties.memoryTypes[i].heapIndex].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) return i;
                suitable = i;
            }
        }
        ASSERT(suitable == UINT32_MAX, "Failed to find suitable vulkan memory");
        return suitable;
    }
    
    void BaseBuffer::SetData(const Context& context, const void* data, const uint32_t length) {
        StartTransferingData(context, true, length);
        AddData(data, length);
        EndTransferingData(context);
    }

    void BaseBuffer::StartTransferingData(const Context& context, const bool resetOffset, const uint32_t overrideSize) {
        ASSERT(_mappedData!=nullptr, "Start transfering data called while vulkan memory is already mapped")
        ASSERT(!(_memoryProperties&(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)), "Cannot transfer to a vulkan buffer that is not host visible")
        if(resetOffset) _writingOffset=0;
        _mappedMemoryOffset = _writingOffset;

        if(overrideSize==UINT32_MAX) {
            const VkResult result = vkMapMemory(context._device, _bufferMemory, _mappedMemoryOffset, _size-_mappedMemoryOffset, 0, &_mappedData);
            ASSERT(result != VK_SUCCESS, "Failed to map vulkan buffer memory")
        }
        ASSERT(overrideSize+_mappedMemoryOffset > _size, "Received overrideSize that is bigger then the allocated vulkan buffer size")
        const VkResult result = vkMapMemory(context._device, _bufferMemory, _mappedMemoryOffset, overrideSize, 0, &_mappedData);
        ASSERT(result != VK_SUCCESS, "Failed to map vulkan buffer memory")
    }

    void BaseBuffer::AddData(const void* data, const uint32_t length) {
        ASSERT(_writingOffset+length>_size, "Received data too big to fit in the allocated vulkan buffer")
        ASSERT(_mappedData==nullptr, "AddData called before StartTransferingData was called")

        memcpy(static_cast<char*>(_mappedData)+_writingOffset-_mappedMemoryOffset, data, length);
        _writingOffset+=length;
    }

    void BaseBuffer::EndTransferingData(const Context& context) {
        ASSERT(_mappedData==nullptr, "End transfering data called while vulkan memory isn't mapped")
        vkUnmapMemory(context._device, _bufferMemory);
        _mappedData = nullptr;
    }
    




    void TransferBuffer::CopyTo(const Context& context, CommandBuffer commandBuffer, BaseBuffer* buffer) {
        if(buffer->_size < _size) buffer->ResizeInternal(context, _size);
        commandBuffer.CopyBuffer(_buffer, buffer->_buffer, _size);
    }

    void TransferBuffer::CopyTo(Context& context, BaseBuffer* buffer) {
        CommandBuffer commandBuffer;
        QueueType type = context.GetQueue(QueueType::TransferQueue)==VK_NULL_HANDLE? QueueType::GraphicsQueue : QueueType::TransferQueue;
        commandBuffer.Init(context, type, 1);
        commandBuffer.StartRecording(context, UINT32_MAX, true);
        CopyTo(context, commandBuffer, buffer);
        commandBuffer.EndRecording();
        commandBuffer.Submit(context);
        context.WaitQueueIdle(type);
    }
}
}
}