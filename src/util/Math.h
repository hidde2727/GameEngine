#ifndef ENGINE_UTIL_MATH_H
#define ENGINE_UTIL_MATH_H

namespace Engine {
namespace Util {

    template<class T>
    inline T min(const T a, const T b) {
        if(a < b) return a;
        else return b;
    }
    template<class T>
    inline T max(const T a, const T b) {
        if(a > b) return a;
        else return b;
    }
    template<class T>
    inline T abs(const T a) {
        if(a < 0) return a*-1;
        else return a;
    }
    template<class T>
    inline T sqr(const T a) {
        return a*a;
    }
    template<class T>
    inline T sign(const T a) {
        if(a > 0) return 1;
        else return -1;
    }
    template<class T>
    inline T clamp(const T a, const T min, const T max) {
        if(a < min) return min;
        if(a > max) return max;
        return a;
    }

}
}

#endif