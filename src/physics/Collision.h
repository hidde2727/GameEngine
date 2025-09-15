#ifndef ENGINE_PHYSICS_COLLISION_H
#define ENGINE_PHYSICS_COLLISION_H

#include "core/PCH.h"
#include "util/Vec2D.h"
#include "core/Components.h"
#include "physics/Components.h"

namespace Engine {
namespace Physics {

    struct CollisionManifold {
        Util::Vec2F normal;
        float penetration;
        Util::Vec2F contacts[2];
        int contactCount;
    };
    CollisionManifold GetCollisionManifold(Component::Collider& a, Component::Position& posA, Component::Collider& b, Component::Position& posB);
    
    CollisionManifold RectangleToRectangle(Component::Collider& a, Component::Position& posA, Component::Collider& b, Component::Position& posB);

    CollisionManifold PolygonToPolygon(PrecalculatedPolygon a, Util::Vec2F posA, PrecalculatedPolygon b, Util::Vec2F posB);

}
}

#endif