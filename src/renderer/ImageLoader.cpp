#include "renderer/ImageLoader.h"

namespace Engine {
namespace Renderer {

    ImageLoader::ImageLoader(const std::string file) {
        _file = file;
    }
    
    void ImageLoader::Init() {
        ASSERT(Util::FileManager::DoesFileExists(_file), "[Renderer::ImageLoader] Received an image that does not exist (" + _file + ")")
        ASSERT(Util::FileManager::IsFileRegular(_file), "[Renderer::ImageLoader] Received an invalid file (" + _file + ")")
    }
    size_t ImageLoader::GetAmountTextures() {
        return 1;
    }
    void ImageLoader::SetTextureSizes(Util::Vec3U32* start) {
        int x, y, n;
        bool success = Util::FileManager::GetImageInfo(_file, &x, &y, &n);
        ASSERT(success, "[Renderer::ImageLoader] Failed to load image info with the stbi_info function:\n" + Util::FileManager::GetImageReadError())

        *start = Util::Vec3U32(x, y, 0);
    }
    void ImageLoader::RenderTexture(Util::AreaU8* texture, const Util::Vec2U32 textureSize, const Util::AreaU32 area, const size_t id) {
        _area = area;
        int width, height, channels;
        std::shared_ptr<uint8_t> imageDataPtr = Util::FileManager::ReadImageFile(_file, &width, &height, &channels, 4);
        uint8_t* imageData = imageDataPtr.get();

        for(int y = 0; y < height; y++) {
            Util::AreaU8* row = texture + (area.y+y)*textureSize.x + area.x;
            for(int x = 0; x < width; x++) {
                uint8_t* pixelLocation = imageData + (y*width + x) * 4;
                *(row + x) = Util::AreaU8(*pixelLocation, *(pixelLocation+1), *(pixelLocation+2), *(pixelLocation+3));
            }
        }
    }
    
    void ImageLoader::SetTextureRenderInfo(const Util::AreaF area, const uint32_t boundTexture, const size_t id) {
        _area = area;
        _descriptorArrayID = boundTexture;
    }
    std::shared_ptr<uint8_t> ImageLoader::GetRenderInfo() {
        return std::reinterpret_pointer_cast<uint8_t>(std::make_shared<ImageRenderInfo>(_area, _descriptorArrayID));
    }

}
}