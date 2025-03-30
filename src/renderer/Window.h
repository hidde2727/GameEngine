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

#include "util/Vec2D.h"
#include "util/Vec3D.h"

// The amount of bits the asset vs the texturemap will use of the AssetID
// Defaults to 24 bits for the asset and 8 bits for the texturemap
#ifndef ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS
    #define ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS 24
#endif

namespace Engine {
namespace Renderer {

    struct Vertex {
        Vertex(const Utils::Vec2F pos, const Utils::Vec3F color, const Utils::Vec2F texturePos, uint32_t texture) : pos(pos), color(color), texturePos(texturePos), texture(texture) {}
        Utils::Vec2F pos;
        Utils::Vec3F color;
        Utils::Vec2F texturePos;
        uint32_t texture;
    };
    typedef uint32_t AssetID;

    class Window {
    public:

        Window(const uint32_t textureMapSlots);
        ~Window();

        bool ShouldClose();
        void Update();
        void Draw(entt::registry& registry, uint32_t amountRectangles);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        static void FramebufferResize(GLFWwindow* window, int width, int height);

        void StartAssetLoading(const size_t textureMapID);
        void SetAssetLoadingCacheName(const size_t textureMapID, const std::string cacheName);
        AssetID AddAsset(const size_t textureMapID, std::unique_ptr<AssetLoader> textureLoader);
        void EndAssetLoading(const size_t textureMapID);
        void CleanupAssets(const size_t textureMapID);

        std::pair<Utils::AreaF, uint32_t>* GetTextureInfo(AssetID asset);

    private:

        GLFWwindow* _window = nullptr;
        Vulkan::Context _vkContext;
        Vulkan::RenderPass _vkRenderPass;
        Vulkan::Swapchain _vkSwapchain;
        Vulkan::Pipeline _vkPipeline;
        Vulkan::CommandBuffer _vkCommandBuffer;

        Vulkan::Fence _vkInFlightFence;
        Vulkan::Semaphore _vkImageAvailableSemaphore;
        Vulkan::Semaphore _vkRenderFinishedSemaphore;

        Vulkan::EfficientVertexBuffer _vkVertexBuffer;
        Vulkan::IndexBuffer _vkIndexBuffer;

        Vulkan::TextureSampler _pixelSampler;
        Vulkan::TextureSampler _linearSampler;

        bool _framebufferResized;

        std::vector<TextureMap> _textureMaps;
    };

}
}

#endif