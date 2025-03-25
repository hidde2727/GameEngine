#ifndef ENGINE_RENDERER_VULKAN_BUFFERS_H
#define ENGINE_RENDERER_VULKAN_BUFFERS_H

#include "renderer/vulkan/Context.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class BaseBuffer {
    public:

        void SetData(const Context& context, const void* data, const uint32_t length);
        template<class T>
        void SetData(const Context& context, const T& data) {
            SetData(context, &data, sizeof(T));
        }
        template<class T>
        void SetData(const Context& context, const T* data) {
            SetData(context, data, sizeof(T));
        }
        template<class T, uint32_t S>
        void SetData(const Context& context, const T (&data)[S]) {
            SetData(context, &data, sizeof(T)*S);
        }
        template<class T>
        void SetData(const Context& context, const std::vector<T>& data) {
            SetData(context, data.data(), (uint32_t)(sizeof(T)*data.size()));
        }

        void StartTransferingData(const Context& context, const bool resetOffset=true, const uint32_t overrideSize=UINT32_MAX);
        void AddData(const void* data, const uint32_t length);
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
        void EndTransferingData(const Context& context);

    protected:
        void Init(const Context& context, const VkBufferUsageFlagBits usage, const uint32_t size, const VkMemoryPropertyFlags memoryProperties);
        void ResizeInternal(const Context& context, const uint32_t size);
        void Cleanup(const Context& context);
        
        friend class TransferBuffer;

        VkBuffer _buffer = VK_NULL_HANDLE;
        uint32_t _size;
    private:  

        uint32_t FindMemoryType(const Context& context, const uint32_t typeFilter, const VkMemoryPropertyFlags properties);

        VkBufferUsageFlagBits _usage;
        VkMemoryPropertyFlags _memoryProperties;
        VkDeviceMemory _bufferMemory = VK_NULL_HANDLE;
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
                gpuLocal ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
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
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
        }
        inline void Cleanup(const Context& context) { BaseBuffer::Cleanup(context); }

        void CopyTo(const Context& context, const CommandBuffer commandBuffer, BaseBuffer* buffer);
        void CopyTo(Context& context, BaseBuffer* buffer);
    
    private:
        
    };

}
}
}

#endif