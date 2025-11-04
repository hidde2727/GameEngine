#ifndef ENGINE_UTIL_DEBUG_GRAPHICS_H
#define ENGINE_UTIL_DEBUG_GRAPHICS_H

#include "core/PCH.h"
#include "core/Game.h"

namespace Engine {
namespace Util {

    void SetDebugGraphicsTarget(Game* game);
    void SetDebugGraphicsTargetIfNull(Game* game);

    void DrawDebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color);

    void DrawDebugQuad(const Util::Vec2F middle, const float halfWidth, const Util::Vec3F color);
    void DrawDebugQuad(const Util::Vec2F middle, const float halfWidth, const float rotation, const Util::Vec3F color);
    void DrawDebugQuad(
        const Util::Vec2F middle, const float halfWidth, const float rotation, 
        const Util::Vec3F color1, const Util::Vec3F color2, const Util::Vec3F color3, const Util::Vec3F color4
    );
    void DrawDebugQuad(const Util::Vec2F middle, const Util::Vec2F halfWidth, const Util::Vec3F color);
    void DrawDebugQuad(const Util::Vec2F middle, const Util::Vec2F halfWidth, const float rotation, const Util::Vec3F color);
    void DrawDebugQuad(
        const Util::Vec2F middle, const Util::Vec2F halfWidth, const float rotation, 
        const Util::Vec3F color1, const Util::Vec3F color2, const Util::Vec3F color3, const Util::Vec3F color4
    );

    void DrawDebugArrow(const Util::Vec2F start, const Util::Vec2F end, const float width, const Util::Vec3F color);
    void DrawDebugArrow(const Util::Vec2F start, const Util::Vec2F direction, const float length, const float width, const Util::Vec3F color);

}
}

#endif