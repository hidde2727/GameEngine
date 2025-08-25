#include "physics/CollisionSolvers.h"

#include "util/DebugGraphics.h"

namespace Engine {
namespace Physics {

    #define SAT_MIN_MAX(axis, point, min, max) {     \
        float projection = (point).dot(axis); \
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
        if(max1 <= min2 || max2 <= min1) return 0;
        return max1-min2;
    }
    struct ClipResult {
        Util::Vec2F _points[2];
    };
    ClipResult ClipToHalfspace(Util::Vec2F p1, Util::Vec2F p2, Util::Vec2F normal, float offset) {
        ClipResult result{};
        float dot1 = normal.dot(p1) - offset;
        float dot2 = normal.dot(p2) - offset;
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
            Util::Vec2F nrl = normal.rotatedR();
            result._points[numberOfPoints] = p1 + (p2-p1)*(dot1/(dot1-dot2));
            numberOfPoints += 1;
        }
        return result;
    }
    struct CollisionResponse {
        Util::Vec2F vel1;
        Util::Vec2F vel2;
        float rot1;
        float rot2;
    };
    CollisionResponse GetCollisionResponse(
        Util::Vec2F manifold, Util::Vec2F normal, 
        Util::Vec2F cent1, RectangleColliderComponent& col1, VelocityComponent* velc1, 
        Util::Vec2F cent2, RectangleColliderComponent& col2, VelocityComponent* velc2
    ) {
        VelocityComponent vel1 = velc1==nullptr?VelocityComponent(0):*velc1;
        VelocityComponent vel2 = velc2==nullptr?VelocityComponent(0):*velc2;
        Util::Vec2F r1 = manifold - cent1;
        Util::Vec2F r2 = manifold - cent2;
        Util::Vec2F relativeV = (vel2.velocity+Util::Cross(vel2.angular, r2)) - 
                                (vel1.velocity+Util::Cross(vel1.angular, r1));
        float restitution = std::max(col1.restitution,col2.restitution);
        float impulse = (relativeV.dot(normal)*(-(1.0f+restitution)))
        /(
            1/col1.mass + 1/col2.mass + 
            Util::sqr(r1.cross(normal))/(col1.momentOfIntertia) + 
            Util::sqr(r2.cross(normal))/(col2.momentOfIntertia)
        );
        if(relativeV*normal>0) return CollisionResponse{vel1.velocity, vel2.velocity, vel1.angular, vel2.angular};// Don't apply anything, they are already moving apart
        return CollisionResponse {
            vel1.velocity - (normal*(impulse/col1.mass)),
            vel2.velocity + (normal*(impulse/col2.mass)),
            vel1.angular - (impulse/col1.momentOfIntertia)*r1.cross(normal),
            vel2.angular + (impulse/col2.momentOfIntertia)*r2.cross(normal)
        };
    }
    bool ResolveCollision(
        PositionComponent& pos1, RectangleColliderComponent& col1, VelocityComponent* vel1, 
        PositionComponent& pos2, RectangleColliderComponent& col2, VelocityComponent* vel2
    ) {
        if(col1.flags&ColliderFlags::NoMove && col2.flags&ColliderFlags::NoMove) return false;

        PositionComponent::Corners rect1 = pos1.GetCornerPositions(col1._size.x, col1._size.y);
        PositionComponent::Corners rect2 = pos2.GetCornerPositions(col2._size.x, col2._size.y);
        Util::Vec2F mtv;
        float minimumOverlap = std::numeric_limits<float>::max();
        bool mtvOf1 = false;
        uint_fast8_t referenceEdgeId = 0;

        int previousEdge = 3;
        for(int i = 0; i < 4; i++) {
            Util::Vec2F axis = (rect1._points[i] - rect1._points[previousEdge]).normalized().rotatedR();
            float overlap = GetOverlapOnAxis(axis, rect1, rect2);
            if(overlap == 0) return false;// No collision
            else if(abs(overlap)<abs(minimumOverlap)) { 
                mtv = axis; 
                minimumOverlap = overlap; 
                mtvOf1 = true; 
                referenceEdgeId = i; 
            }
            previousEdge = i;
        }
        previousEdge = 3;
        for(int i = 0; i < 4; i++) {
            Util::Vec2F axis = (rect2._points[i] - rect2._points[previousEdge]).normalized().rotatedR();
            float overlap = GetOverlapOnAxis(axis, rect2, rect1);
            if(overlap == 0) return false;// No collision
            else if(abs(overlap)<abs(minimumOverlap)) { 
                mtv = axis; 
                minimumOverlap = overlap; 
                mtvOf1 = false; 
                referenceEdgeId = i; 
            }
            previousEdge = i;
        }

        // Resolve the collision
        if(mtvOf1) {
            if(col1.flags&ColliderFlags::NoMove) pos2._pos += mtv*minimumOverlap;
            else if(col2.flags&ColliderFlags::NoMove) pos1._pos -= mtv*minimumOverlap;
            else {
                pos1._pos -= mtv*minimumOverlap*0.5f;//*(col2.mass/(col1.mass+col2.mass));
                pos2._pos += mtv*minimumOverlap*0.5f;//*(col1.mass/(col1.mass+col2.mass));
            }
        } else {
            if(col1.flags&ColliderFlags::NoMove) pos2._pos -= mtv*minimumOverlap;
            else if(col2.flags&ColliderFlags::NoMove) pos1._pos += mtv*minimumOverlap;
            else {
                pos1._pos += mtv*minimumOverlap*0.5f;//*(col2.mass/(col1.mass+col2.mass));
                pos2._pos -= mtv*minimumOverlap*0.5f;//*(col1.mass/(col1.mass+col2.mass));
            }
        }

        if(col1.flags&ColliderFlags::NoVelocityChanges && col2.flags&ColliderFlags::NoVelocityChanges) return true;
        if(vel1==nullptr && vel2==nullptr) return true;

        // Search for the incident edge
        // For more information about collision resolution of the second order:
        // https://timallanwheeler.com/blog/2024/08/01/2d-collision-detection-and-resolution/
        Util::Vec2F incidentPA, incidentPB;
        if(mtvOf1) {// Reference edge of 1, incident edge of 2
            float minDot = std::numeric_limits<float>::infinity();
            int prevEdge = 3;
            for(int i = 0; i<4; i++) {
                Util::Vec2F normal = (rect2._points[i]-rect2._points[prevEdge]).normalized().rotatedR();
                float dot = normal.dot(mtv);
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
                Util::Vec2F normal = (rect1._points[i]-rect1._points[prevEdge]).normalized().rotatedR();
                float dot = normal.dot(mtv);
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
            Util::Vec2F normal = (rect1._points[referenceEdgeId] - rect1._points[(referenceEdgeId+3)%4]).normalized();
            clip = ClipToHalfspace(
                incidentPA, 
                incidentPB, 
                normal, 
                rect1._points[referenceEdgeId].dot(normal)
            );
            clip = ClipToHalfspace(
                clip._points[0], 
                clip._points[1], 
                normal.rotated180Degree(), 
                rect1._points[(referenceEdgeId+3)%4].dot(normal.rotated180Degree())
            );
            refrenceNormal = normal.rotatedR();
            offset = refrenceNormal.dot(rect1._points[referenceEdgeId]);
        } else {
            Util::Vec2F normal = (rect2._points[referenceEdgeId] - rect2._points[(referenceEdgeId+3)%4]).normalized();
            clip = ClipToHalfspace(
                incidentPA, 
                incidentPB, 
                normal, 
                rect2._points[referenceEdgeId].dot(normal)
            );
            clip = ClipToHalfspace(
                clip._points[0], 
                clip._points[1], 
                normal.rotated180Degree(), 
                rect2._points[(referenceEdgeId+3)%4].dot(normal.rotated180Degree())
            );
            refrenceNormal = normal.rotatedR();
            offset = refrenceNormal.dot(rect2._points[referenceEdgeId]);
        }

        Util::Vec2F manifold[2];
        int numManifold = 0;
        for (int i = 0; i < 2; i++) {
            float separation = refrenceNormal.dot(clip._points[i]) - offset;
            if (separation <= 0) {
                manifold[numManifold] = clip._points[i];
                numManifold += 1;
            }
        }

        if(numManifold == 1) {
            Util::Vec2F cent1 = rect1._points[0]*0.5f + rect1._points[2]*0.5f;
            Util::Vec2F cent2 = rect2._points[0]*0.5f + rect2._points[2]*0.5f;
            CollisionResponse response = GetCollisionResponse(
                manifold[0], refrenceNormal, 
                cent1, col1, vel1,
                cent2, col2, vel2
            );
            if(!(col1.flags&ColliderFlags::NoMove) && !(col1.flags&ColliderFlags::NoVelocityChanges) && vel1!=nullptr) {
                vel1->velocity = response.vel1;
                vel1->angular  = response.rot1;
            }
            if(!(col2.flags&ColliderFlags::NoMove) && !(col2.flags&ColliderFlags::NoVelocityChanges) && vel2!=nullptr) {
                vel2->velocity = response.vel2;
                vel2->angular  = response.rot2;
            }
        } else if(numManifold == 2) {
            Util::Vec2F cent1 = rect1._points[0]*0.5f + rect1._points[2]*0.5f;
            Util::Vec2F cent2 = rect2._points[0]*0.5f + rect2._points[2]*0.5f;
            float depthOffset = (mtvOf1?rect1._points[referenceEdgeId]:rect2._points[referenceEdgeId]).dot(refrenceNormal);
            CollisionResponse response1 = GetCollisionResponse(
                manifold[0], refrenceNormal, 
                cent1, col1, vel1,
                cent2, col2, vel2
            );
            float depth1 = manifold[0].dot(refrenceNormal) - depthOffset;
            CollisionResponse response2 = GetCollisionResponse(
                manifold[1], refrenceNormal, 
                cent1, col1, vel1,
                cent2, col2, vel2
            );
            float depth2 = manifold[1].dot(refrenceNormal) - depthOffset;
            float fraction1 = depth1/(depth1+depth2);
            float fraction2 = depth2/(depth1+depth2);
            if(!(col1.flags&ColliderFlags::NoMove) && !(col1.flags&ColliderFlags::NoVelocityChanges) && vel1!=nullptr) {
                vel1->velocity = response1.vel1*fraction1+response2.vel1*fraction2;
                vel1->angular  = response1.rot1*fraction1+response2.rot1*fraction2;
            }
            if(!(col2.flags&ColliderFlags::NoMove) && !(col2.flags&ColliderFlags::NoVelocityChanges) && vel2!=nullptr) {
                vel2->velocity = response1.vel2*fraction1+response2.vel2*fraction2;
                vel2->angular  = response1.rot2*fraction1+response2.rot2*fraction2;
            }
        } else {
            Util::Error("WTF, why is there no collision manifold");
        }if(mtvOf1) {
            Util::DrawDebugLine(incidentPA, incidentPB, Util::Vec3F(0,0,1));
            Util::DrawDebugLine(rect1._points[referenceEdgeId], rect1._points[(referenceEdgeId+3)%4], Util::Vec3F(0,1,0));
            Util::DrawDebugQuad(manifold[0], 5, Util::Vec3F(1,0,0));
            if(numManifold>1) Util::DrawDebugQuad(manifold[1], 5, Util::Vec3F(1,0,0));
            Util::DrawDebugArrow(rect1._points[0]*0.5 + rect1._points[2]*0.5, refrenceNormal, 50, 10.f, Util::Vec3F(0,0,1));
        }
        else {
            Util::DrawDebugLine(incidentPA, incidentPB, Util::Vec3F(0,0,1));
            Util::DrawDebugLine(rect2._points[referenceEdgeId], rect2._points[(referenceEdgeId+3)%4], Util::Vec3F(0,1,0));
            Util::DrawDebugQuad(manifold[0], 5, Util::Vec3F(1,0,0));
            if(numManifold>1) Util::DrawDebugQuad(manifold[1], 5, Util::Vec3F(1,0,0));  
            Util::DrawDebugArrow(rect2._points[0]*0.5 + rect2._points[2]*0.5, refrenceNormal, 50, 10.f, Util::Vec3F(0,0,1));
        }

        return true;
    }

}
}