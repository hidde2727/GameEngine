#include "physics/CollisionSolvers.h"

#include "util/DebugGraphics.h"

namespace Engine {
namespace Physics {

    #define SAT_MIN_MAX(axis, point, min, max) {     \
        float projection = (point).dotProduct(axis); \
        if(projection < min) min = projection;       \
        if(projection > max) max = projection;       \
    }
    float GetOverlapOnAxis(const Util::Vec2F& axis, const PositionComponent::Corners& rect1, const PositionComponent::Corners& rect2) {
        float min1=std::numeric_limits<float>::infinity(), max1=-std::numeric_limits<float>::infinity();
        SAT_MIN_MAX(axis, rect1._points[0] , min1, max1);
        SAT_MIN_MAX(axis, rect1._points[1] , min1, max1);
        SAT_MIN_MAX(axis, rect1._points[2] , min1, max1);
        SAT_MIN_MAX(axis, rect1._points[3] , min1, max1);
        float min2=std::numeric_limits<float>::infinity(), max2=-std::numeric_limits<float>::infinity();
        SAT_MIN_MAX(axis, rect2._points[0] , min2, max2);
        SAT_MIN_MAX(axis, rect2._points[1] , min2, max2);
        SAT_MIN_MAX(axis, rect2._points[2] , min2, max2);
        SAT_MIN_MAX(axis, rect2._points[3] , min2, max2);
        if(max1 < min2 || max2 < min1) return 0;
        if(max1-min2 < max2-min1) {
            return max1-min2;
        } else {
            return min1-max2;// (max2-min1)*-1
        }
    }
    struct ClipResult {
        Util::Vec2F _points[2];
    };
    ClipResult ClipToHalfspace(Util::Vec2F p1, Util::Vec2F p2, Util::Vec2F normal, float offset) {
        ClipResult result{};
        float dot1 = normal.dotProduct(p1) - offset;
        float dot2 = normal.dotProduct(p2) - offset;
        int numberOfPoints = 0;
        if(dot1 <= 0) {
            result._points[0] = p1;
            numberOfPoints = 1;
        }
        if(dot2 <= 0) {
            result._points[numberOfPoints] = p2;
            numberOfPoints += 1;
        }
        if(dot1*dot2 < 0) {
            Util::Vec2F nrl = normal.rotatedL();
            result._points[numberOfPoints] = p1 + (p2-p1)*(dot1/(dot1-dot2));
            numberOfPoints += 1;
        }
        return result;
    }
    void ResolveCollision(
        PositionComponent& pos1, RectangleColliderComponent& col1, VelocityComponent* vel1, 
        PositionComponent& pos2, RectangleColliderComponent& col2, VelocityComponent* vel2
    ) {
        if(col1.flags&ColliderFlags::NoMove && col2.flags&ColliderFlags::NoMove) return;

        PositionComponent::Corners rect1 = pos1.GetCornerPositions(col1._size.x, col1._size.y);
        PositionComponent::Corners rect2 = pos2.GetCornerPositions(col2._size.x, col2._size.y);
        Util::Vec2F mtv;
        float minimumOverlap = std::numeric_limits<float>::max();
        bool mtvOf1 = false;
        uint_fast8_t referenceEdgeId = 0;

        {
            Util::Vec2F axis = (rect1._points[0] - rect1._points[3]).normalized();
            float overlap = GetOverlapOnAxis(axis, rect1, rect2);
            if(overlap == 0) return;// No collision
            else { mtv = axis; minimumOverlap = overlap; mtvOf1 = true; referenceEdgeId = overlap>0?0:2; }
        } {
            Util::Vec2F axis = (rect1._points[1] - rect1._points[0]).normalized();
            float overlap = GetOverlapOnAxis(axis, rect1, rect2);
            if(overlap == 0) return;// No collision
            else if(abs(overlap)<abs(minimumOverlap)) { mtv = axis; minimumOverlap = overlap; mtvOf1 = true; referenceEdgeId = overlap>0?1:3;  }
        } {
            Util::Vec2F axis = (rect2._points[0] - rect2._points[3]).normalized();
            float overlap = GetOverlapOnAxis(axis, rect2, rect1);
            if(overlap == 0) return;// No collision
            else if(abs(overlap)<abs(minimumOverlap)) { mtv = axis; minimumOverlap = overlap; mtvOf1 = false; referenceEdgeId = overlap>0?0:2;  }
        } {
            Util::Vec2F axis = (rect2._points[1] - rect2._points[0]).normalized();
            float overlap = GetOverlapOnAxis(axis, rect2, rect1);
            if(overlap == 0) return;// No collision
            else if(abs(overlap)<abs(minimumOverlap)) { mtv = axis; minimumOverlap = overlap; mtvOf1 = false; referenceEdgeId = overlap>0?1:3;  }
        }

        // Resolve the collision
        //pos2._pos += mtv*minimumOverlap*0.5f;
        //pos1._pos -= mtv*minimumOverlap*0.5f;

        if(col1.flags&ColliderFlags::NoVelocityChanges && col2.flags&ColliderFlags::NoVelocityChanges) return;
        if(vel1==nullptr && vel2==nullptr) return;

        mtv *= minimumOverlap>0?-1:1;

        // Search for the incident edge
        // For more information about collision resolution of the second order:
        // https://timallanwheeler.com/blog/2024/08/01/2d-collision-detection-and-resolution/
        Util::Vec2F incidentPA, incidentPB;
        if(mtvOf1) {// Reference edge of 1, incident edge of 2
            float minDot = std::numeric_limits<float>::infinity();
            int prevEdge = 3;
            for(int i = 0; i<4; i++) {
                Util::Vec2F normal = (rect2._points[i]-rect2._points[prevEdge]).normalized().rotatedL();
                float dot = normal.dotProduct(mtv);
                if(dot<minDot) {
                    minDot = dot;
                    incidentPA = rect2._points[i];
                    incidentPB = rect2._points[prevEdge];
                }
                prevEdge = i;
            }
        } else {// Reference edge of 2, incident edge of 1
            float minDot = std::numeric_limits<float>::infinity();
            int prevEdge = 3;
            for(int i = 0; i<4; i++) {
                Util::Vec2F normal = (rect1._points[i]-rect1._points[prevEdge]).normalized().rotatedL();
                float dot = normal.dotProduct(mtv);
                if(dot<minDot) {
                    minDot = dot;
                    incidentPA = rect1._points[i];
                    incidentPB = rect1._points[prevEdge];
                }
                prevEdge = i;
            }
        }

        // Clip the incident edge to get the collision manifold
        ClipResult clip;
        Util::Vec2F refrenceNormal;
        float offset;
        if(mtvOf1) {
            Util::Vec2F normal = (rect1._points[referenceEdgeId] - rect1._points[(referenceEdgeId+1)%4]).normalized();
            clip = ClipToHalfspace(
                incidentPA, 
                incidentPB, 
                normal, 
                rect1._points[referenceEdgeId].dotProduct(normal)
            );
            clip = ClipToHalfspace(
                clip._points[0], 
                clip._points[1], 
                normal.rotated180Degree(), 
                rect1._points[(referenceEdgeId+1)%4].dotProduct(normal.rotated180Degree())
            );
            refrenceNormal = normal.rotatedL();
            offset = refrenceNormal.dotProduct(rect1._points[referenceEdgeId]);
        } else {
            Util::Vec2F normal = (rect2._points[referenceEdgeId] - rect2._points[(referenceEdgeId+1)%4]).normalized();
            clip = ClipToHalfspace(
                incidentPA, 
                incidentPB, 
                normal, 
                rect2._points[referenceEdgeId].dotProduct(normal)
            );
            clip = ClipToHalfspace(
                clip._points[0], 
                clip._points[1], 
                normal.rotated180Degree(), 
                rect2._points[(referenceEdgeId+1)%4].dotProduct(normal.rotated180Degree())
            );
            refrenceNormal = normal.rotatedL();
            offset = refrenceNormal.dotProduct(rect2._points[referenceEdgeId]);
        }

        Util::Vec2F manifold[2];
        int numManifold = 0;
        for (int i = 0; i < 2; i++) {
            float separation = refrenceNormal.dotProduct(clip._points[i]) - offset;
            if (separation <= 0) {
                manifold[numManifold] = clip._points[i];
                numManifold += 1;
            }
        }

        if(mtvOf1) {
            Util::DrawDebugLine(incidentPA, incidentPB, Util::Vec3F(0,0,1));
            Util::DrawDebugLine(rect1._points[referenceEdgeId], rect1._points[(referenceEdgeId+1)%4], Util::Vec3F(0,1,0));
            Util::DrawDebugQuad(manifold[0], 5, Util::Vec3F(1,0,0));
            if(numManifold>1) Util::DrawDebugQuad(manifold[1], 5, Util::Vec3F(1,0,0));
        }
        else {
            Util::DrawDebugLine(incidentPA, incidentPB, Util::Vec3F(0,0,1));
            Util::DrawDebugLine(rect2._points[referenceEdgeId], rect2._points[(referenceEdgeId+1)%4], Util::Vec3F(0,1,0));
            Util::DrawDebugQuad(manifold[0], 5, Util::Vec3F(1,0,0));
            if(numManifold>1) Util::DrawDebugQuad(manifold[1], 5, Util::Vec3F(1,0,0));  
        }
    }

}
}