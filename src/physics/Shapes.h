#ifndef ENGINE_PHYSICS_SHAPES_RECTANGLE_H
#define ENGINE_PHYSICS_SHAPES_RECTANGLE_H

#include "core/PCH.h"
#include "util/Vec2D.h"
#include "core/Components.h"

namespace Engine {
namespace Physics {

    #ifndef ENGINE_PHYSICS_MAX_POLYGON_SIZE
    #define ENGINE_PHYSICS_MAX_POLYGON_SIZE 8
    #endif


    struct Projection {
        float min = std::numeric_limits<float>::max();
        float max = -std::numeric_limits<float>::max();
    };
    struct ClipResult {
        Util::Vec2F points[2];
        int numPoints;
        
        ClipResult ClipToHalfspace(const Util::Vec2F normal, const float planeOffset) const {
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
        ClipResult DiscardToHalfspace(const Util::Vec2F normal, const float planeOffset) const {
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
    };
    struct Edge {
        Util::Vec2F a;
        Util::Vec2F b;
        Util::Vec2F normal;

        inline ClipResult ClipToHalfspace(const Util::Vec2F normal, const float planeOffset) const {
            ClipResult result{a, b, 2};
            return result.ClipToHalfspace(normal, planeOffset);
        }
    };
    


    struct PrecalculatedPolygon {
        Util::Vec2F points[ENGINE_PHYSICS_MAX_POLYGON_SIZE];
        Util::Vec2F normals[ENGINE_PHYSICS_MAX_POLYGON_SIZE];
        int numPoints = 0;

        Projection GetProjectionOnNormal(const Util::Vec2F normal) const {
            Projection minmax{};
            for(int i = 0; i < numPoints; i++) {
                float projection = points[i].dot(normal);
                if(projection < minmax.min) minmax.min = projection;
                if(projection > minmax.max) minmax.max = projection;
            }
            return minmax;
        }
        Edge GetEdgeWithMostOpposedNormal(const Util::Vec2F normal) const {
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
    };
    struct Polygon {

        PrecalculatedPolygon GetPointsWorldSpace(Component::Position pos) {
            PrecalculatedPolygon pre{};
            return pre;
        }

        Util::Vec2F points[ENGINE_PHYSICS_MAX_POLYGON_SIZE];
        int numPoints = 0;
    };




    struct Rectangle {

        PrecalculatedPolygon GetPointsWorldSpace(Component::Position pos) {
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

        Util::Vec2F size;
    };




    struct Circle {
        float radius;
    };

}
}

#endif