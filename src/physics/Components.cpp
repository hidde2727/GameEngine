#include "physics/Components.h"

namespace Engine {
namespace Component {

    
    Collider::Collider(const Collider& col) : shape(), flags(col.flags), e(col.e), im(col.im), iL(col.iL), sf(col.sf), df(col.df) {
        if(col.flags&ColliderFlags::Polygon) shape.polygon=col.shape.polygon;
        if(col.flags&ColliderFlags::Rectangle) shape.rectangle=col.shape.rectangle;
        if(col.flags&ColliderFlags::Circle) shape.circle=col.shape.circle;
    }
    Collider& Collider::operator=(const Collider& c) {
        flags = c.flags; 
        e = c.e;
        im = c.im; 
        iL = c.iL;
        sf = sf;
        df = df;
        if(c.flags&ColliderFlags::Polygon) shape.polygon=c.shape.polygon;
        if(c.flags&ColliderFlags::Rectangle) shape.rectangle=c.shape.rectangle;
        if(c.flags&ColliderFlags::Circle) shape.circle=c.shape.circle;
        return *this;
    }

    
    bool Collider::IsStatic() const {
        return im == 0 || (flags & ColliderFlags::NoMove) || (flags & ColliderFlags::NoVelocityChanges);
    }
    bool Collider::IsKinematic() const {
        return flags & ColliderFlags::Kinematic;
    }

    
    void Collider::RecalculateMass(const float density) {
        if(flags & ColliderFlags::NoMove) {
            im = 0;
            iL = 0;
            return;
        }
        if(flags & ColliderFlags::Rectangle) {
            im = 1.0f / (density * shape.rectangle.size.x * shape.rectangle.size.y);
            iL = im * 12.0f / (Util::sqr(shape.rectangle.size.x) + Util::sqr(shape.rectangle.size.y));
        } else {
            THROW("[Physics::Collider] RecalculateMass() only implemented for rectangles");
        }
    }

    Collider Collider::StaticRect(const Util::Vec2F size, const PhysicsMaterial mat) {
        Collider col;
        col.shape.rectangle.size = size;
        col.flags = ColliderFlags::Rectangle | ColliderFlags::NoMove | ColliderFlags::NoVelocityChanges;
        col.e = mat.e;
        col.sf = mat.sf;
        col.df = mat.df;
        col.RecalculateMass(1.0f);
        return col;
    }
    Collider Collider::StaticRect(const Util::Vec2F size, const uint16_t flags, const PhysicsMaterial mat) {
        Collider col;
        col.shape.rectangle.size = size;
        col.flags = ColliderFlags::Rectangle | ColliderFlags::NoMove | ColliderFlags::NoVelocityChanges | flags;
        col.e = mat.e;
        col.sf = mat.sf;
        col.df = mat.df;
        col.RecalculateMass(1.0f);
        return col;
    }
    Collider Collider::KinematicRect(const Util::Vec2F size, const PhysicsMaterial mat) {
        Collider col;
        col.shape.rectangle.size = size;
        col.flags = ColliderFlags::Rectangle | ColliderFlags::Kinematic;
        col.e = mat.e;
        col.sf = mat.sf;
        col.df = mat.df;
        col.RecalculateMass(1.0f);
        return col;
    }
    Collider Collider::DynamicRect(const Util::Vec2F size, const PhysicsMaterial mat) {
        Collider col;
        col.shape.rectangle.size = size;
        col.flags = ColliderFlags::Rectangle;
        col.e = mat.e;
        col.sf = mat.sf;
        col.df = mat.df;
        col.RecalculateMass(1.0f);
        return col;
    }

}
}