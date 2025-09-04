#include "physics/CollisionManifold.h"

#include "util/DebugGraphics.h"

namespace Engine {
namespace Physics {
    
    void CollisionManifold::CalculateManifold() {
        if((this->a->flags & Component::ColliderFlags::Rectangle) && (this->b->flags & Component::ColliderFlags::Rectangle))
            RectangleToRectangle();
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
        }

        Edge incEdge = inc->GetEdgeWithMostOpposedNormal(this->normal);
        ClipResult clip = incEdge.ClipToHalfspace(this->normal.rotatedL(), refA*this->normal);
        clip = clip.ClipToHalfspace(this->normal.rotatedR(), refB*this->normal);

        Util::DrawDebugQuad(refA, 5, Util::Vec3F(1,0,0));
        Util::DrawDebugQuad(refB, 5, Util::Vec3F(1,0,0));
        Util::DrawDebugArrow((refA+refB)*0.5, this->normal, 20.f, 5.f, Util::Vec3F(0, 1, 1));
        Util::DrawDebugQuad(clip.points[0], 5, Util::Vec3F(0,0,1));
        Util::DrawDebugQuad(clip.points[1], 5, Util::Vec3F(0,0,1));

    }

}
}