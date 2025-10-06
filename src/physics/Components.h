#ifndef ENGINE_PHYSICS_COMPONENTS_H
#define ENGINE_PHYSICS_COMPONENTS_H

#include "core/PCH.h"
#include "physics/Shapes.h"

namespace Engine {
namespace Component {

    struct Acceleration {
        Util::Vec2F a;// acceleration;
        float alpha;// angular acceleration
    };

    struct Velocity {
        Util::Vec2F v;// velocity
        float w;// angular velocity
    };

    enum ColliderFlags {
        Continuos=1,
        NoMove=2,
        NoVelocityChanges=4,
        Kinematic=8,

        Polygon=256,
        Rectangle=512,
        Circle=1024
    };

    struct PhysicsMaterial {
        float e = 1;
        float sf = 0.5f;
        float df = 0.3f;

        PhysicsMaterial() {}
        PhysicsMaterial(const float e, const float sf, const float df) : e(e), sf(sf), df(df) {}

        static PhysicsMaterial Default() { return PhysicsMaterial(0.5f, 0.5f, 0.3f); }
        static PhysicsMaterial Bouncy() { return PhysicsMaterial(0.9f, 0.2f, 0.1f); }
        static PhysicsMaterial UltraBouncy() { return PhysicsMaterial(1.f, 0.f, 0.f); }
        static PhysicsMaterial Rock() { return PhysicsMaterial(0.3f, 0.6f, 0.4f); }
    };

    struct Collider {
        Collider() {}
        Collider(const Collider& col);
        Collider& operator=(const Collider& c);

        struct Empty {};
        union Shape {
            Physics::Polygon polygon;
            Physics::Rectangle rectangle;
            Physics::Circle circle;
            Empty empty;
            Shape() : empty() {}
        } shape;
        uint16_t flags;
        float e = 1;// restitution
        float im = 1;// inverse mass
        float iL = 0.5;// inverse inertia
        float sf = 0.5f;// static friction coefficient
        float df = 0.3f;// dynamic friction coefficient

        bool IsStatic() const;
        bool IsKinematic() const;

        static Collider StaticRect(const Util::Vec2F size, const PhysicsMaterial mat = PhysicsMaterial::Default());
        static Collider StaticRect(const Util::Vec2F size, const uint16_t flags, const PhysicsMaterial mat = PhysicsMaterial::Default());
        static Collider KinematicRect(const Util::Vec2F size, const PhysicsMaterial mat = PhysicsMaterial::Default());
        static Collider DynamicRect(const Util::Vec2F size, const PhysicsMaterial mat = PhysicsMaterial::Default());

        void RecalculateMass(const float density);
    };

}
}

#endif