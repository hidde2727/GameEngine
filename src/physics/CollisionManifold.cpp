#include "physics/CollisionManifold.h"

#include "util/DebugGraphics.h"

namespace Engine {
namespace Physics {

    CollisionManifold::CollisionManifold(
        Component::Collider* a, Component::Position* posA, Component::Velocity* velA, 
        Component::Collider* b, Component::Position* posB, Component::Velocity* velB
    ) : _a(a), _posA(posA), _velA(velA), _b(b), _posB(posB), _velB(velB) {
        ASSERT(a!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with collider a == nullptr");
        ASSERT(b!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with collider b == nullptr");
        ASSERT(posA!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with position a == nullptr");
        ASSERT(posB!=nullptr, "[Physics::CollisionManifold] Cannot constructor a manifold with position b == nullptr");
        CalculateManifold();
    }
    CollisionManifold::~CollisionManifold() {
        // Flag used by the PhysicsEngine to indicate the pointer needs to be deleted
        //      because the pointer is temporary
        if(_a && (_a->flags & Component::ColliderFlags::Internal)) {
            delete _a;
            delete _posA;
        }
        _a = nullptr;
        _posA = nullptr;
        _velA = nullptr;
        // Flag used by the PhysicsEngine to indicate the pointer needs to be deleted
        //      because the pointer is temporary
        if(_b && (_b->flags & Component::ColliderFlags::Internal)) {
            delete _b;
            delete _posB;
        }
        _b = nullptr;
        _posB = nullptr;
        _velB = nullptr;
    }

    bool CollisionManifold::DoesCollide() {
        return _contactCount!=0;
    }
    // ----------------------------------------------------------------------
    // ------------------------ Collision Resolution ------------------------
    // ----------------------------------------------------------------------

    // Source for Physics: https://timallanwheeler.com/blog/2024/08/01/2d-collision-detection-and-resolution/
    void CollisionManifold::ApplyImpulse() {
        bool aStatic = _a->IsStatic() || _velA == nullptr;
        bool bStatic = _b->IsStatic() || _velB == nullptr;
        // Static colliders do not move
        if(aStatic && bStatic) return;
        bool aKinematic = _a->IsKinematic() || aStatic;
        bool bKinematic = _b->IsKinematic() || bStatic;
        // Static and kinematic colliders do not affect each other
        if(aKinematic && bKinematic) return;
        if(_contactCount == 0) return;

        Component::Velocity nullV{};
        // Velocity components that are not nullptr
        Component::Velocity* velAnnul = _velA == nullptr ? &nullV : _velA;
        Component::Velocity* velBnnul = _velB == nullptr ? &nullV : _velB;

        float sf = std::sqrt(_a->sf * _b->sf);
        float df = std::sqrt(_a->df * _b->df);

        // resA and resB are used to take an average of the new velocities of both contactPoints
        for(int i = 0; i < _contactCount; i++) {
            Component::Velocity velCopyA = *velAnnul;
            Component::Velocity velCopyB = *velBnnul;

            Util::Vec2F ra = _contacts[i] - _posA->_pos;
            Util::Vec2F rb = _contacts[i] - _posB->_pos;

            Util::Vec2F vRel = velCopyB.v + Cross(velCopyB.w, rb) -
                               velCopyA.v - Cross(velCopyA.w, ra);

            if(vRel.dot(_normal) > 0) return;

            float e = Util::min(_a->e, _b->e);
            float denominator = _a->im + _b->im + 
                Util::sqr(ra.cross(_normal)) * _a->iL + 
                Util::sqr(rb.cross(_normal)) * _b->iL;
            float j = -(1 + e) * vRel.dot(_normal);
            j /= denominator;

            velCopyA.v -= _normal * (j * _a->im);
            velCopyA.w -= Cross(ra, _normal) * j * _a->iL;
            velCopyB.v += _normal * (j * _b->im);
            velCopyB.w += Cross(rb, _normal) * j * _b->iL;

            // Friction
            vRel = velCopyB.v + Cross(velCopyB.w, rb) -
                   velCopyA.v - Cross(velCopyA.w, ra);
            Util::Vec2F t = vRel - (_normal * vRel.dot(_normal));
            t = t.normalized();

            float jt = -vRel.dot(t);
            jt /= denominator;

            if(std::abs(jt) < 0.0001f) return;

            if(std::abs(jt) >= j * sf) {
                // Dynamic friction (Cap the amount of friction)
                jt = -j * df;
            }
            velCopyA.v -= t * (jt * _a->im);
            velCopyA.w -= Cross(ra, t) * jt * _a->iL;
            velCopyB.v += t * (jt * _b->im);
            velCopyB.w += Cross(rb, t) * jt * _b->iL;

            float importance = _penetration/_contactCount;
            velAnnul->newV += velCopyA.v * importance;
            velAnnul->newW += velCopyA.w * importance;
            velAnnul->divisionFactor += importance;
            velBnnul->newV += velCopyB.v * importance;
            velBnnul->newW += velCopyB.w * importance;
            velBnnul->divisionFactor += importance;
        }
    }

    void CollisionManifold::PositionalCorrection() {
        RecalculatePenetration();
        if(_penetration >= 0) return;
        _posA->_pos += _normal * _penetration * (_a->im / (_a->im + _b->im));
        _posB->_pos -= _normal * _penetration * (_b->im / (_a->im + _b->im));
    }


        
    // ----------------------------------------------------------------------
    // ------------------------- Collision Manifold -------------------------
    // ----------------------------------------------------------------------

    void CollisionManifold::CalculateManifold() {
        if((this->_a->flags & Component::ColliderFlags::NoMove) && (this->_b->flags & Component::ColliderFlags::NoMove)) return;

        if((this->_a->flags & Component::ColliderFlags::Rectangle) && (this->_b->flags & Component::ColliderFlags::Rectangle))
            ManifoldRectangleToRectangle();
        else
            THROW("[CollisionManifold] CalculateManifold not implemented for the collider shapes")
        
        if(_bIsRef) {
            // Swap A and B, making sure A is the reference polygon
            Component::Collider* a = this->_a;
            Component::Position* posA = this->_posA;
            Component::Velocity* velA = this->_velA;
            this->_a = this->_b;
            this->_posA = this->_posB;
            this->_velA = this->_velB;
            this->_b = a;
            this->_posB = posA;
            this->_velB = velA;
        }
    }
    
    void CollisionManifold::ManifoldRectangleToRectangle() {
        PrecalculatedPolygon polA = _a->shape.rectangle.GetPointsWorldSpace(*_posA);
        PrecalculatedPolygon polB = _b->shape.rectangle.GetPointsWorldSpace(*_posB);
        ManifoldPolygonToPolygon(
            polA, _posA->_pos, 
            polB, _posB->_pos
        );
    }

    void CollisionManifold::ManifoldPolygonToPolygon(PrecalculatedPolygon& a, Util::Vec2F posA, PrecalculatedPolygon& b, Util::Vec2F posB) {
        this->_penetration = std::numeric_limits<float>::max();
        bool flip = false;
        Util::Vec2F refA;
        Util::Vec2F refB;

        int prev = a.numPoints-1;
        for(int i = 0; i < a.numPoints; i++) {
            Util::Vec2F normal = a.normals[i];
            Projection projA = a.GetProjectionOnNormal(normal);
            Projection projB = b.GetProjectionOnNormal(normal);
            if(projA.max <= projB.min || projB.max <= projA.min) return;
            float penetration = Util::min(
                projB.max - projA.min,
                projB.min - projA.max);
            if(abs(this->_penetration) > abs(penetration)) {
                this->_penetration = penetration;
                this->_normal = normal;
                refA = a.points[i];
                refB = a.points[prev];
            }
            prev=i;
        }

        prev=b.numPoints-1;
        for(int i = 0; i < b.numPoints; i++) {
            Util::Vec2F normal = b.normals[i];
            Projection projA = b.GetProjectionOnNormal(normal);
            Projection projB = a.GetProjectionOnNormal(normal);
            if(projA.max <= projB.min || projB.max <= projA.min) return;
            float penetration = Util::min(
                projB.max - projA.min,
                projB.min - projA.max);
            if(abs(this->_penetration) > abs(penetration)) {
                this->_penetration = penetration;
                this->_normal = normal;
                flip = true;
                refA = b.points[i];
                refB = b.points[prev];
            }
            prev=i;
        }

        PrecalculatedPolygon* ref = &a;
        PrecalculatedPolygon* inc = &b;
        if(flip) {
            ref = &b;
            inc = &a;
            _bIsRef = true;
        }

        Edge incEdge = inc->GetEdgeWithMostOpposedNormal(this->_normal);
        // ClipResult clip = incEdge.ClipToHalfspace((refA-refB).normalized(), refA*(refA-refB).normalized());
        ClipResult clip = incEdge.ClipToHalfspace(this->_normal.rotatedL(), refA*this->_normal.rotatedL());
        // clip = clip.ClipToHalfspace((refB-refA).normalized(), refB*(refB-refA).normalized());
        clip = clip.ClipToHalfspace(this->_normal.rotatedR(), refB*this->_normal.rotatedR());

        ASSERT(clip.numPoints==2, "[Physics::CollisionManifold] Clipping did not result in 2 points");
        clip = clip.DiscardToHalfspace(this->_normal, refA*this->_normal);
        ASSERT(clip.numPoints >= 1, "[Physics::CollisionManifold] Clipping did not result in at least 1 point");

        this->_contactCount = clip.numPoints;
        this->_contacts[0] = clip.points[0];
        this->_contacts[1] = clip.points[1];
    }

    
    // ----------------------------------------------------------------------
    // ----------------------- Recalculate penetration ----------------------
    // ----------------------------------------------------------------------
    void CollisionManifold::RecalculatePenetration() {
        if((this->_a->flags & Component::ColliderFlags::Rectangle) && (this->_b->flags & Component::ColliderFlags::Rectangle))
            PenetrationRectangleToRectangle();
        else
            THROW("[CollisionManifold] RecalculatePenetration not implemented for the collider shapes")
    }
    void CollisionManifold::PenetrationRectangleToRectangle() {
        PrecalculatedPolygon polA = _a->shape.rectangle.GetPointsWorldSpace(*_posA);
        PrecalculatedPolygon polB = _b->shape.rectangle.GetPointsWorldSpace(*_posB);
        PenetrationPolygonToPolygon(
            polA, _posA->_pos, 
            polB, _posB->_pos
        );
    }
    void CollisionManifold::PenetrationPolygonToPolygon(PrecalculatedPolygon& a, Util::Vec2F posA, PrecalculatedPolygon& b, Util::Vec2F posB) {
        Projection projA = a.GetProjectionOnNormal(_normal);
        Projection projB = b.GetProjectionOnNormal(_normal);
        _penetration = Util::min(
                projB.max - projA.min,
                projB.min - projA.max);
    }
}
}