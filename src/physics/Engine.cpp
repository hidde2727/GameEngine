#include "physics/Engine.h"

namespace Engine {
namespace Physics {

    void PhysicsEngine::Update(entt::registry& registry, float dt) {
		std::vector<CollisionManifold> manifolds;

		int i = 0;
		for(auto& [uuid, body] : _movingBodies) {
			Component::Position& pos1 = registry.get<Component::Position>(body.entity);
			Component::Velocity* vel1 = registry.try_get<Component::Velocity>(body.entity);
			for(auto& [uuid2, body2] : _staticBodies) {
				Component::Velocity* vel = registry.try_get<Component::Velocity>(body.entity);
				CollisionManifold manifold(
					&body.col, &pos1, vel,
					&body2.col, &body2.pos, nullptr);
				if(manifold.DoesCollide())
                	manifolds.push_back(manifold);
			}
			int j = 0;
			for(auto& [uuid2, body2] : _movingBodies) {
				j++;
				if(j <= i) continue;
				if(body.entity == body2.entity) continue;
				Component::Velocity* vel2 = registry.try_get<Component::Velocity>(body2.entity);
				Component::Position& pos2 = registry.get<Component::Position>(body2.entity);
				CollisionManifold manifold(
					&body.col, &pos1, vel1,
					&body2.col, &pos2, vel2);
				if(manifold.DoesCollide())
                	manifolds.push_back(manifold);
			}
			i++;
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

	// Static UUID has the first bit set to 0
	#define NEW_STATIC_UUID (uint64_t)0x7FFFFFFFFFFFFFFF & _nextStaticUUID
	#define IS_STATIC(uuid) !(((uint64_t)1<<63) & uuid)
	// Moving UUID has the first bit set to 1
	#define NEW_MOVING_UUID ((uint64_t)1<<63) | _nextMovingUUID
	#define IS_MOVING(uuid) ((uint64_t)1<<63) & uuid
	void PhysicsEngine::AddCollider(entt::registry& registry, const entt::entity entity, const Component::Collider collider) {
		if(collider.IsStatic()) {
			ASSERT(registry.all_of<Component::Position>(entity), "[PhysicsEngine::AddCollider] Cannot add a static collider to an entity without a position")
			_staticBodies[NEW_STATIC_UUID] = StaticBody(registry.get<Component::Position>(entity), collider);
			registry.emplace<Component::ColliderUUID>(entity, NEW_STATIC_UUID);
			_nextStaticUUID++;
		} else {
			_movingBodies[NEW_MOVING_UUID] = MovingBody(collider, entity);
			registry.emplace<Component::ColliderUUID>(entity, NEW_MOVING_UUID);
			_nextMovingUUID++;
		}
	}
	void PhysicsEngine::SetCollider(entt::registry& registry, const entt::entity entity, const Component::Collider collider) {
		uint64_t uuid = registry.get<Component::ColliderUUID>(entity).uuid;
		if(IS_STATIC(uuid)) {
			ASSERT(collider.IsStatic(), "[PhysicsEngine::SetCollider] Cannot set a static collider = dynamic collider")
			_staticBodies[uuid].col = collider;
		} else {
			ASSERT(!collider.IsStatic(), "[PhysicsEngine::SetCollider] Cannot set a dynamic collider = static collider")
			_movingBodies[uuid].col = collider;
		}
	}
	bool PhysicsEngine::HasCollider(entt::registry& registry, const entt::entity entity) {
		return registry.all_of<Component::ColliderUUID>(entity);
	}
	Component::Collider& PhysicsEngine::GetCollider(entt::registry& registry, const entt::entity entity) {
		uint64_t uuid = registry.get<Component::ColliderUUID>(entity).uuid;
		if(IS_STATIC(uuid)) {
			return _staticBodies[uuid].col;
		} else {
			return _movingBodies[uuid].col;
		}
	}
	void PhysicsEngine::RemoveCollider(entt::registry& registry, const entt::entity entity) {
		uint64_t uuid = registry.get<Component::ColliderUUID>(entity).uuid;
		registry.remove<Component::ColliderUUID>(entity);
		if(IS_STATIC(uuid)) {
			_staticBodies.erase(uuid);
		} else {
			_movingBodies.erase(uuid);
		}
	}

}
}