#ifndef ENGINE_UTIL_SERIALIZATION_PRINT_H
#define ENGINE_UTIL_SERIALIZATION_PRINT_H

#include "core/PCH.h"
#include "util/serialization/Serialization.h"

namespace Engine {
namespace Util {

    // Waiting until it is possible to use variable names (c++26 reflection)
    class ClassPrinter {
        

        virtual inline void StartClass(const size_t amountMembers) {

        }
        virtual inline void EndClass() {

        }

        virtual inline void StartPair() {

        }
        virtual inline void EndPair() {

        }

        virtual inline void StartArray(const size_t amountElements) {

        }
        virtual inline void EndArray() {

        }

        virtual inline void AddVariable(const int8_t v) {

        }
        virtual inline void AddVariable(const int16_t v) { 
            
        }
        virtual inline void AddVariable(const int32_t v) { 
            
        }
        virtual inline void AddVariable(const int64_t v) { 
            
        }

        virtual inline void AddVariable(const uint8_t v) { 
            
        }
        virtual inline void AddVariable(const uint16_t v) { 

        }
        virtual inline void AddVariable(const uint32_t v) { 
            
        }
        virtual inline void AddVariable(const uint64_t v) { 
            
        }

        virtual inline void AddVariable(const float v) { 
            
        }
        virtual inline void AddVariable(const double v) { 
            
        }
    };

}
}

#endif