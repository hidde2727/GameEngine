#ifndef ENGINE_PHYSICS_COMPONENTS_H
#define ENGINE_PHYSICS_COMPONENTS_H

#include "core/PCH.h"
#include "util/Vec2D.h"

namespace Engine {

    struct VelocityComponent {
        VelocityComponent() {}
        VelocityComponent(const float v) : velocity(v), angular(v), velocityDamping(v), angularDamping(v) {}
        VelocityComponent(const Util::Vec2F v, const float a) : velocity(v), angular(a) {}
        VelocityComponent(const Util::Vec2F v, const float a, const float velocityDamping, const float angularDamping) : velocity(v), angular(a), velocityDamping(velocityDamping), angularDamping(angularDamping) {}
        VelocityComponent(const Util::Vec2F v, const float a, const Util::Vec2F velocityDamping, const Util::Vec2F angularDamping) : velocity(v), angular(a), velocityDamping(velocityDamping), angularDamping(angularDamping) {}


        Util::Vec2F velocity = {0,0};
        float angular = 0;
        Util::Vec2F velocityDamping = { 0.001f, 0.00001f };// x is damping proportional to v, y is damping proportional to v^2
        Util::Vec2F angularDamping = { 0.01f, 0.001f };// x is damping proportional to v, y is damping proportional to v^2

        operator std::string() const {
            return "{ velocity: " + (std::string)velocity + ", angular: " + std::to_string(angular) + " }";
        }
    };
    struct RotationToVelocity {

    };
    struct AccelerationComponent {
        Util::Vec2F acceleration;
        float angular;
        Util::Vec2F accelerationDamping;// x is damping proportional to v, y is damping proportional to v^2
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
        float restitution = 0.5;
        float mass = 1;
        float momentOfIntertia = 1;
        Util::Vec2F _size;
    };

}

#endif