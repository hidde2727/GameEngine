#ifndef ENGINE_PHYSICS_ENGINE_H
#define ENGINE_PHYSICS_ENGINE_H

#include "core/PCH.h"
#include "core/Components.h"
#include "physics/Components.h"
#include "physics/CollisionManifold.h"

namespace Engine {
namespace Physics {

    class PhysicsEngine {
    public:

        void Update(entt::registry& registry, float dt);

    private:

    };

}
}

#endif