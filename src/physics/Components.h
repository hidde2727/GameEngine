#ifndef ENGINE_PHYSICS_COMPONENTS_H
#define ENGINE_PHYSICS_COMPONENTS_H

#include "core/PCH.h"
#include "util/Vec2D.h"

namespace Engine {

    struct VelocityComponent {
        Util::Vec2F velocity;
        float angular;
        Util::Vec2F velocityDamping;// x is damping proportional to v, y is damping proportional to v^2
        Util::Vec2F angularDamping;// x is damping proportional to v, y is damping proportional to v^2
    };

    enum ColliderFlags {
        Continuos=1,
        NoMove=2,
        NoVelocityChanges=4
    };
    struct RectangleColliderComponent {
        RectangleColliderComponent(Util::Vec2F size) : _size(size), flags(0), restitution(0) {}
        RectangleColliderComponent(Util::Vec2F size, uint8_t flags) : _size(size), flags(flags), restitution(0) {}
        RectangleColliderComponent(Util::Vec2F size, uint8_t flags, float restitution) : _size(size), flags(flags), restitution(restitution) {}

        uint8_t flags;
        float restitution = 0;
        Util::Vec2F _size;
    };

}

#endif