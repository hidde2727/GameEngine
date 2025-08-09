#ifndef ENGINE_PHYSICS_ENGINE_H
#define ENGINE_PHYSICS_ENGINE_H

#include "core/PCH.h"
#include "core/Components.h"
#include "physics/Components.h"
#include "physics/CollisionSolvers.h"

namespace Engine {
namespace Physics {

    class Engine {
    public:

        void Update(entt::registry& registry);

    private:

    };

}
}

#endif