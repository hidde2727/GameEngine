#ifndef ENGINE_RENDERER_IMAGELOADER_H
#define ENGINE_RENDERER_IMAGELOADER_H

#include "core/PCH.h"
#include "renderer/TextureMap.h"

namespace Engine {
namespace Renderer {

    typedef std::pair<Util::AreaF, uint32_t> ImageRenderInfo;
    class ImageLoader : public AssetLoader {
    public:

        ImageLoader(const std::string file);

        bool CanUseCache(const std::filesystem::file_time_type lastCacheChange) override;
        size_t GetAmountTextures() override;
        void SetTextureSizes(Util::Vec3U32* start) override;
        void RenderTexture(Util::AreaU8* texture, const Util::Vec2U32 textureSize, const Util::AreaU32 area, const size_t id) override;

        void SetTextureRenderInfo(const Util::AreaF area, const uint32_t boundTexture, const size_t id) override;
        std::shared_ptr<uint8_t> GetRenderInfo() override;
        std::shared_ptr<uint8_t> GetRenderInfo(uint8_t* cache, const size_t size) override;
        std::shared_ptr<uint8_t> GetCacheData() override;


    private:
        
        std::string _file;
        Util::AreaF _area;
        uint32_t _descriptorArrayID;

    };

}
}

#endif