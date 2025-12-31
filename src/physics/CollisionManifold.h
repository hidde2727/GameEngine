#ifndef ENGINE_PHYSICS_COLLISIONMANFIOLD_H
#define ENGINE_PHYSICS_COLLISIONMANFIOLD_H

#include "core/PCH.h"
#include "core/Components.h"
#include "physics/Components.h"

namespace Engine {
namespace Physics {

    class CollisionManifold {
    public:

        CollisionManifold(
            Component::Collider* a, Component::Position* posA, Component::Velocity* velA, 
            Component::Collider* b, Component::Position* posB, Component::Velocity* velB
        );
        CollisionManifold(const CollisionManifold& c) = delete;
        CollisionManifold(CollisionManifold&& c) noexcept {
            *this = std::move(c);// operator=(CollisionManifold&&)
        }
        ~CollisionManifold();
        CollisionManifold& operator=(const CollisionManifold& c) = delete;
        CollisionManifold& operator=(CollisionManifold&& c) noexcept {
            _a = c._a;
            _posA = c._posA;
            _velA = c._velA;
            _b = c._b;
            _posB = c._posB;
            _velB = c._velB;

            _normal = std::move(c._normal);
            _penetration = c._penetration;
            _contacts[0] = std::move(c._contacts[0]);
            _contacts[1] = std::move(c._contacts[1]);
            _contactCount = c._contactCount;
            _bIsRef = c._bIsRef;
            // Make sure the colliders aren't destructed when 'col' is destructed
            c._a = nullptr;
            c._posA = nullptr;
            c._b = nullptr;
            c._posB = nullptr;
            return *this;
        }

        bool DoesCollide();
        void ApplyImpulse();
        void PositionalCorrection();

    private:
        friend class PhysicsEngine;
        void CalculateManifold();
        void ManifoldRectangleToRectangle();
        void ManifoldPolygonToPolygon(PrecalculatedPolygon& a, Util::Vec2F posA, PrecalculatedPolygon& b, Util::Vec2F posB);

        void RecalculatePenetration();
        void PenetrationRectangleToRectangle();
        void PenetrationPolygonToPolygon(PrecalculatedPolygon& a, Util::Vec2F posA, PrecalculatedPolygon& b, Util::Vec2F posB);


        Component::Collider* _a;
        Component::Position* _posA;
        Component::Velocity* _velA;
        Component::Collider* _b;
        Component::Position* _posB;
        Component::Velocity* _velB;

        Util::Vec2F _normal;
        float _penetration;
        Util::Vec2F _contacts[2];
        int _contactCount = 0;
        bool _bIsRef = false;// True if b is the reference polygon;
    };

}
}

#endif