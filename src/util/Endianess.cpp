#include "util/Endianess.h"
#include <core/PCH.h>

namespace Engine {
namespace Util {
    
    bool isSystemBigEndian = ([]() -> bool {
        if constexpr (std::endian::native == std::endian::big) {
            LOG("[Endianess.cpp] System is big-endian")
            return true;
        }
        else if constexpr (std::endian::native == std::endian::little) {
            LOG("[Endianess.cpp] System is litle-endian")
            return false;
        }
        else {
            WARNING("[Endianess.cpp] System is mixed-endian")
            uint16_t* ptr16 = new uint16_t(1);
            uint8_t* ptr8 = reinterpret_cast<uint8_t*>(ptr16);
            if(*ptr8 == 0) return false;
            else if(*ptr8 == 1) return true;
            THROW("[Endianess.cpp] I don't know what just happened")
        }
    })();

}
}