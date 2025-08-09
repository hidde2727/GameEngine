#include "physics/Engine.h"

namespace Engine {
namespace Physics {

    void Engine::Update(entt::registry& registry) {
        // Velocity
        auto group = registry.group<VelocityComponent>(entt::get<PositionComponent>);
		for (const auto [entity, velocity, pos] : group.each()) {
            pos._pos += velocity.velocity;
            pos._rotation += velocity.angular;
		}

        // Collisions
        auto group2 = registry.group<RectangleColliderComponent>(entt::get<PositionComponent>);
        for(const auto [entity, collider, pos] : group2.each()) {
            auto velocity = registry.try_get<VelocityComponent>(entity);
            for(const auto [entity2, collider2, pos2] : group2.each()) {
                if(entity == entity2) continue;
                auto velocity2 = registry.try_get<VelocityComponent>(entity2);
                ResolveCollision(pos, collider, velocity, pos2, collider2, velocity2);
            }
        }
    }

}
}