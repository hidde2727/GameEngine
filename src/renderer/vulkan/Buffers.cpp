#include "renderer/vulkan/Buffers.h"
#include "renderer/vulkan/CommandBuffer.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    void BaseBuffer::Init(const Context& context, const VkBufferUsageFlagBits usage, const uint32_t size, const VmaAllocationCreateFlags memoryFlags) {
        ASSERT(size<=0, "Size must be greater than zero for a vulkan buffer")
        _usage = usage;
        _memoryFlags = memoryFlags;
        BaseBuffer::ResizeInternal(context, size);
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
    
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = _memoryFlags;

        VkResult result = vmaCreateBuffer(context._allocator, &bufferInfo, &allocInfo, &_buffer, &_allocation, nullptr);
        ASSERT(result != VK_SUCCESS, "Failed to allocate vulkan memory for a buffer");
    }
    
    void BaseBuffer::Cleanup(const Context& context) {
        vmaDestroyBuffer(context._allocator, _buffer, _allocation);
        _buffer = VK_NULL_HANDLE;
        _allocation = VK_NULL_HANDLE;
    }
    
    void BaseBuffer::SetData(Context& context, const void* data, const uint32_t length) {
        StartTransferingData(context, true, length);
        AddData(data, length);
        EndTransferingData(context);
    }

    void BaseBuffer::StartTransferingData(const Context& context, const bool resetOffset, const uint32_t overrideSize) {
        if(resetOffset) _writingOffset=0;
        _mappedMemoryOffset = _writingOffset;
        vmaMapMemory(context._allocator, _allocation, &_mappedData);
    }

    void BaseBuffer::AddData(const void* data, const uint32_t length) {
        ASSERT(_mappedData==nullptr, "Received data but StartTransferingData has not been called yet")
        ASSERT(_writingOffset+length>_size, "Received data too big to fit in the allocated vulkan buffer")

        memcpy(static_cast<char*>(_mappedData)+_writingOffset-_mappedMemoryOffset, data, length);
        _writingOffset+=length;
    }

    void BaseBuffer::EndTransferingData(Context& context) {
        ASSERT(_mappedData==nullptr, "End transfering data called while vulkan memory isn't mapped")
        _mappedData = nullptr;
        vmaUnmapMemory(context._allocator, _allocation);
        VkResult result = vmaFlushAllocation(context._allocator, _allocation, 0, VK_WHOLE_SIZE);
        ASSERT(result != VK_SUCCESS, "Failed to flush vulkan allocation");
    }
    




    void TransferBuffer::CopyTo(const Context& context, CommandBuffer commandBuffer, BaseBuffer* buffer) {
        if(buffer->_size < _size) buffer->ResizeInternal(context, _size);
        commandBuffer.CopyBuffer(_buffer, buffer->_buffer, _size);
    }
    void TransferBuffer::CopyTo(const Context& context, CommandBuffer commandBuffer, VkImage image, const uint32_t width, const uint32_t height) {
        commandBuffer.CopyBufferToImage(_buffer, image, width, height);
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




    void EfficientGPUBuffer::Init(const Context& context, const VkBufferUsageFlagBits usage, const uint32_t size) {  
        BaseBuffer::Init(
            context, 
            (VkBufferUsageFlagBits)(usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
            size, 
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT
        );
        VkMemoryPropertyFlags memPropFlags;
        vmaGetAllocationMemoryProperties(context._allocator, _allocation, &memPropFlags);
        if(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) return;
        _gpuLocal=true;
        _transferBuffer.Init(context, size);
    }
    
    void EfficientGPUBuffer::ResizeInternal(const Context& context, const uint32_t size) {
        if(size < _size) return;
        if(_gpuLocal) { _transferBuffer.Cleanup(context); _transferBuffer.Init(context, size); }
        BaseBuffer::ResizeInternal(context, size);
    }
    void EfficientGPUBuffer::StartTransferingData(const Context& context, const bool resetOffset, const uint32_t overrideSize) {
        if(_gpuLocal) _transferBuffer.StartTransferingData(context, resetOffset, overrideSize);
        else BaseBuffer::StartTransferingData(context, resetOffset, overrideSize);
    }
    void EfficientGPUBuffer::AddData(const void* data, const uint32_t length) {
        if(_gpuLocal) _transferBuffer.AddData(data, length);
        else BaseBuffer::AddData(data, length);
    }
    void EfficientGPUBuffer::EndTransferingData(Context& context) {
        if(!_gpuLocal) {
            BaseBuffer::EndTransferingData(context);
            return;
        }
        CommandBuffer commandBuffer;
        QueueType type = context.GetQueue(QueueType::TransferQueue)==VK_NULL_HANDLE? QueueType::GraphicsQueue : QueueType::TransferQueue;
        commandBuffer.Init(context, type, 1);
        commandBuffer.StartRecording(context, UINT32_MAX, true);
        EndTransferingData(context, commandBuffer);
        commandBuffer.EndRecording();
        commandBuffer.Submit(context);
        context.WaitQueueIdle(type);
    }
    void EfficientGPUBuffer::EndTransferingData(Context& context, CommandBuffer& commandBuffer) {
        if(!_gpuLocal) {
            BaseBuffer::EndTransferingData(context);
            return;
        }
        _transferBuffer.EndTransferingData(context);
        _transferBuffer.CopyTo(context, commandBuffer, this);
    }

}
}
}