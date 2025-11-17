#include "physics/Engine.h"

// UUIDS:

// Static UUID has the first bit set to 0
#define NEW_STATIC_UUID(baseUUID) (uint64_t)0x7FFFFFFFFFFFFFFF & baseUUID
#define IS_STATIC(uuid) !(((uint64_t)1<<63) & uuid)
// Moving UUID has the first bit set to 1
#define NEW_MOVING_UUID ((uint64_t)1<<63) | _nextMovingID
#define IS_MOVING(uuid) ((uint64_t)1<<63) & uuid


namespace Engine {
namespace Physics {

    void PhysicsEngine::Update(entt::registry& registry, float dt) {
		std::vector<CollisionManifold> manifolds;

		int i = 0;
		for(auto& [uuid, body] : _movingBodies) {
			Component::Position& pos1 = registry.get<Component::Position>(body.entity);
			Component::Velocity* vel1 = registry.try_get<Component::Velocity>(body.entity);
			AABB aabb1 = body.GetAABB(pos1);
			for(auto& body2 : _staticBodies.Query(aabb1)) {
				// Lifetime of the body2 ends after this forloop
				// Create a copy of the collider+position and indicate with a flag CollisionManifold needs
				// 		to delete the pointers
				Component::Collider* body2Ptr = new Component::Collider(body2.col);
				body2Ptr->flags != Component::ColliderFlags::Internal;
				Component::Position* pos2Ptr = new Component::Position(body2.pos);

				CollisionManifold manifold(
					&body.col, &pos1, vel1,
					body2Ptr, pos2Ptr, nullptr);
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
			if(vel.divisionFactor != 0) {
				vel.v = vel.newV / vel.divisionFactor;
				vel.w = vel.newW / vel.divisionFactor;

				vel.divisionFactor = 0;
				vel.newV = Util::Vec2F(0);
				vel.newW = 0;
			}
			Component::ColliderUUID* col = registry.try_get<Component::ColliderUUID>(entity);
			if(col && IS_MOVING(col->uuid)) {
				vel.v += _gravity * _movingBodies[col->uuid].col.gravityFactor * dt;
			}

			pos._pos += vel.v * dt;
			pos._rotation += vel.w * dt;
		}
		}

		for(auto& manifold : manifolds) {
			manifold.PositionalCorrection();
		}
    }
	
    void PhysicsEngine::SetGravity(const Util::Vec2F gravity) {
		_gravity = gravity;
	}

