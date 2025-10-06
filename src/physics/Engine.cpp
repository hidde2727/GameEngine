#include "physics/Engine.h"

namespace Engine {
namespace Physics {

    void PhysicsEngine::Update(entt::registry& registry, float dt) {
		std::vector<CollisionManifold> manifolds;
		{
		auto group = registry.group<Component::Collider>(entt::get<Component::Position>);
		for (int i = 0; i < group.size(); i++) {
			for (int j = i; j < group.size(); j++) {
				entt::entity entity1 = group[i];
				entt::entity entity2 = group[j];
				if(i == j) continue;

				auto [collider1, pos1] = group.get<Component::Collider, Component::Position>(entity1);
				auto [collider2, pos2] = group.get<Component::Collider, Component::Position>(entity2);

				auto vel1 = registry.try_get<Component::Velocity>(entity1);
				auto vel2 = registry.try_get<Component::Velocity>(entity2);

				ASSERT(!collider1.IsStatic() || vel1 == nullptr, "[Physics::Engine] Static collider cannot have velocity component");
				ASSERT(!collider2.IsStatic() || vel2 == nullptr, "[Physics::Engine] Static collider cannot have velocity component");

				CollisionManifold manifold(&collider1, &pos1, vel1, &collider2, &pos2, vel2);
				if(manifold.DoesCollide())
                	manifolds.push_back(manifold);
		    }
		}
		}

		for(auto& manifold : manifolds) {
			manifold.ApplyImpulse();
		}
		
		{
		auto group = registry.group<Component::Velocity>(entt::get<Component::Position>);
		for (const auto [entity, vel, pos] : group.each()) {
			pos._pos += vel.v * dt;
			pos._rotation += vel.w * dt;
		}
		}

		for(auto& manifold : manifolds) {
			manifold.PositionalCorrection();
		}
    }

}
}