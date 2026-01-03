#ifndef ENGINE_UTIL_MATH_H
#define ENGINE_UTIL_MATH_H

#include "util/math/Matrix.h"
#include "util/math/Vec2D.h"
#include "util/math/Vec3D.h"
#include "util/math/Area.h"
#include "util/math/EquationSolvers.h"
#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    template<FloatingPointType T>
    inline constexpr int floor(const T a) {
        return (int)a;
    }
    template<FloatingPointType T>
    inline constexpr int ceil(const T a) {
        return (int)(a+1);
    }
    template<FundamentalType T>
    inline constexpr T min(const T a, const T b) {
        if(a < b) return a;
        else return b;
    }
    template<FundamentalType T>
    inline constexpr T max(const T a, const T b) {
        if(a > b) return a;
        else return b;
    }
    template<FundamentalType T>
    inline constexpr T abs(const T a) {
        if(a < 0) return a*-1;
        else return a;
    }
    template<FundamentalType T>
    inline Util::Vec2<T> abs(const Util::Vec2<T> a) {
        return Util::Vec2<T>(abs(a.x), abs(a.y));
    }
    template<FundamentalType T>
    inline Util::Vec3<T> abs(const Util::Vec3<T> a) {
        return Util::Vec3<T>(abs(a.x), abs(a.y), abs(a.z));
    }
    template<FundamentalType T>
    inline constexpr T sqr(const T a) {
        return a*a;
    }
    template<FundamentalType T>
    inline constexpr T sign(const T a) {
        if(a > 0) return 1;
        else return -1;
    }
    template<FundamentalType T>
    inline constexpr T clamp(const T a, const T min, const T max) {
        if(a < min) return min;
        if(a > max) return max;
        return a;
    }

}
}

#endif