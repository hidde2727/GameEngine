#include "physics/CollisionManifold.h"

namespace Engine {
namespace Physics {

    CollisionManifold::CollisionManifold(
        Component::Collider* a, Component::Position* posA, Component::Velocity* velA, 
        Component::Collider* b, Component::Position* posB, Component::Velocity* velB
    ) : a(a), posA(posA), velA(velA), b(b), posB(posB), velB(velB) {
        ASSERT(a!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with collider a == nullptr");
        ASSERT(b!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with collider b == nullptr");
        ASSERT(posA!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with position a == nullptr");
        ASSERT(posB!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with position b == nullptr");
        CalculateManifold();
    }

    // CalculateManifold implemented in Collision.cpp

    bool CollisionManifold::DoesCollide() {
        return contactCount!=0;
    }

    void CollisionManifold::ApplyImpulse() {
        bool aStatic = a->IsStatic() || velA == nullptr;
        bool bStatic = b->IsStatic() || velB == nullptr;
        // Static colliders do not move
        if(aStatic && bStatic) return;
        bool aKinematic = a->IsKinematic() || aStatic;
        bool bKinematic = b->IsKinematic() || bStatic;
        // Static and kinematic colliders do not affect each other
        if(aKinematic && bKinematic) return;
        if(contactCount == 0) return;

        Component::Velocity nullV{};
        // Velocity components that are not nullptr
        Component::Velocity* velAnnul = velA == nullptr ? &nullV : velA;
        Component::Velocity* velBnnul = velB == nullptr ? &nullV : velB;

        float sf = std::sqrt(a->sf * b->sf);
        float df = std::sqrt(a->df * b->df);

        for(int i = 0; i < contactCount; i++) {
            Util::Vec2F ra = contacts[i] - posA->_pos;
            Util::Vec2F rb = contacts[i] - posB->_pos;

            Util::Vec2F vRel = velBnnul->v + Cross(velBnnul->w, rb) -
                               velAnnul->v - Cross(velAnnul->w, ra);

            if(vRel.dot(normal) > 0) return;

            float e = Util::min(a->e, b->e);
            float denominator = a->im + b->im + 
                Util::sqr(ra.cross(normal)) * a->iL + 
                Util::sqr(rb.cross(normal)) * b->iL;
            float j = -(1 + e) * vRel.dot(normal);
            j /= denominator;
            j /= (float)contactCount;

            velAnnul->v -= normal * (j * a->im);
            velAnnul->w -= Cross(ra, normal) * j * a->iL;
            velBnnul->v += normal * (j * b->im);
            velBnnul->w += Cross(rb, normal) * j * b->iL;

            // Friction
            vRel = velBnnul->v + Cross(velBnnul->w, rb) -
                   velAnnul->v - Cross(velAnnul->w, ra);
            Util::Vec2F t = vRel - (normal * vRel.dot(normal));
            t = t.normalized();

            float jt = -vRel.dot(t);
            jt /= denominator;
            jt /= (float)contactCount;

            if(std::abs(jt) < 0.0001f) return;

            if(std::abs(jt) >= j * sf) {
                // Dynamic friction
                jt = -j * df;
            }
            velAnnul->v -= t * (jt * a->im);
            velAnnul->w -= Cross(ra, t) * jt * a->iL;
            velBnnul->v += t * (jt * b->im);
            velBnnul->w += Cross(rb, t) * jt * b->iL;
        }
    }

    void CollisionManifold::PositionalCorrection() {
        posA->_pos += normal * penetration * (a->im / (a->im + b->im));
        posB->_pos -= normal * penetration * (b->im / (a->im + b->im));
    }

}
}