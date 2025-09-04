#include "physics/CollisionManifold.h"

namespace Engine {
namespace Physics {

    CollisionManifold::CollisionManifold(Component::Collider* a, Component::Position* posA, Component::Collider* b, Component::Position* posB)
    : a(a), posA(posA), b(b), posB(posB) {
        CalculateManifold();
    }

    bool CollisionManifold::DoesCollide() {
        return contactCount!=0;
    }

}
}