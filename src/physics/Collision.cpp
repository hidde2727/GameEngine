#include "physics/CollisionManifold.h"

#include "util/DebugGraphics.h"

namespace Engine {
namespace Physics {
    
    void CollisionManifold::CalculateManifold() {
        if((this->a->flags & Component::ColliderFlags::NoMove) && (this->b->flags & Component::ColliderFlags::NoMove)) return;

        if((this->a->flags & Component::ColliderFlags::Rectangle) && (this->b->flags & Component::ColliderFlags::Rectangle))
            RectangleToRectangle();
        
        if(bIsRef) {
            Component::Collider* a = this->a;
            Component::Position* posA = this->posA;
            Component::Velocity* velA = this->velA;
            this->a = this->b;
            this->posA = this->posB;
            this->velA = this->velB;
            this->b = a;
            this->posB = posA;
            this->velB = velA;
        }
    }
    
    void CollisionManifold::RectangleToRectangle() {
        PrecalculatedPolygon polA = a->shape.rectangle.GetPointsWorldSpace(*posA);
        PrecalculatedPolygon polB = b->shape.rectangle.GetPointsWorldSpace(*posB);
        PolygonToPolygon(
            polA, posA->_pos, 
            polB, posB->_pos
        );
    }

    void CollisionManifold::PolygonToPolygon(PrecalculatedPolygon& a, Util::Vec2F posA, PrecalculatedPolygon& b, Util::Vec2F posB) {
        this->penetration = std::numeric_limits<float>::max();
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
            if(abs(this->penetration) > abs(penetration)) {
                this->penetration = penetration;
                this->normal = normal;
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
            if(abs(this->penetration) > abs(penetration)) {
                this->penetration = penetration;
                this->normal = normal;
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
            bIsRef = true;
        }

        Edge incEdge = inc->GetEdgeWithMostOpposedNormal(this->normal);
        // ClipResult clip = incEdge.ClipToHalfspace((refA-refB).normalized(), refA*(refA-refB).normalized());
        ClipResult clip = incEdge.ClipToHalfspace(this->normal.rotatedL(), refA*this->normal.rotatedL());
        // clip = clip.ClipToHalfspace((refB-refA).normalized(), refB*(refB-refA).normalized());
        clip = clip.ClipToHalfspace(this->normal.rotatedR(), refB*this->normal.rotatedR());

        ASSERT(clip.numPoints==2, "[Physics::CollisionManifold] Clipping did not result in 2 points");
        clip = clip.DiscardToHalfspace(this->normal, refA*this->normal);
        ASSERT(clip.numPoints >= 1, "[Physics::CollisionManifold] Clipping did not result in at least 1 point");

        this->contactCount = clip.numPoints;
        this->contacts[0] = clip.points[0];
        this->contacts[1] = clip.points[1];
    }

}
}