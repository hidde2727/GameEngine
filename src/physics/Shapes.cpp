#include "physics/Shapes.h"

namespace Engine {
namespace Physics {

    
    ClipResult ClipResult::ClipToHalfspace(const Util::Vec2F normal, const float planeOffset) const {
        ClipResult result{};

        float distA = normal*points[0] - planeOffset;
        float distB = normal*points[1] - planeOffset;

        if(distA <= 0) {
            result.points[result.numPoints] = points[0];
            result.numPoints++;
        }
        if(distB <= 0) {
            result.points[result.numPoints] = points[1];
            result.numPoints++;
        }

        if(distA*distB < 0) {
            float interp = distA / (distA-distB);
            result.points[result.numPoints] = points[0] + (points[1]-points[0])*interp;
            result.numPoints++;
        }

        return result;
    }
    ClipResult ClipResult::DiscardToHalfspace(const Util::Vec2F normal, const float planeOffset) const {
        ClipResult result{};
        for (int i = 0; i < 2; i++) {
            float separation = (normal*points[i]) - planeOffset;
            if (separation <= 0) {
                result.points[result.numPoints] = points[i];
                result.numPoints += 1;
        }
        }
        return result;
    }


    Projection PrecalculatedPolygon::GetProjectionOnNormal(const Util::Vec2F normal) const {
        Projection minmax{};
        for(int i = 0; i < numPoints; i++) {
            float projection = points[i].dot(normal);
            if(projection < minmax.min) minmax.min = projection;
            if(projection > minmax.max) minmax.max = projection;
        }
        return minmax;
    }
    Edge PrecalculatedPolygon::GetEdgeWithMostOpposedNormal(const Util::Vec2F normal) const {
        Edge edge{};
        float minDot = std::numeric_limits<float>::infinity();
        int prevEdge = numPoints-1;
        for(int i = 0; i<4; i++) {
            float dot = normals[i].dot(normal);
            if(dot<minDot) {
                minDot = dot;
                edge.a = points[i];
                edge.b = points[prevEdge];
            }
            prevEdge = i;
        }
        return edge;
    }

    
    PrecalculatedPolygon Polygon::GetPointsWorldSpace(Component::Position pos) const {
        THROW("[Physics::Polygon] GetPointsWorldSpace not implemented yet");
        PrecalculatedPolygon pre{};
        return pre;
    }

    PrecalculatedPolygon Rectangle::GetPointsWorldSpace(Component::Position pos) const {
        PrecalculatedPolygon pre{};
        pre.numPoints = 4;
        auto corners = pos.GetCornerPositions(size);
        std::copy_n(&corners._points[0], 4, &pre.points[0]);
        int prev = 3;
        for(int i = 0; i < 4; i++) {
            pre.normals[i] = (pre.points[i] - pre.points[prev]).normalized().rotatedR();
            prev = i;
        }
        return pre;
    }

}
}