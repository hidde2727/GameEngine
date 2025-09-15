#ifndef ENGINE_UTIL_BITMASK_H
#define ENGINE_UTIL_BITMASK_H

#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    template<FundamentalType T>
    constexpr T BitMask(const int from, const int till) {
        assert(sizeof(T)<8);
        assert(from>0);
        assert(from<sizeof(T)*8);
        assert(till>0);
        assert(till<sizeof(T)*8);
        assert(from>till);

        return ((((T)0xFFFFFFFFFFFFFFFF)>>till)<<(till+sizeof(T)*8-from))>>(sizeof(T)*8-from);
    }

    template<FundamentalType T>
    T GetBits(const T value, const T from, const T till) {
        return (value & BitMask<T>(from, till)) >> till;
    }

}
}

#endif