#ifndef ENGINE_PHYSICS_ENGINE_H
#define ENGINE_PHYSICS_ENGINE_H

#include "core/PCH.h"
#include "core/Components.h"
#include "physics/Components.h"
#include "physics/CollisionManifold.h"
#include <tuple>

namespace Engine {
namespace Physics {

    // This class does not use the ECS in a normal way
    // Every request to add/retrieve/remove a collider should not be done yourself
    //      use this class to do it for you
    // This class only stores an uuid to the collider on the entity and keeps the collider
    //      in a vector, retrieving a Component::Collider will return nothing if done directly through the ECS
    class PhysicsEngine {
    public:

        void Update(entt::registry& registry, float dt);
        
        // All these functions are to make it easier to store only the uuid of the collider in the ECS
        // We store only the UUID so we can organize the colliders any way we want
        void AddCollider(entt::registry& registry, const entt::entity entity, const Component::Collider collider);
        void SetCollider(entt::registry& registry, const entt::entity entity, const Component::Collider collider);
        bool HasCollider(entt::registry& registry, const entt::entity entity);
        Component::Collider& GetCollider(entt::registry& registry, const entt::entity entity);
        void RemoveCollider(entt::registry& registry, const entt::entity entity);

    private:
        struct StaticBody {
            StaticBody() {}
            StaticBody(const Component::Position& pos, const Component::Collider& col) : pos(pos), col(col) {}
            Component::Position pos;
            Component::Collider col;  
        };
        std::map<uint64_t, StaticBody> _staticBodies;
        uint64_t _nextStaticUUID = 0;
        struct MovingBody {
            MovingBody() {}
            MovingBody(const Component::Collider& col, const entt::entity entity) 
                : col(col), entity(entity) {}
            Component::Collider col;
            entt::entity entity;
        };
        std::map<uint64_t, MovingBody> _movingBodies;
        uint64_t _nextMovingUUID = 0;
    };

}
}

#endif