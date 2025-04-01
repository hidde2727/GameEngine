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

namespace Engine {
namespace Renderer {

    class AssetLoader {
    public:
        // Should return if it possible to use the cache for this AssetLoader
        virtual bool CanUseCache(const std::filesystem::file_time_type lastCacheChange) = 0;
        // Should return a number of textures that this TextureLoader provides
        // Can at no moment return something else then it returned the first time GetAmountTextures was called
        virtual size_t GetAmountTextures() = 0;
        // Should set the start* till the start+GetAmountTextures() to the sizes the mapped textures should be
        // Only the x and y has to be set, the z is for internal use
        // If the cache is not used, this function will be the first to be called (you can init your loading utilities here)
        // Garantueed to only be called once
        virtual void SetTextureSizes(Utils::Vec3U32* start) = 0;
        // Should render the textures on the returned texture at the returned areas
        // ID=n is the nth texture returned in SetTextureSizes
        virtual void RenderTexture(Utils::AreaU8* texture, const Utils::Vec2U32 textureSize, const Utils::AreaU32 area, const size_t id) = 0;

        // Called with the area and descriptor binding for the ID
        // Area is different from render texture that it is normalized
        // ID=n is the nth texture returned in SetTextureSizes
        virtual void SetTextureRenderInfo(const Utils::AreaF area, const uint32_t boundTexture, const size_t id) = 0;

        // Called after every texture has been called to render
        // Needs to return the information neccesary to render the assets
        virtual std::shared_ptr<uint8_t> GetRenderInfo() = 0;
        // Called if the information was all cached, should return the same as GetRenderInfo() but from the cached data
        virtual std::shared_ptr<uint8_t> GetRenderInfo(uint8_t* cache, const size_t size) = 0;

        // Should return the bytes neccesary to reconstruct the result of GetRenderInfo()
        virtual std::shared_ptr<uint8_t> GetCacheData() = 0;

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
        uint32_t AddTextureLoader(std::unique_ptr<AssetLoader> textureLoader);
        void SetCacheName(const std::string name);
        void EndLoading(Vulkan::Context& context, std::initializer_list<Vulkan::Pipeline*> bindToPipelines);

        std::shared_ptr<uint8_t> GetRenderInfo(const uint32_t id);

    private:
    
        std::vector<std::unique_ptr<AssetLoader>> _assetLoaders;
        std::vector<std::shared_ptr<uint8_t>> _renderInfos;// Reinterpret casted pointers
        size_t _amountTextures;
        std::string _cacheName;
        std::vector<Vulkan::Texture> _textures;

    };

}
}

#endif