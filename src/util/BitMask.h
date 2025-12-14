#ifndef ENGINE_UTIL_BITMASK_H
#define ENGINE_UTIL_BITMASK_H

#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    /**
     * @brief Gives a bitmask that keeps bits in a range
     * 
     * @tparam T The type to create a bitmask for
     * @param from The first bit to include (starts at 0 until sizeof(T)-1)
     * @param till The first bit to include (starts at 0 until sizeof(T)-1)
     * @return The bitmask
     */
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

    /**
     * @brief Get the value the bits in a certain range represent
     * Automaticaly bitshifts to get the value between [from, till]
     * 
     * @tparam T The type of the input
     * @param value The input value
     * @param from First bit to include in the return
     * @param till Last bit to include in the return
     * @return Value between [from, till] 
     */
    template<FundamentalType T>
    constexpr T GetBits(const T value, const T from, const T till) {
        return (value & BitMask<T>(from, till)) >> till;
    }

}
}

#endif