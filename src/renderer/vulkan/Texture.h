#ifndef ENGINE_RENDERER_TEXTURE_H
#define ENGINE_RENDERER_TEXTURE_H

#include "core/PCH.h"

#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/CommandBuffer.h"
#include "renderer/vulkan/Buffers.h"

#include "util/Vec2D.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class Texture {
    public:

        void Init(Context& context, const Util::Vec2U32 size, const VkFormat format);
        void Cleanup(Context& context);

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
        uint8_t* GetTransferLocation();
        void EndTransferingData(Context& context);
        void EndTransferingData(Context& context, CommandBuffer& copyCommandBuffer);
        void TransferCompleteOnCommandBuffer(Context& context);

        uint32_t GetBoundDescriptorSlot() { return _boundDescriptorSlot; }

    private:
        friend class Pipeline;

        VkImage _image;
        VkImageView _imageView;
        VmaAllocation _allocation;
        TransferBuffer _transferBuffer;
        size_t _size;
        Util::Vec2U32 _rectSize;

        uint32_t _boundDescriptorSlot = UINT32_MAX;
    };

    class TextureSampler {
    public:

        void Init(const Context& context, const VkFilter magFilter=VK_FILTER_LINEAR, const VkFilter minFilter=VK_FILTER_LINEAR);
        void Cleanup(const Context& context);

    private:
        friend class Pipeline;

        VkSampler _sampler;
    };

}
}
}

#endif