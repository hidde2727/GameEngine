#include "physics/Engine.h"

namespace Engine {
namespace Physics {

    void Engine::Update(entt::registry& registry, float dt) {
		auto group = registry.group<Component::Collider>(entt::get<Component::Position>);
		for (const auto [entity1, collider1, pos1] : group.each()) {
			for (const auto [entity2, collider2, pos2] : group.each()) {
				if(entity1==entity2) continue;
                CollisionManifold manifold(&collider1, &pos1, &collider2, &pos2);
		    }
		}
    }

}
}