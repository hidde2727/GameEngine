#ifndef ENGINE_PHYSICS_AABB_H
#define ENGINE_PHYSICS_AABB_H

#include "core/PCH.h"

namespace Engine {
namespace Physics {

    struct AABB {
        AABB() {}
        
        bool ContainedIn(const AABB& other) const {
            if(_topLeft.x < other._topLeft.x) return false;
            if(_bottomRight.x > other._bottomRight.x) return false;
            if(_topLeft.y < other._topLeft.y) return false;
            if(_bottomRight.y > other._bottomRight.y) return false;
            return true;
        }
        bool HasOverlap(const AABB& other) const {
            if(_topLeft.x > other._bottomRight.x) return false;
            if(_bottomRight.x < other._topLeft.x) return false;
            if(_topLeft.y > other._bottomRight.y) return false;
            if(_bottomRight.y < other._topLeft.y) return false;
            return true;
        }
        Util::Vec2F GetMiddle() const {
            return (_bottomRight+_topLeft)*0.5f;
        }
        Util::Vec2F GetHalfDimensions() const {
            return Util::abs(_bottomRight-_topLeft)*0.5f;
        }

        Util::Vec2F _topLeft;
        Util::Vec2F _bottomRight;

        static AABB FromMiddleAndDimensions(const Util::Vec2F middle, const Util::Vec2F halfDimensions) {
            AABB aabb{};
            aabb._topLeft = middle - halfDimensions;
            aabb._bottomRight = middle + halfDimensions;
            return aabb;
        }
        static AABB FromCorners(const Util::Vec2F topLeft, const Util::Vec2F bottomRight) {
            AABB aabb{};
            aabb._topLeft = topLeft;
            aabb._bottomRight = bottomRight;
            return aabb;
        }
    };

}
}

#endif