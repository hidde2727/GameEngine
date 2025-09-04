#ifndef ENGINE_PHYSICS_COLLISIONMANFIOLD_H
#define ENGINE_PHYSICS_COLLISIONMANFIOLD_H

#include "core/PCH.h"
#include "core/Components.h"
#include "physics/Components.h"

namespace Engine {
namespace Physics {

    class CollisionManifold {
    public:

        CollisionManifold(Component::Collider* a, Component::Position* posA, Component::Collider* b, Component::Position* posB);
        bool DoesCollide();

    private:

        void CalculateManifold();
        void RectangleToRectangle();
        void PolygonToPolygon(PrecalculatedPolygon& a, Util::Vec2F posA, PrecalculatedPolygon& b, Util::Vec2F posB);

        Component::Collider* a;
        Component::Position* posA;
        Component::Collider* b;
        Component::Position* posB;

        Util::Vec2F normal;
        float penetration;
        Util::Vec2F contacts[2];
        int contactCount = 0;
    };

}
}

#endif