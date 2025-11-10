#ifndef ENGINE_PHYSICS_COMPONENTS_H
#define ENGINE_PHYSICS_COMPONENTS_H

#include "core/PCH.h"
#include "physics/Shapes.h"
#include "physics/AABB.h"

namespace Engine {
namespace Component {

    struct Acceleration {
        Util::Vec2F a;// acceleration;
        float alpha;// angular acceleration
    };

    struct Velocity {
        Velocity() {}
        Velocity(const Util::Vec2F v, const float w) : v(v), w(w) {}

        Util::Vec2F v = Util::Vec2F(0);// velocity
        float w = 0;// angular velocity

        inline Velocity& operator+=(const Velocity& other) {
            v += other.v;
            w += other.w;
            return *this;
        }
        inline Velocity& operator/=(const float a) {
            v /= a;
            w /= a;
            return *this;
        }

        // Internal:
        Util::Vec2F newV = Util::Vec2F(0);
        float newW = 0;
        float divisionFactor = 0;
    };

    enum ColliderFlags {
        Continuos=1,// Not implemented yet
        NoMove=2,
        NoVelocityChanges=4,
        Kinematic=8,

        Polygon=256,
        Rectangle=512,
        Circle=1024,

        Internal=32768// Used to indicate CollisionManifold needs to cleanup the Collider component
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
        static PhysicsMaterial Sticky() { return PhysicsMaterial(0.2f, 0.6f, 0.6f); }
        static PhysicsMaterial UltraSticky() { return PhysicsMaterial(0.f, 1.f, 1.f); }
    };

    struct ColliderUUID {
        ColliderUUID() {}
        ColliderUUID(const uint64_t uuid) : uuid(uuid) {}

        uint64_t uuid;
    };

    struct Collider {
        Collider() {}
        Collider(const Collider& col);
        ~Collider();
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
        float gravityFactor = 1;// multiplied with gravity and then added to the velocity

        bool IsStatic() const;
        bool IsKinematic() const;
        Physics::AABB GetAABB(const Component::Position& pos) const;

        static Collider StaticRect(const Util::Vec2F size, const PhysicsMaterial mat = PhysicsMaterial::Default());
        static Collider StaticRect(const Util::Vec2F size, const uint16_t flags, const PhysicsMaterial mat = PhysicsMaterial::Default());
        static Collider KinematicRect(const Util::Vec2F size, const PhysicsMaterial mat = PhysicsMaterial::Default());
        static Collider DynamicRect(const Util::Vec2F size, const PhysicsMaterial mat = PhysicsMaterial::Default());

        void RecalculateMass(const float density);
    };

    struct ImageBasedColliderID {
        uint32_t id;

        uint64_t firstStaticBody = std::numeric_limits<uint64_t>::infinity();
        uint64_t lastStaticBody = std::numeric_limits<uint64_t>::infinity();
    };
    struct ImageBasedCollider {
        ImageBasedCollider() {}
        ImageBasedCollider(const std::string file, const Util::Vec2F forcedSize = Util::Vec2F(0), const PhysicsMaterial mat = PhysicsMaterial::Default()) {
            _file = file;
            _material = mat;
            _forcedSize = forcedSize;
        }
        std::string _file;
        PhysicsMaterial _material;
        Util::Vec2F _forcedSize = Util::Vec2F(0);
        uint8_t _colliderColor = 0;// The color in the image that should be converted to become a collider (image is first converted to black and white)
    };

}
}

#endif