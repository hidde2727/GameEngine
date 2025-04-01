#ifndef ENGINE_CORE_COMPONENTS_H
#define ENGINE_CORE_COMPONENTS_H

#include "core/PCH.h"
#include "util/Area.h"

namespace Engine {

    typedef Utils::AreaF AreaComponent;

    class Scene;
    struct TextureComponent {
        
        TextureComponent(Scene* scene, uint32_t assetID);

        Utils::AreaF _textureArea;
        uint32_t _descriptorID;
    };
    typedef TextureComponent QRCodeComponent;

    struct TextComponent {
        TextComponent(Scene* scene, uint32_t assetID, const uint32_t size, const std::u32string text);
        
        struct CharRenderInfo {
            Utils::AreaF _position;
            Utils::AreaF _textureArea;
            uint32_t _descriptorID;
            float _pxRange;
        };
        std::vector<CharRenderInfo> _renderInfo;
    };


}

#endif