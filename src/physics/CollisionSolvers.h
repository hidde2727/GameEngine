#ifndef ENGINE_PHYSICS_COLLISIONSOLVERS_H
#define ENGINE_PHYSICS_COLLISIONSOLVERS_H

#include "core/PCH.h"
#include "core/Components.h"
#include "physics/Components.h"

namespace Engine {
namespace Physics {

    bool ResolveCollision(
        PositionComponent& pos1, RectangleColliderComponent& col1, VelocityComponent* vel1, 
        PositionComponent& pos2, RectangleColliderComponent& col2, VelocityComponent* vel2
    );

}
}

#endif