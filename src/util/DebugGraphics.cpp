#include "util/DebugGraphics.h"

namespace Engine {
namespace Util {

    Game* target = nullptr;
    void SetDebugGraphicsTarget(Game* game) {
        target = game;
    }
    void SetDebugGraphicsTargetIfNull(Game* game) {
        if(target == nullptr) target = game;
    }

    void DrawDebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color) {
        ASSERT(target==nullptr, "Cannot draw debug graphics without a game set as the target");
        target->DebugLine(start, end, color);
    }

    void DrawDebugQuad(const Util::Vec2F middle, const float halfWidth, const Util::Vec3F color) {
        ASSERT(target==nullptr, "Cannot draw debug graphics without a game set as the target");
        Util::Vec2F p1 = (middle+Util::Vec2F( halfWidth,  halfWidth));
        Util::Vec2F p2 = (middle+Util::Vec2F(-halfWidth,  halfWidth));
        Util::Vec2F p3 = (middle+Util::Vec2F(-halfWidth, -halfWidth));
        Util::Vec2F p4 = (middle+Util::Vec2F( halfWidth, -halfWidth));
        target->DebugLine(p1, p2, color);
        target->DebugLine(p2, p3, color);
        target->DebugLine(p3, p4, color);
        target->DebugLine(p4, p1, color);
    }
    void DrawDebugQuad(const Util::Vec2F middle, const float halfWidth, const float rotation, const Util::Vec3F color) {
        ASSERT(target==nullptr, "Cannot draw debug graphics without a game set as the target");
        Util::Vec2F p1 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation);
        Util::Vec2F p2 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation+0.5f*PI);
        Util::Vec2F p3 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation+1.0f*PI);
        Util::Vec2F p4 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation+1.5f*PI);
        target->DebugLine(p1, p2, color);
        target->DebugLine(p2, p3, color);
        target->DebugLine(p3, p4, color);
        target->DebugLine(p4, p1, color);
    }
    void DrawDebugQuad(
        const Util::Vec2F middle, const float halfWidth, const float rotation, 
        const Util::Vec3F color1, const Util::Vec3F color2, const Util::Vec3F color3, const Util::Vec3F color4
    ) {
        ASSERT(target==nullptr, "Cannot draw debug graphics without a game set as the target");
        Util::Vec2F p1 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation);
        Util::Vec2F p2 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation+0.5f*PI);
        Util::Vec2F p3 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation+1.0f*PI);
        Util::Vec2F p4 = (middle+Util::Vec2F(halfWidth, halfWidth)).rotate(rotation+1.5f*PI);
        target->DebugLine(p1, p2, color1);
        target->DebugLine(p2, p3, color2);
        target->DebugLine(p3, p4, color3);
        target->DebugLine(p4, p1, color4);
    }
    
    void DrawDebugArrow(const Util::Vec2F start, const Util::Vec2F end, const float width, const Util::Vec3F color) {
        ASSERT(target==nullptr, "Cannot draw debug graphics without a game set as the target");
        target->DebugLine(start, end, color);
        target->DebugLine(end, end-Util::Vec2F( width, -width).rotate(end.angleToY()), color);
        target->DebugLine(end, end-Util::Vec2F(-width, -width).rotate(end.angleToY()), color);
    }
    void DrawDebugArrow(const Util::Vec2F start, const Util::Vec2F direction, const float length, const float width, const Util::Vec3F color) {
        ASSERT(target==nullptr, "Cannot draw debug graphics without a game set as the target");
        const Util::Vec2F end = start + direction.normalized()*length;
        target->DebugLine(start, end, color);
        target->DebugLine(end, end-Util::Vec2F( width, -width).rotate(end.angleToY()), color);
        target->DebugLine(end, end-Util::Vec2F(-width, -width).rotate(end.angleToY()), color);
    }

}
}