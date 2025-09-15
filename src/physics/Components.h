#ifndef ENGINE_PHYSICS_COMPONENTS_H
#define ENGINE_PHYSICS_COMPONENTS_H

#include "core/PCH.h"
#include "physics/Shapes.h"

namespace Engine {
namespace Component {

    struct Acceleration {
        Util::Vec2F a;// acceleration;
        float alpha;// angular acceleration
    };

    struct Velocity {
        Util::Vec2F v;// velocity
        float w;// angular velocity
    };

    enum ColliderFlags {
        Continuos=1,
        NoMove=2,
        NoVelocityChanges=4,

        Polygon=256,
        Rectangle=512,
        Circle=1024
    };

    struct Collider {
        Collider() {}
        Collider(const Collider& col) : shape(), flags(col.flags), e(col.e), im(col.im), iL(col.iL) {
            if(col.flags&ColliderFlags::Polygon) shape.polygon=col.shape.polygon;
            if(col.flags&ColliderFlags::Rectangle) shape.rectangle=col.shape.rectangle;
            if(col.flags&ColliderFlags::Circle) shape.circle=col.shape.circle;
        }
        Collider& operator=(const Collider& c) {
            flags = c.flags; 
            e = c.e;
            im = c.im; 
            iL = c.iL;
            if(c.flags&ColliderFlags::Polygon) shape.polygon=c.shape.polygon;
            if(c.flags&ColliderFlags::Rectangle) shape.rectangle=c.shape.rectangle;
            if(c.flags&ColliderFlags::Circle) shape.circle=c.shape.circle;
            return *this;
        }

        struct Empty {};
        union Shape {
            Physics::Polygon polygon;
            Physics::Rectangle rectangle;
            Physics::Circle circle;
            Empty empty;
            Shape() : empty() {}
        } shape;
        uint16_t flags;
        float e = 1;// restitution
        float im = 1;// inverse mass
        float iL = 1;// inverse inertia

        static Collider StaticRect(const Util::Vec2F size) {
            Collider col;
            col.shape.rectangle.size = size;
            col.flags = ColliderFlags::Rectangle;
            col.im = 0;
            col.iL = 0;
            return col;
        }
        static Collider StaticRect(const Util::Vec2F size, const uint16_t flags) {
            Collider col;
            col.shape.rectangle.size = size;
            col.flags = ColliderFlags::Rectangle | flags;
            col.im = 0;
            col.iL = 0;
            return col;
        }
        static Collider KinematicRect(const Util::Vec2F size) {
            Collider col;
            col.shape.rectangle.size = size;
            col.flags = ColliderFlags::Rectangle;
            return col;
        }
        static Collider DynamicRect(const Util::Vec2F size) {
            Collider col;
            col.shape.rectangle.size = size;
            col.flags = ColliderFlags::Rectangle;
            return col;
        }
    };

}
}

#endif