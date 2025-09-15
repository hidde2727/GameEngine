#include "core/Components.h"

#include "core/Scene.h"

namespace Engine {
namespace Component {

    Position::Precalculated Position::GetPrecalculated(const float w, const float h) {
        Util::Vec2F middle = Util::Vec2F(_pos.x, _pos.y);
        Precalculated ret;
        ret._topLeft = Util::Vec2F(_pos.x - 0.5f*w, _pos.y - 0.5f*h).rotate(_rotation, middle);
        ret._bottomRight = Util::Vec2F(_pos.x + 0.5f*w, _pos.y + 0.5f*h).rotate(_rotation, middle);
        ret._deltaPosition = Util::Vec2F(_pos.x + 0.5f*w, _pos.y-0.5f*h).rotate(_rotation, middle) - ret._topLeft;
        return ret;
    }
    Position::Corners Position::GetCornerPositions(const float w, const float h) {
        Util::Vec2F middle = Util::Vec2F(_pos.x, _pos.y);
        Corners ret;
        ret._points[0] = Util::Vec2F(_pos.x - 0.5f*w, _pos.y - 0.5f*h).rotate(_rotation, middle);
        ret._points[1] = Util::Vec2F(_pos.x + 0.5f*w, _pos.y - 0.5f*h).rotate(_rotation, middle);
        ret._points[2] = Util::Vec2F(_pos.x + 0.5f*w, _pos.y + 0.5f*h).rotate(_rotation, middle);
        ret._points[3] = Util::Vec2F(_pos.x - 0.5f*w, _pos.y + 0.5f*h).rotate(_rotation, middle);
        return ret;
    }

    Texture::Texture(Scene* scene, const uint32_t assetID, const Util::Vec2F size) {
        std::shared_ptr<Renderer::ImageRenderInfo> info = scene->_window->GetTextureInfo(assetID);
        _textureArea = info->first;
        _descriptorID = info->second;
        _size = size;
    }

    Text::Text(Scene* scene, uint32_t assetID, const uint32_t size, std::u32string text) {
        std::shared_ptr<Renderer::TextRenderInfo> info = scene->_window->GetTextInfo(assetID);
        ASSERT(!(info), "Text size is not loaded, cannot create a text component of a size that is not loaded")
        _renderInfo.reserve(text.size());
        float currentX = 0;
        float currentY = 0;
        for(uint32_t i = 0; i < text.size(); i++) {
            Renderer::CharInfo charInfo = info->at(size)[text[i]];
            if(charInfo._boundTexture!=UINT32_MAX && charInfo._textureArea.w!=0 && charInfo._textureArea.h!=0) {
                float x = currentX + (i>0?charInfo._horizontalKerning[text[i-1]]+charInfo._leftSideBearing : 0);
                _renderInfo.push_back({
                    Util::AreaF(
                        std::floor(x - ENGINE_RENDERER_SDF_PADDING) + charInfo._min.x,
                        currentY - charInfo._max.y - ENGINE_RENDERER_SDF_PADDING,
                        charInfo._max.x - charInfo._min.x + 2*ENGINE_RENDERER_SDF_PADDING,
                        charInfo._max.y - charInfo._min.y + 2*ENGINE_RENDERER_SDF_PADDING
                    ),
                    charInfo._textureArea,
                    charInfo._boundTexture,
                    (float)ENGINE_RENDERER_PX_RANGE_FACTOR*sqrt((charInfo._max.x-charInfo._min.x)*(charInfo._max.x-charInfo._min.x)+(charInfo._max.y-charInfo._min.y)*(charInfo._max.y-charInfo._min.y))
                });
            }
            currentX += charInfo._advance;
        }
    }

}
}