	void PhysicsEngine::AddCollider(entt::registry& registry, const entt::entity entity, const Component::Collider collider) {
		if(collider.IsStatic()) {
			ASSERT(registry.all_of<Component::Position>(entity), "[PhysicsEngine::AddCollider] Cannot add a static collider to an entity without a position")
			StaticBody body = StaticBody(registry.get<Component::Position>(entity), collider);
			uint32_t uuid = _staticBodies.Insert(body, body.GetAABB());
			registry.emplace<Component::ColliderUUID>(entity, NEW_STATIC_UUID(uuid));
		} else {
			_movingBodies[NEW_MOVING_UUID] = MovingBody(collider, entity);
			registry.emplace<Component::ColliderUUID>(entity, NEW_MOVING_UUID);
			_nextMovingID++;
		}
	}
	void PhysicsEngine::SetCollider(entt::registry& registry, const entt::entity entity, const Component::Collider collider) {
		uint64_t uuid = registry.get<Component::ColliderUUID>(entity).uuid;
		if(IS_STATIC(uuid)) {
			ASSERT(collider.IsStatic(), "[PhysicsEngine::SetCollider] Cannot set a static collider = dynamic collider")
			StaticBody body = StaticBody(registry.get<Component::Position>(entity), collider);
			_staticBodies.Set(uuid, body, body.GetAABB());
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
			return _staticBodies.Get(uuid).col;
		} else {
			return _movingBodies[uuid].col;
		}
	}
    void PhysicsEngine::UpdateCollider(entt::registry& registry, const entt::entity entity) {
		uint64_t uuid = registry.get<Component::ColliderUUID>(entity).uuid;
		if(IS_STATIC(uuid)) {
			SetCollider(registry, entity, _staticBodies.Get(uuid).col);
		}
		// Dynamic objects do not need to be updated
	}
	void PhysicsEngine::RemoveCollider(entt::registry& registry, const entt::entity entity) {
		uint64_t uuid = registry.get<Component::ColliderUUID>(entity).uuid;
		registry.remove<Component::ColliderUUID>(entity);
		if(IS_STATIC(uuid)) {
			_staticBodies.Remove(uuid);
		} else {
			_movingBodies.erase(uuid);
		}
	}

	
	void PhysicsEngine::AddImageCollider(entt::registry& registry, const entt::entity entity, const Component::ImageBasedCollider collider) {
		ASSERT(registry.all_of<Component::Position>(entity), "[PhysicsEngine::AddImageCollider] Cannot add an image collider on an entity without a position")
		registry.emplace<Component::ImageBasedColliderID>(entity, _nextImageColliderID);
		_nextImageColliderID++;
		SetImageCollider(registry, entity, collider);
	}
	void PhysicsEngine::SetImageCollider(entt::registry& registry, const entt::entity entity, const Component::ImageBasedCollider collider) {
		ASSERT(registry.all_of<Component::Position>(entity), "[PhysicsEngine::SetImageCollider] Cannot set an image collider on an entity without a position")
		Component::ImageBasedColliderID& id = registry.get<Component::ImageBasedColliderID>(entity);
		Component::Position& pos = registry.get<Component::Position>(entity);
		// Remove the old colliders
		if(
			id.firstStaticBody != std::numeric_limits<uint64_t>::infinity() && 
			id.lastStaticBody != std::numeric_limits<uint64_t>::infinity()
		) {
			RemoveImageCollider(registry, entity);
			registry.emplace<Component::ImageBasedColliderID>(entity, id);
		}

		_imageBasedColliders[id.id] = collider;

		// Convert the active pixels to static colliders
		_staticBodies.SetContinuousIDs(true);

		id.firstStaticBody = _staticBodies.GetNextChildID();
        int width, height, channels;
		std::shared_ptr<uint8_t> imageDataPtr = Util::FileManager::ReadImageFile(collider._file, &width, &height, &channels, 1);
		uint8_t* imageData = imageDataPtr.get();
		float scaleX = collider._forcedSize == 0 ? 1.f : collider._forcedSize.x / (float)width;
		float scaleY = collider._forcedSize == 0 ? 1.f : collider._forcedSize.y / (float)height;
		float offsetX = pos._pos.x - (width/2.f)*scaleX;
		float offsetY = pos._pos.y - (height/2.f)*scaleY;

        for(int y = 0; y < height; y++) {
			int xStart = 0;
            for(int x = 0; x < width; x++) {
                uint8_t* pixelLocation = imageData + y*width + x;
				// Try to batch pixel that are on next to each other togheter into one collider
				if(*pixelLocation != collider._colliderColor) {
					if(x-xStart == 0) { xStart = x+1; continue; }
					
					StaticBody body = StaticBody(
						Component::Position((xStart+x)*0.5f*scaleX+offsetX, (y+0.5f)*scaleY+offsetY),
						Component::Collider::StaticRect(Util::Vec2F((x-xStart)*scaleX,scaleY), collider._material)
					);
 					_staticBodies.Insert(body, body.GetAABB());
					xStart = x+1;
				}
            }
			if(xStart != width) {
				StaticBody body = StaticBody(
					Component::Position((xStart+width)*0.5f*scaleX+offsetX, (y+0.5f)*scaleY+offsetY),
					Component::Collider::StaticRect(Util::Vec2F((width-xStart)*scaleX,scaleY),  collider._material)
				);
				_staticBodies.Insert(body, body.GetAABB());
			}
        }
		id.lastStaticBody = _staticBodies.GetNextChildID()-1;

		_staticBodies.SetContinuousIDs(false);
	}
	bool PhysicsEngine::HasImageCollider(entt::registry& registry, const entt::entity entity) {
		return registry.all_of<Component::ImageBasedCollider>(entity);
	}
	Component::ImageBasedCollider& PhysicsEngine::GetImageCollider(entt::registry& registry, const entt::entity entity) {
		Component::ImageBasedColliderID id = registry.get<Component::ImageBasedColliderID>(entity);
		return _imageBasedColliders[id.id];
	}
	void PhysicsEngine::UpdateImageCollider(entt::registry& registry, const entt::entity entity) {
		THROW("Why whould you update an image collider, that is extremely inefficient. Please just create a new scene if you want a new collider. TODO implement this function")
	}
	void PhysicsEngine::RemoveImageCollider(entt::registry& registry, const entt::entity entity) {
		Component::ImageBasedColliderID id = registry.get<Component::ImageBasedColliderID>(entity);
		for(uint64_t i = id.firstStaticBody; i < id.lastStaticBody; i++) {
			_staticBodies.Remove(i);
		}
		registry.remove<Component::ImageBasedColliderID>(entity);
	}

}
}