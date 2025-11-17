#ifndef ENGINE_RENDERER_TEXTUREMAP_H
#define ENGINE_RENDERER_TEXTUREMAP_H

#include "core/PCH.h"

#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Texture.h"
#include "renderer/vulkan/CommandBuffer.h"
#include "renderer/RectanglePacker.h"

#include "util/Vec2D.h"
#include "util/Vec3D.h"
#include "util/Area.h"
#include "util/FileManager.h"

#ifndef ENGINE_RENDERER_MAX_IMAGE_SIZE
#define ENGINE_RENDERER_MAX_IMAGE_SIZE Util::Vec2U32(1920, 1080)
#endif

namespace Engine {
namespace Renderer {

    class AssetLoader {
    public:
        // Will be the first function called after intialising this class
        // fileManager will be set
        virtual void Init() {}
        // Should return a number of textures that this TextureLoader provides
        // Can at no moment return something else then it returned the first time GetAmountTextures was called
        virtual size_t GetAmountTextures() = 0;
        // Should set the start* till the start+GetAmountTextures() to the sizes the textures should be
        // Only the x and y has to be set, the z is for internal use
        // If the cache is not used, this function will be the first to be called (you can init your loading utilities here)
        // Garantueed to only be called once
        virtual void SetTextureSizes(Util::Vec3U32* start) = 0;
        // Should render the texture with requested id on the requested texture at the requested area
        // ID=n is the nth texture returned in SetTextureSizes
        virtual void RenderTexture(Util::AreaU8* texture, const Util::Vec2U32 textureSize, const Util::AreaU32 area, const size_t id) = 0;

        // Called with the area and descriptor binding for the ID
        // Area is different from the area in the RenderTexture function because this area is normalized (to the size of the texture)
        // ID=n is the nth texture returned in SetTextureSizes
        virtual void SetTextureRenderInfo(const Util::AreaF area, const uint32_t boundTexture, const size_t id) = 0;

        // Called after every texture has been called to render
        // Needs to return the information neccesary to render the assets
        virtual std::shared_ptr<uint8_t> GetRenderInfo() = 0;
    
    private:
        friend class TextureMap;
        size_t _firstTexture = 0;
        size_t _lastTexture = 0;        
    };

    class TextureMap {
    public:

        void Init();
        void Cleanup(Vulkan::Context& context, std::initializer_list<Vulkan::Pipeline*> boundToPipelines);

        void StartLoading();
        uint32_t AddTextureLoader(std::shared_ptr<AssetLoader> textureLoader);
        void SetCacheName(const std::string name);
        void EndLoading(Vulkan::Context& context, std::initializer_list<Vulkan::Pipeline*> bindToPipelines);

        std::shared_ptr<uint8_t> GetRenderInfo(const uint32_t id);

    private:
    
        std::vector<std::shared_ptr<AssetLoader>> _assetLoaders;
        std::vector<std::shared_ptr<uint8_t>> _renderInfos;// Reinterpret casted pointers
        size_t _amountTextures;
        std::string _cacheName;
        std::vector<Vulkan::Texture> _textures;

    };

}
}

#endif