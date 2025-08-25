#include "physics/Engine.h"

namespace Engine {
namespace Physics {

    void Engine::Update(entt::registry& registry, float dt) {
        // Acceleration
        auto group = registry.group<AccelerationComponent>(entt::get<VelocityComponent>);
		for (const auto [entity, acceleration, velocity] : group.each()) {
            velocity.velocity += acceleration.acceleration * dt;
            velocity.angular  += acceleration.angular * dt;
		}
        // Velocity
        auto group2 = registry.group<VelocityComponent>(entt::get<PositionComponent>);
		for (const auto [entity, velocity, pos] : group2.each()) {
            float length = velocity.velocity.length();
            float newLength = Util::clamp(length - length*velocity.velocityDamping.x*dt - length*length*velocity.velocityDamping.y*dt, 0.f, length);
            if(length!=0) velocity.velocity *= (newLength/length);
            
            velocity.angular = Util::clamp(velocity.angular - velocity.angular*velocity.angularDamping.x*dt - velocity.angular*velocity.angular*velocity.angularDamping.y*dt, 0.f, velocity.angular);

            pos._pos += velocity.velocity*dt;
            pos._rotation += velocity.angular*dt;
		}
        // Rotation to velocity
        auto group4 = registry.group<RotationToVelocity>(entt::get<VelocityComponent, PositionComponent>);
        for(const auto [entity, vel, pos] : group4.each()) {
            float newRot = vel.velocity.angleToX();
            vel.angular = (newRot - pos._rotation) * (1/dt);
            pos._rotation = newRot;
        }

        // Collisions
        auto group3 = registry.group<RectangleColliderComponent>(entt::get<PositionComponent>);
        for(const auto [entity, collider, pos] : group3.each()) {
            auto velocity = registry.try_get<VelocityComponent>(entity);
            for(const auto [entity2, collider2, pos2] : group3.each()) {
                if(entity == entity2) continue;
                auto velocity2 = registry.try_get<VelocityComponent>(entity2);
                bool col = ResolveCollision(pos, collider, velocity, pos2, collider2, velocity2);
            }
        }
    }

}
}