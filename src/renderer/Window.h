#ifndef ENGINE_RENDERER_WINDOW_H
#define ENGINE_RENDERER_WINDOW_H

#include "core/PCH.h"
#include "core/Components.h"

#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Renderpass.h"
#include "renderer/vulkan/Swapchain.h"
#include "renderer/vulkan/Pipeline.h"
#include "renderer/vulkan/CommandBuffer.h"
#include "renderer/vulkan/Texture.h"

#include "renderer/TextureMap.h"
#include "renderer/ImageLoader.h"
#include "renderer/TextLoader.h"

#include "util/Vec2D.h"
#include "util/Vec3D.h"
#include "util/BitMask.h"

// The amount of bits the asset vs the texturemap will use of the AssetID
// Defaults to 24 bits for the asset and 8 bits for the texturemap
#ifndef ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS
    #define ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS 24
#endif
// The amount of bits the asset vs the texturemap will use of the AssetID
// Defaults to 24 bits for the asset and 8 bits for the texturemap
#ifndef ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS
    #define ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS 22
#endif

#define ENGINE_RENDERER_ASSETTYPE_TEXTURE 1
#define ENGINE_RENDERER_ASSETTYPE_TEXT 2

namespace Engine {
namespace Renderer {

    struct Vertex {
        Utils::Vec2F dimensions;
    };
    struct InstanceDataRect {
        InstanceDataRect(const Utils::Vec2F pos, const Utils::Vec2F dimensions, const Utils::Vec3F color, const Utils::Vec2F texturePos, const Utils::Vec2F textureDimensions, const uint32_t texture)
         : pos(pos), dimensions(dimensions), color(color), texturePos(texturePos), textureDimensions(textureDimensions), texture(texture) {}
        Utils::Vec2F pos;
        Utils::Vec2F dimensions;
        Utils::Vec3F color;
        Utils::Vec2F texturePos;
        Utils::Vec2F textureDimensions;
        uint32_t texture;
    };
    struct InstanceDataText {
        InstanceDataText(const Utils::Vec2F pos, const Utils::Vec2F dimensions, const Utils::Vec3F color, const Utils::Vec2F texturePos, const Utils::Vec2F textureDimensions, const uint32_t texture, const float pxRange)
         : pos(pos), dimensions(dimensions), color(color), texturePos(texturePos), textureDimensions(textureDimensions), texture(texture), pxRange(pxRange) {}
        Utils::Vec2F pos;
        Utils::Vec2F dimensions;
        Utils::Vec3F color;
        Utils::Vec2F texturePos;
        Utils::Vec2F textureDimensions;
        uint32_t texture;
        float pxRange;
    };
    typedef uint32_t AssetID;

    class Window {
    public:

        Window(const uint32_t textureMapSlots);
        ~Window();

        bool ShouldClose();
        void Update();
        void Draw(entt::registry& registry, const uint32_t amountRectangles, const uint32_t amountText);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        static void FramebufferResize(GLFWwindow* window, int width, int height);

        void StartAssetLoading(const size_t textureMapID);
        void SetAssetLoadingCacheName(const size_t textureMapID, const std::string cacheName);
        AssetID AddAsset(const size_t textureMapID, std::unique_ptr<AssetLoader> textureLoader, const uint32_t assetTypeID);
        void EndAssetLoading(const size_t textureMapID);
        void CleanupAssets(const size_t textureMapID);

        std::shared_ptr<ImageRenderInfo> GetTextureInfo(AssetID asset);
        std::shared_ptr<TextRenderInfo> GetTextInfo(AssetID asset);

    private:

        GLFWwindow* _window = nullptr;
        Vulkan::Context _vkContext;
        Vulkan::RenderPass _vkRenderPass;
        Vulkan::Swapchain _vkSwapchain;
        Vulkan::Pipeline _vkRectPipeline;
        Vulkan::Pipeline _vkTextPipeline;
        Vulkan::CommandBuffer _vkCommandBuffer;

        Vulkan::Fence _vkInFlightFence;
        Vulkan::Semaphore _vkImageAvailableSemaphore;
        Vulkan::Semaphore _vkRenderFinishedSemaphore;

        // First (sizeof(Vertex)*4) is reserved for the vertex data, afterwards sits the instance data
        Vulkan::EfficientVertexBuffer _vkRectVertexBuffer;
        // First (sizeof(Vertex)*4) is reserved for the vertex data, afterwards sits the instance data
        Vulkan::EfficientVertexBuffer _vkTextVertexBuffer;
        Vulkan::IndexBuffer _vkIndexBuffer;
        Vulkan::EfficientVertexBuffer _vkPerVertexBuffer;

        Vulkan::TextureSampler _pixelSampler;
        Vulkan::TextureSampler _linearSampler;

        bool _framebufferResized;
        Utils::Vec2F _framebufferSize;

        std::vector<TextureMap> _textureMaps;

    };

}
}

#endif