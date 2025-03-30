#ifndef ENGINE_RENDERER_VULKAN_BUFFERS_H
#define ENGINE_RENDERER_VULKAN_BUFFERS_H

#include "renderer/vulkan/Context.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class BaseBuffer {
    public:

        void SetData(Context& context, const void* data, const uint32_t length);
        template<class T>
        void SetData(Context& context, const T& data) {
            SetData(context, &data, sizeof(T));
        }
        template<class T>
        void SetData(Context& context, const T* data) {
            SetData(context, data, sizeof(T));
        }
        template<class T, uint32_t S>
        void SetData(Context& context, const T (&data)[S]) {
            SetData(context, &data, sizeof(T)*S);
        }
        template<class T>
        void SetData(Context& context, const std::vector<T>& data) {
            SetData(context, data.data(), (uint32_t)(sizeof(T)*data.size()));
        }

        virtual void StartTransferingData(const Context& context, const bool resetOffset=true, const uint32_t overrideSize=UINT32_MAX);
        virtual void AddData(const void* data, const uint32_t length);
        template<class T>
        void AddData(const T& data) {
            AddData(&data, sizeof(T));
        }
        template<class T>
        void AddData(const T* data) {
            AddData(data, sizeof(T));
        }
        template<class T, uint32_t S>
        void AddData(const T (&data)[S]) {
            AddData(&data, sizeof(T)*S);
        }
        template<class T>
        void AddData(const std::vector<T> &data) {
            AddData(data.data(), (uint32_t)(sizeof(T)*data.size()));
        }
        virtual void EndTransferingData(Context& context);

    protected:
        void Init(const Context& context, const VkBufferUsageFlagBits usage, const uint32_t size, const VmaAllocationCreateFlags memoryFlags);
        void ResizeInternal(const Context& context, const uint32_t size);
        void Cleanup(const Context& context);
        
        friend class TransferBuffer;

        VkBuffer _buffer = VK_NULL_HANDLE;
        uint32_t _size;
        VmaAllocation _allocation;
    private:
        friend class Texture;

        VkBufferUsageFlagBits _usage;
        VmaAllocationCreateFlags _memoryFlags;
        void* _mappedData = nullptr;
        uint32_t _writingOffset=0;
        uint32_t _mappedMemoryOffset=0;
    };

    class VertexBuffer : public BaseBuffer {
    public:

        inline void Init(const Context& context, const uint32_t size, const bool gpuLocal) {  
            BaseBuffer::Init(
                context, 
                (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | (gpuLocal ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0)), 
                size, 
                gpuLocal ? 0 : VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
            );
        }
        inline void Cleanup(const Context& context) { BaseBuffer::Cleanup(context); }
    
    private:
        friend class CommandBuffer;
    };

    class IndexBuffer : public BaseBuffer {
    public:

        inline void Init(const Context& context, const uint32_t size, const bool gpuLocal) {  
            BaseBuffer::Init(
                context, 
                (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | (gpuLocal ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0)), 
                size, 
                gpuLocal ? 0 : VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
            );
        }
        inline void Cleanup(const Context& context) { BaseBuffer::Cleanup(context); }
    
    private:
        friend class CommandBuffer;
    };

    class CommandBuffer;
    class TransferBuffer : public BaseBuffer {
    public:

        inline void Init(const Context& context, const uint32_t size) {  
            BaseBuffer::Init(
                context, 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                size, 
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
            );
        }
        inline void Cleanup(const Context& context) { BaseBuffer::Cleanup(context); }

        void CopyTo(const Context& context, const CommandBuffer commandBuffer, BaseBuffer* buffer);
        void CopyTo(const Context& context, const CommandBuffer commandBuffer, VkImage image, const uint32_t width, const uint32_t height);
        void CopyTo(Context& context, BaseBuffer* buffer);
    
    private:
        
    };

    class EfficientGPUBuffer : public BaseBuffer {
    public:

        void StartTransferingData(const Context& context, const bool resetOffset=true, const uint32_t overrideSize=UINT32_MAX) override;
        void AddData(const void* data, const uint32_t length) override;
        template<class T>
        void AddData(const T& data) {
            AddData(&data, sizeof(T));
        }
        template<class T>
        void AddData(const T* data) {
            AddData(data, sizeof(T));
        }
        template<class T, uint32_t S>
        void AddData(const T (&data)[S]) {
            AddData(&data, sizeof(T)*S);
        }
        template<class T>
        void AddData(const std::vector<T> &data) {
            AddData(data.data(), (uint32_t)(sizeof(T)*data.size()));
        }
        void EndTransferingData(Context& context) override;
        void EndTransferingData(Context& context, CommandBuffer& commandBuffer);

    protected:

        void Init(const Context& context, const VkBufferUsageFlagBits usage, const uint32_t size);
        void ResizeInternal(const Context& context, const uint32_t size);
        inline void Cleanup(const Context& context) { BaseBuffer::Cleanup(context); }

    private:
        
        bool _gpuLocal = false;
        TransferBuffer _transferBuffer;

    };

    class EfficientVertexBuffer : public EfficientGPUBuffer {
    public:

        inline void Init(const Context& context, const uint32_t size) {  
            EfficientGPUBuffer::Init(
                context, 
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                size
            );
        }
        inline void Cleanup(const Context& context) { EfficientGPUBuffer::Cleanup(context); }

        inline void Resize(const Context& context, const uint32_t size) {
            ResizeInternal(context, size);
        }
    
    private:
        friend class CommandBuffer;
    };

    class EfficientIndexBuffer : public EfficientGPUBuffer {
    public:

        inline void Init(const Context& context, const uint32_t size) {  
            EfficientGPUBuffer::Init(
                context, 
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                size
            );
        }
        inline void Cleanup(const Context& context) { EfficientGPUBuffer::Cleanup(context); }
    
    private:
        friend class CommandBuffer;
    };

}
}
}

#endif