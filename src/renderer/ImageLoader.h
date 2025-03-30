#ifndef ENGINE_RENDERER_IMAGELOADER_H
#define ENGINE_RENDERER_IMAGELOADER_H

#include "core/PCH.h"
#include "renderer/TextureMap.h"

namespace Engine {
namespace Renderer {

    class ImageLoader : public AssetLoader {
    public:

        ImageLoader(const std::string file);

        bool CanUseCache(const std::filesystem::file_time_type lastCacheChange) override;
        size_t GetAmountTextures() override;
        void SetTextureSizes(Utils::Vec3U32* start) override;
        void RenderTexture(Utils::AreaU8* texture, const Utils::AreaU32 area, const size_t id) override;

        void SetTextureRenderInfo(const Utils::AreaF area, const uint32_t boundTexture, const size_t id) override;
        std::vector<uint8_t> GetRenderInfo() override;
        std::vector<uint8_t> GetRenderInfo(uint8_t* cache, const size_t size) override;
        std::vector<uint8_t> GetCacheData() override;


    private:
        
        std::string _file;
        Utils::AreaF _area;
        uint32_t _descriptorArrayID;

    };

}
}

#endif