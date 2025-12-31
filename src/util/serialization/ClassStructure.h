#ifndef ENGINE_UTIL_SERIALIZATION_CLASSSTRUCTURE_H
#define ENGINE_UTIL_SERIALIZATION_CLASSSTRUCTURE_H

#include "core/PCH.h"
#include "util/VectorStreamBuffer.h"
#include "util/serialization/Serialization.h"
#include "util/Endianess.h"

namespace Engine {
namespace Util {

    #ifndef ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION
    #define ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION 2
    #endif

    /**
     * @brief Used to communicate to another language that is loosely typed how a c++ class looks
     * Used my the networking module to send a class structure over a websocket
     * 
     * @warning This class cannot magically access private members, if you want private members serialized, you need to add ```friend class Util::ClassStructureSerializer```
     */
    class ClassStructureSerializer {
    public:

        template<class T, class Q> requires(sizeof(Q) == 1)
        void Serialize(std::vector<Q>& vector, uint8_t flags = 0) {
            VectorStreamBuffer<Q, char> buffer(vector);
            std::ostream tempStream(&buffer);
            Serialize<T>(tempStream, flags);
        }
        template<class T>
        void Serialize(std::basic_ostream<char>& stream, uint8_t flags = 0) {
            _outputStream = &stream;
            _flags = flags;
            ASSERT(!(IsFlagSet(BinarySerializationOutputFlag::OutputBigEndian) && IsFlagSet(BinarySerializationOutputFlag::OutputLitleEndian)), "[Util::ClassStructureSerializer] Cannot output both big and litle endian");
            if(!IsFlagSet(BinarySerializationOutputFlag::OutputBigEndian) && !IsFlagSet(BinarySerializationOutputFlag::OutputLitleEndian)) {
                if(isSystemBigEndian) flags |= OutputBigEndian;
                else flags |= OutputLitleEndian;
            }
            Output<uint8_t>(flags);// flags
            if(!IsFlagSet(BinarySerializationOutputFlag::ExcludeVersioning))
                Output<uint8_t>(ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION);// version
            SerializeInternal<T>(PrettyNameOf<T>());
        }

        template<class T> requires (!SpecialSerialization<T>)
        void SerializeInternal(std::string_view name) {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            constexpr size_t amountMembers = memberTypes.size();
            if(amountMembers == 0) WARNING("[Util::ClassStructureSerializer] Found a class (" + PrettyNameOf<T>() + ") with 0 member variables, are you sure it is intented this class has only private members? (maybe you forgot to add 'friend class Util::ClassStructureSerializer')")

            Output<uint8_t>(SerializationTypes::Class);
            Output(name);
            Output<uint8_t>(amountMembers);
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if(HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                SerializeInternal<typename [: std::meta::type_of(memberTypeInfo) :]>(std::meta::identifier_of(memberTypeInfo));
            }
        }
        template<FundamentalType T>
        void SerializeInternal(std::string_view name) { THROW("[Util::ClassStructureSerializer] !This should never be called!")}
        template<>
        void SerializeInternal<uint8_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::UInt8);
            Output(name);
        }
        template<>
        void SerializeInternal<uint16_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::UInt16);
            Output(name);
        }
        template<>
        void SerializeInternal<uint32_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::UInt32);
            Output(name);
        }
        template<>
        void SerializeInternal<uint64_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::UInt64);
            Output(name);
        }
        template<>
        void SerializeInternal<int8_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Int8);
            Output(name);
        }
        template<>
        void SerializeInternal<int16_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Int16);
            Output(name);
        }
        template<>
        void SerializeInternal<int32_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Int32);
            Output(name);
        }
        template<>
        void SerializeInternal<int64_t>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Int64);
            Output(name);
        }
        template<>
        void SerializeInternal<float>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Float);
            Output(name);
        }
        template<>
        void SerializeInternal<double>(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Double);
            Output(name);
        }
        template<StdStringType T>
        void SerializeInternal(std::string_view name) {
            Output<uint8_t>(SerializationTypes::String);
            Output(name);
        }
        template<StdMapType T>
        void SerializeInternal(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Map);
            Output(name);
            constexpr auto templateArguments = std::define_static_array(std::meta::template_arguments_of(^^T));
            SerializeInternal<typename [: templateArguments[0] :] >("Key-type");   // Key   of the map
            SerializeInternal<typename [: templateArguments[1] :] >("Value-type"); // Value of the map
        }
        template<StdVectorType T>
        void SerializeInternal(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Array);
            Output(name);
            constexpr auto templateArguments = std::define_static_array(std::meta::template_arguments_of(^^T));
            SerializeInternal<typename [: templateArguments[0] :] >("Value-type"); // Value in the vector
        }
        template<PointerType T>
        void SerializeInternal(std::string_view name) {
            SerializeInternal<std::pointer_traits<T>::element_type>(name);
        }
        template<StdTimePoint T>
        void SerializeInternal(std::string_view name) {
            SerializeInternal<uint64_t>(name);
        }
        template<class T> requires Derived<T, Serializable>
        void SerializeInternal(std::string_view name) {
            T::SerializeTypes(this, name);
        }
        template<StdSharedPtr T>
        void SerializeInternal(std::string_view name) {
            constexpr auto templateArguments = std::define_static_array(std::meta::template_arguments_of(^^T));
            SerializeInternal<typename [: templateArguments[0] :] >(name); // Value in the vector
        }
        template<ArrayType T>
        void SerializeInternal(std::string_view name) {
            Output<uint8_t>(SerializationTypes::Array);
            Output(name);
            SerializeInternal<typename [: std::meta::remove_extent(^^T) :]>("Value-type");
        }

        inline void Output(const std::string_view str) {
            ASSERT(str.size() < UINT16_MAX, "[Util::ClassStructureSerializer] Cannot output a string bigger than UINT16_MAX")
            Output<uint16_t>(str.size());
            for(const char c : str) {
                Output<char>(c);
            }
        }
        template<IntegralType T>
        inline void Output(T t) {
            if(IsFlagSet(BinarySerializationOutputFlag::OutputBigEndian)) t = Util::ToBigEndian(t);
            if(IsFlagSet(BinarySerializationOutputFlag::OutputLitleEndian)) t = Util::ToLitleEndian(t);

            _outputStream->write(reinterpret_cast<char*>(&t), sizeof(T));
            ASSERT(!_outputStream->fail() && !_outputStream->bad(), "[Util::ClassStructureSerializer] Output buffer is not big enough")
        }
        inline bool IsFlagSet(const BinarySerializationOutputFlag flag) const {
            return _flags & flag;
        }
    private:

        std::basic_ostream<char>* _outputStream;
        uint8_t _flags = 0;
    };

}
}

#endif