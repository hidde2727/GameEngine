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
        
        ClipResult ClipToHalfspace(const Util::Vec2F normal, const float planeOffset) const;
        ClipResult DiscardToHalfspace(const Util::Vec2F normal, const float planeOffset) const;
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

        Projection GetProjectionOnNormal(const Util::Vec2F normal) const;
        Edge GetEdgeWithMostOpposedNormal(const Util::Vec2F normal) const;
    };
    struct Polygon {

        PrecalculatedPolygon GetPointsWorldSpace(Component::Position pos) const;

        Util::Vec2F points[ENGINE_PHYSICS_MAX_POLYGON_SIZE];
        int numPoints = 0;
    };




    struct Rectangle {

        PrecalculatedPolygon GetPointsWorldSpace(Component::Position pos) const;

        Util::Vec2F size;
    };




    struct Circle {
        float radius;
    };

}
}

#endif