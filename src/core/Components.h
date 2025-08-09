#ifndef ENGINE_CORE_COMPONENTS_H
#define ENGINE_CORE_COMPONENTS_H

#include "core/PCH.h"
#include "util/Vec2D.h"
#include "util/Area.h"

namespace Engine {

    struct PositionComponent {
		PositionComponent(const float value) : _pos(value) {}
		PositionComponent(const float x, const float y) : _pos(x, y) {}
		PositionComponent(const float x, const float y, const float rotation) : _pos(x, y), _rotation(rotation) {}
        PositionComponent(Util::Vec2F pos) : _pos(pos) {}
        PositionComponent(Util::Vec2F pos, const float rotation) : _pos(pos), _rotation(rotation) {}

        struct Precalculated {
            Util::Vec2F _topLeft;
            Util::Vec2F _bottomRight;
            Util::Vec2F _deltaPosition;
        };
        Precalculated GetPrecalculated(const float w, const float h);
        struct Corners {
            Util::Vec2F _points[4];
        };
        Corners GetCornerPositions(const float w, const float h);

        Util::Vec2F _pos;
        float _rotation=0;
    };

    class Scene;
    struct TextureComponent {
        
        TextureComponent(Scene* scene, const uint32_t assetID, const Util::Vec2F size);

        Util::AreaF _textureArea;
        uint32_t _descriptorID;
        Util::Vec2F _size;
    };
    typedef TextureComponent QRCodeComponent;

    struct TextComponent {
        TextComponent(Scene* scene, uint32_t assetID, const uint32_t size, const std::u32string text);
        
        struct CharRenderInfo {
            Util::AreaF _position;
            Util::AreaF _textureArea;
            uint32_t _descriptorID;
            float _pxRange;
        };
        std::vector<CharRenderInfo> _renderInfo;
    };


}

#endif