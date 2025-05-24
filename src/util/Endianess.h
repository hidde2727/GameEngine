#ifndef ENGINE_UTILS_ENDIANESS_H
#define ENGINE_UTILS_ENDIANESS_H

#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    extern bool isSystemBigEndian;

    // The input is in litle endian and is transformed to the systems endianess
    template<IntegralType T>
    T FromLitleEndian(T t) {
        if(!isSystemBigEndian) return t;

        T ret = t;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&t);
        uint8_t* retPtr = reinterpret_cast<uint8_t*>(&ret);
        for(int i = 0; i < sizeof(T); i++) {
            retPtr[sizeof(T)-i-1] = ptr[i];
        }
        return ret;
    }
    // The input is in big endian and is transformed to the systems endianess
    template<IntegralType T>
    T FromBigEndian(T t) {
        if(isSystemBigEndian) return t;

        T ret = t;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&t);
        uint8_t* retPtr = reinterpret_cast<uint8_t*>(&ret);
        for(int i = 0; i < sizeof(T); i++) {
            retPtr[sizeof(T)-i-1] = ptr[i];
        }
        return ret;
    }
    // The input is in the systems endianess and is transformed to litle endian
    template<IntegralType T>
    T ToLitleEndian(T t) {
        if(!isSystemBigEndian) return t;

        T ret = t;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&t);
        uint8_t* retPtr = reinterpret_cast<uint8_t*>(&ret);
        for(int i = 0; i < sizeof(T); i++) {
            retPtr[sizeof(T)-i-1] = ptr[i];
        }
        return ret;
    }
    // The input is in the systems endianess and is transformed to big endian
    template<IntegralType T>
    T ToBigEndian(T t) {
        if(isSystemBigEndian) return t;

        T ret = t;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&t);
        uint8_t* retPtr = reinterpret_cast<uint8_t*>(&ret);
        for(int i = 0; i < sizeof(T); i++) {
            retPtr[sizeof(T)-i-1] = ptr[i];
        }
        return ret;
    }

    // The input is in the systems endianess
    template<IntegralType T>
    uint8_t* GetLeastSignificantByte(const T* t) {
        if(isSystemBigEndian) return reinterpret_cast<uint8_t*>(t);
        return reinterpret_cast<uint8_t*>(t)+sizeof(T)-1;
    }

    // The input is in the systems endianess
    template<IntegralType T>
    uint8_t* GetMostSignificantByte(const T* t) {
        if(isSystemBigEndian) return reinterpret_cast<uint8_t*>(t)+sizeof(T)-1;
        return reinterpret_cast<uint8_t*>(t);
    }

}
}

#endif