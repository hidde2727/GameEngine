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
#include "util/FileManager.h"

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

#ifndef ENGINE_ENABLE_DEBUG_GRAPHICS
    #define ENGINE_ENABLE_DEBUG_GRAPHICS __DEBUG__
#endif

#define ENGINE_RENDERER_ASSETTYPE_TEXTURE 1
#define ENGINE_RENDERER_ASSETTYPE_TEXT 2

namespace Engine {
namespace Renderer {

    struct VertexDataRect {
        uint32_t vertex;
    };
    struct VertexDataText {
        Util::Vec2F dimensions;
    };
    struct InstanceDataRect {
        InstanceDataRect(const Component::Position::Precalculated area, const Util::Vec3F color, const Util::Vec2F texturePos, const Util::Vec2F textureDimensions, const uint32_t texture)
         : topLeft(area._topLeft), bottomRight(area._bottomRight), deltaPosition(area._deltaPosition), color(color), texturePos(texturePos), textureDimensions(textureDimensions), texture(texture) {}
        Util::Vec2F topLeft;
        Util::Vec2F bottomRight;
        Util::Vec2F deltaPosition;
        Util::Vec3F color;
        Util::Vec2F texturePos;
        Util::Vec2F textureDimensions;
        uint32_t texture;
    };
    struct InstanceDataText {
        InstanceDataText(const Util::Vec2F pos, const Util::Vec2F dimensions, const Util::Vec3F color, const Util::Vec2F texturePos, const Util::Vec2F textureDimensions, const uint32_t texture, const float pxRange)
         : pos(pos), dimensions(dimensions), color(color), texturePos(texturePos), textureDimensions(textureDimensions), texture(texture), pxRange(pxRange) {}
        Util::Vec2F pos;
        Util::Vec2F dimensions;
        Util::Vec3F color;
        Util::Vec2F texturePos;
        Util::Vec2F textureDimensions;
        uint32_t texture;
        float pxRange;
    };
    typedef uint32_t AssetID;

    class Window {
    public:

        void Init(const uint32_t textureMapSlots, const Util::FileManager& fileManager);
        void Cleanup();

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

        void SetCameraPosition(const Util::Vec2F pos);

        std::shared_ptr<ImageRenderInfo> GetTextureInfo(AssetID asset);
        std::shared_ptr<TextRenderInfo> GetTextInfo(AssetID asset);

        std::string GetVulkanDeviceLimits();

#if ENGINE_ENABLE_DEBUG_GRAPHICS
        // Makes sure the next frame a line gets drawn on the screen, will only last for one frame
        void AddDebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color) {
            _debugLines.push_back(DebugLine{start, color, end, color});
        }
#else
        // Makes sure the next frame a line gets drawn on the screen, will only last for one frame
        void AddDebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color) {}
#endif

    private:
        Util::FileManager const* _fileManager = nullptr;

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

        Vulkan::EfficientVertexBuffer _vkRectVertexBuffer;
        Vulkan::EfficientVertexBuffer _vkTextVertexBuffer;
        Vulkan::IndexBuffer _vkIndexBuffer;
        Vulkan::EfficientVertexBuffer _vkRectPerVertexBuffer;
        Vulkan::EfficientVertexBuffer _vkTextPerVertexBuffer;

        Vulkan::TextureSampler _pixelSampler;
        Vulkan::TextureSampler _linearSampler;

        bool _framebufferResized;
        Util::Vec2F _framebufferSize;
        Util::Vec2F _cameraPosition = Util::Vec2F(0);

        std::vector<TextureMap> _textureMaps;

#if ENGINE_ENABLE_DEBUG_GRAPHICS
        Vulkan::Pipeline _vkDebugPipeline;
        Vulkan::EfficientVertexBuffer _vkDebugVertexBuffer;
        struct DebugLine {
            Util::Vec2F _start;
            Util::Vec3F _colorStart;
            Util::Vec2F _end;
            Util::Vec3F _colorEnd;
        };
        std::vector<DebugLine> _debugLines;
#endif
    };

}
}

#endif