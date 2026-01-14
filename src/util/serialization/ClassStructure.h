#ifndef ENGINE_UTIL_SERIALIZATION_CLASSSTRUCTURE_H
#define ENGINE_UTIL_SERIALIZATION_CLASSSTRUCTURE_H

#include "core/PCH.h"
#include "util/VectorStreamBuffer.h"
#include "util/Endianess.h"
#include "util/serialization/Serialization.h"

namespace Engine {
namespace Util {

    #ifndef ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION
    #define ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION 3
    #endif

    /**
     * @brief The base for class structure (de)serializing
     * 
     */
    class ClassStructureBase {
    public:
        enum OutputFlag : uint8_t {
            Empty                = 1, // Here for parity with OutputFlag
            OutputBigEndian      = 2, // Default is system endianess
            OutputLitleEndian    = 4, // Default is system endianess
            ExcludeVersioning    = 8, // Defaults to including the versioning data
            ExcludeVariableNames = 16,// Defaults to include variable names (is only output if IncludeTypeInfo=true)
            CheckVariableNames   = 32 // Defaults to not check the variable names (is only checked if IncludeTypeInfo=true)
        };

        enum SerializationType : uint8_t {
            Class                   = 0,

            Uint8                   = 1,
            Uint16                  = 2,
            Uint32                  = 3,
            Uint64                  = 4,
            Uint128                 = 5,
            Int8                    = 6,
            Int16                   = 7,
            Int32                   = 8,
            Int64                   = 9,
            Int128                  = 10,
            Bool                    = 11,

            Float                   = 15,
            Double                  = 16,
            LongDouble              = 17,

            String                  = 20,

        // STL Types:
            Array                   = 30,
            Vector                  = 31,
            Deque                   = 32,
            ForwardList             = 33,
            List                    = 34,
            Set                     = 35,
            Multiset                = 36,
            Map                     = 37,
            Multimap                = 38,
            UnorderedSet            = 39,
            UnorderedMultiset       = 40,
            UnorderedMap            = 41,
            UnorderedMultimap       = 42,
            Stack                   = 43,
            Queue                   = 44,
            PriorityQueue           = 45,
            FlatSet                 = 46,
            FlatMultiset            = 47,
            FlatMap                 = 48,
            FlatMultimap            = 49,

        // STD types:
            TimePoint               = 60,
            Duration                = 61
        };

        enum ClockTypes : uint8_t {
            System              = 0,
            Steady              = 1,
            HighResolution      = 2,
            UTC                 = 3,
            TAI                 = 4,
            GPS                 = 5,
            File                = 6
        };
    protected:
        inline virtual void OutputName(const std::string_view str) {}
        inline virtual void OutputUint8(uint8_t t) {}
        inline virtual void OutputUint16(uint16_t t) {}
        inline virtual void OutputUint32(uint32_t t) {}


        /**
         * @brief Counts the publicly accesible members of a class (without the SkipSerialization annotation)
         * 
         * @tparam T The type
         * @return The amount of member variables 
         */
        template<class T>
        static consteval size_t CountMembers() {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            size_t count = 0;
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if constexpr (HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                count++;
            }
            return count;
        }

        template<class T> requires (!SpecialSerialization<T> && !NotImplementedSerialization<T>)
        void SerializeInternal(std::string_view name) {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            constexpr size_t amountMembers = CountMembers<T>();
            if(amountMembers == 0) WARNING("[Util::ClassStructureBase] Found a class (" + PrettyNameOf<T>() + ") with 0 member variables, are you sure it is intented this class has only private members? (maybe you forgot to add 'friend class Util::ClassStructureSerializer')")

            OutputUint8(SerializationType::Class);
            OutputName(name);
            OutputUint8(amountMembers);
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if constexpr (HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                else SerializeInternal<typename [: std::meta::type_of(memberTypeInfo) :]>(std::meta::identifier_of(memberTypeInfo));
            }
        }
        
        template<NotImplementedSerialization T>
        void SerializeInternal(std::string_view name) {
            static_assert(false, "[Util::ClassStructureBase] STD library class not yet implemented");
        }

        template<FundamentalType T>
        void SerializeInternal(std::string_view name) {
            if      constexpr(IsType<T, bool>) OutputUint8(SerializationType::Uint8);
            else if constexpr(sizeof(T) == 1  &&  std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Uint8);
            else if constexpr(sizeof(T) == 2  &&  std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Uint16);
            else if constexpr(sizeof(T) == 4  &&  std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Uint32);
            else if constexpr(sizeof(T) == 8  &&  std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Uint64);
            else if constexpr(sizeof(T) == 16 &&  std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Uint128);
            else if constexpr(sizeof(T) == 1  &&  std::numeric_limits<T>::is_integer &&  std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Int8);
            else if constexpr(sizeof(T) == 2  &&  std::numeric_limits<T>::is_integer &&  std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Int16);
            else if constexpr(sizeof(T) == 4  &&  std::numeric_limits<T>::is_integer &&  std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Int32);
            else if constexpr(sizeof(T) == 8  &&  std::numeric_limits<T>::is_integer &&  std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Int64);
            else if constexpr(sizeof(T) == 16 &&  std::numeric_limits<T>::is_integer &&  std::numeric_limits<T>::is_signed) OutputUint8(SerializationType::Int128);
            else if constexpr(sizeof(T) == 4  && !std::numeric_limits<T>::is_integer) OutputUint8(SerializationType::Float);
            else if constexpr(sizeof(T) == 8  && !std::numeric_limits<T>::is_integer) OutputUint8(SerializationType::Double);
            else if constexpr(sizeof(T) == 16 && !std::numeric_limits<T>::is_integer) OutputUint8(SerializationType::LongDouble);
            else static_assert(false, "[Util::ClassStructureBase] SerializeInternal got called with an unknown FundamentalType");
            OutputName(name);
        }

        template<class T> requires ((IterableSTL<T> && InsertableSTL<T> && !IsType<T, std::string>)|| IsTemplatedType<T, ^^std::forward_list> || IsTemplatedType<T, ^^std::queue> || IsTemplatedType<T, ^^std::priority_queue> || IsTemplatedType<T, ^^std::stack> || IsTemplatedType<T, ^^std::array>)
        void SerializeInternal(std::string_view name) {
            if      constexpr (IsTemplatedType<T, ^^std::array>)               OutputUint8(SerializationType::Array);
            else if constexpr (IsTemplatedType<T, ^^std::vector>)              OutputUint8(SerializationType::Vector);
            else if constexpr (IsTemplatedType<T, ^^std::deque>)               OutputUint8(SerializationType::Deque);
            else if constexpr (IsTemplatedType<T, ^^std::forward_list>)        OutputUint8(SerializationType::ForwardList);
            else if constexpr (IsTemplatedType<T, ^^std::list>)                OutputUint8(SerializationType::List);
            else if constexpr (IsTemplatedType<T, ^^std::set>)                 OutputUint8(SerializationType::Set);
            else if constexpr (IsTemplatedType<T, ^^std::multiset>)            OutputUint8(SerializationType::Multiset);
            else if constexpr (IsTemplatedType<T, ^^std::map>)                 OutputUint8(SerializationType::Map);
            else if constexpr (IsTemplatedType<T, ^^std::multimap>)            OutputUint8(SerializationType::Multimap);
            else if constexpr (IsTemplatedType<T, ^^std::unordered_set>)       OutputUint8(SerializationType::UnorderedSet);
            else if constexpr (IsTemplatedType<T, ^^std::unordered_multiset>)  OutputUint8(SerializationType::UnorderedMultiset);
            else if constexpr (IsTemplatedType<T, ^^std::unordered_map>)       OutputUint8(SerializationType::UnorderedMap);
            else if constexpr (IsTemplatedType<T, ^^std::unordered_multimap>)  OutputUint8(SerializationType::UnorderedMultimap);
            else if constexpr (IsTemplatedType<T, ^^std::stack>)               OutputUint8(SerializationType::Stack);
            else if constexpr (IsTemplatedType<T, ^^std::queue>)               OutputUint8(SerializationType::Queue);
            else if constexpr (IsTemplatedType<T, ^^std::priority_queue>)      OutputUint8(SerializationType::PriorityQueue);
            else if constexpr (IsTemplatedType<T, ^^std::flat_set>)            OutputUint8(SerializationType::FlatSet);
            else if constexpr (IsTemplatedType<T, ^^std::flat_multiset>)       OutputUint8(SerializationType::FlatMultiset);
            else if constexpr (IsTemplatedType<T, ^^std::flat_map>)            OutputUint8(SerializationType::FlatMap);
            else if constexpr (IsTemplatedType<T, ^^std::flat_multimap>)       OutputUint8(SerializationType::FlatMultimap);
            else static_assert(false, "[Util::ClassStructureBase] SerializeInternal got called with an unknown STL container");
            OutputName(name);
            SerializeInternal<typename T::value_type>("STL-Value-Type");
        }
        template<class T> requires IsType<T, std::string>
        void SerializeInternal(std::string_view name) {
            OutputUint8(SerializationType::String);
            OutputName(name);
        }
        template<PointerType T>
        void SerializeInternal(std::string_view name) {
            SerializeInternal<std::pointer_traits<T>::element_type>(name);
        }
        template<class T> requires (IsTemplatedType<T, ^^std::shared_ptr> || IsTemplatedType<T, ^^std::unique_ptr> || IsTemplatedType<T, ^^std::atomic>)
        void SerializeInternal(std::string_view name) {
            constexpr auto templateArguments = std::define_static_array(std::meta::template_arguments_of(^^T));
            SerializeInternal<typename [: templateArguments[0] :] >(name); // Value in the pointer
        }
        template<ArrayType T>
        void SerializeInternal(std::string_view name) {
            OutputUint8(SerializationType::Array);
            OutputName(name);
            SerializeInternal<typename [: std::meta::remove_extent(^^T) :]>("Value-type");
        }
        template<class T> requires IsTemplatedType<T, ^^std::chrono::time_point>
        void SerializeInternal(const std::string_view name) {
            OutputUint8(SerializationType::TimePoint);
            OutputName(name);
            constexpr auto templateArguments = std::define_static_array(std::meta::template_arguments_of(^^T));
            if      constexpr (IsType<typename [: templateArguments[0] :], std::chrono::system_clock>)           OutputUint8(ClockTypes::System);
            else if constexpr (IsType<typename [: templateArguments[0] :], std::chrono::steady_clock>)           OutputUint8(ClockTypes::Steady);
            else if constexpr (IsType<typename [: templateArguments[0] :], std::chrono::high_resolution_clock>)  OutputUint8(ClockTypes::HighResolution);
#if defined(__cpp_lib_chrono) && (__cpp_lib_chrono >= 201907L)
            else if constexpr (IsType<typename [: templateArguments[0] :], std::chrono::utc_clock>)              OutputUint8(ClockTypes::UTC);
            else if constexpr (IsType<typename [: templateArguments[0] :], std::chrono::tai_clock>)              OutputUint8(ClockTypes::TAI);
            else if constexpr (IsType<typename [: templateArguments[0] :], std::chrono::gps_clock>)              OutputUint8(ClockTypes::GPS);
#endif
            else if constexpr (IsType<typename [: templateArguments[0] :], std::chrono::file_clock>)             OutputUint8(ClockTypes::File);
            else static_assert(false, "[Util::ClassStructureBase] Unknown clock type");
            SerializeInternal<typename [: templateArguments[1] :]>("STL-TimePoint-Duration");
        }

        template<class T> requires IsTemplatedType<T, ^^std::chrono::duration>
        void SerializeInternal(const std::string_view name) {
            OutputUint8(SerializationType::Duration);
            OutputName(name);
            constexpr auto templateArguments = std::define_static_array(std::meta::template_arguments_of(^^T));
            SerializeInternal<typename [: templateArguments[0] :]>("STL-Duration-Type");
            constexpr auto period = templateArguments[1];
            constexpr auto periodTemplateArguments = std::define_static_array(std::meta::template_arguments_of(period));
            constexpr auto numerator = [: periodTemplateArguments[0] :];
            constexpr auto denominator = [: periodTemplateArguments[1] :];
            static_assert(numerator < UINT32_MAX, "[Util::ClassStructureBase] Found a duration with a period too big to represent");
            static_assert(denominator < UINT32_MAX, "[Util::ClassStructureBase] Found a duration with a period too big to represent");
            OutputUint32(numerator);
            OutputUint32(denominator);
        }
        template<class T> requires IsTemplatedType<T, ^^std::pair>
        void SerializeInternal(std::string_view name) {
            OutputUint8(SerializationType::Class);
            OutputName(name);
            constexpr auto templateArguments = std::define_static_array(std::meta::template_arguments_of(^^T));
            SerializeInternal<typename [: templateArguments[0] :]>("first");
            SerializeInternal<typename [: templateArguments[1] :]>("second");
        }
    };

    /**
     * @brief Used to communicate to another language that is loosely typed how a c++ class looks
     * Used my the networking module to send a class structure over a websocket
     * 
     * @warning This class cannot magically access private members, if you want private members serialized, you need to add ```friend class Util::ClassStructureBase```
     */
    class ClassStructureSerializer : public ClassStructureBase {
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
            ASSERT(!(IsFlagSet(OutputFlag::OutputBigEndian) && IsFlagSet(OutputFlag::OutputLitleEndian)), "[Util::ClassStructureSerializer] Cannot output both big and litle endian");
            if(!IsFlagSet(OutputFlag::OutputBigEndian) && !IsFlagSet(OutputFlag::OutputLitleEndian)) {
                if(isSystemBigEndian) flags |= OutputBigEndian;
                else flags |= OutputLitleEndian;
            }
            OutputUint8(flags);// flags
            if(!IsFlagSet(OutputFlag::ExcludeVersioning))
                OutputUint8(ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION);// version
            SerializeInternal<T>(PrettyNameOf<T>());
        }

    protected:
        inline void OutputName(const std::string_view str) override {
            if(IsFlagSet(OutputFlag::ExcludeVariableNames)) return;
            ASSERT(str.size() < UINT16_MAX, "[Util::ClassStructureSerializer] Cannot output a string bigger than UINT16_MAX")
            OutputUint16(str.size());
            for(const char c : str) {
                OutputUint8(static_cast<uint8_t>(c));
            }
        }
        inline void OutputUint8(uint8_t t) override {
            _outputStream->write(reinterpret_cast<char*>(&t), sizeof(uint8_t));
            ASSERT(!_outputStream->fail() && !_outputStream->bad(), "[Util::ClassStructureSerializer] Output buffer is not big enough")
        }
        inline void OutputUint16(uint16_t t) override {
            if(IsFlagSet(OutputFlag::OutputBigEndian)) t = Util::ToBigEndian(t);
            if(IsFlagSet(OutputFlag::OutputLitleEndian)) t = Util::ToLitleEndian(t);

            _outputStream->write(reinterpret_cast<char*>(&t), sizeof(uint16_t));
            ASSERT(!_outputStream->fail() && !_outputStream->bad(), "[Util::ClassStructureSerializer] Output buffer is not big enough")
        }
        inline void OutputUint32(uint32_t t) override {
            if(IsFlagSet(OutputFlag::OutputBigEndian)) t = Util::ToBigEndian(t);
            if(IsFlagSet(OutputFlag::OutputLitleEndian)) t = Util::ToLitleEndian(t);

            _outputStream->write(reinterpret_cast<char*>(&t), sizeof(uint32_t));
            ASSERT(!_outputStream->fail() && !_outputStream->bad(), "[Util::ClassStructureSerializer] Output buffer is not big enough")
        }
        inline bool IsFlagSet(const OutputFlag flag) const {
            return _flags & flag;
        }

        std::basic_ostream<char>* _outputStream;
        uint8_t _flags = 0;
    };

    /**
     * @brief Checks if the data in a datastream is the same data produced by the ClassStructureSerializer for a certain type
     * Throws if it receives data that does not match the type it is checking.
     * 
     * @warning This class cannot magically access private members, if you want private members serialized, you need to add ```friend class Util::ClassStructureBase```
     */
    class ClassStructureDeserializer : protected ClassStructureBase {
    public:

        template<class T, class Q> requires(sizeof(Q) == 1)
        void CheckStructure(std::vector<Q>& vector) {
            VectorStreamBuffer<Q, char> buffer(vector);
            std::istream tempStream(&buffer);
            CheckStructure<T>(tempStream);
        }
        template<class T>
        void CheckStructure(std::basic_istream<char>& stream) {
            _inputStream = &stream;
            _flags = _inputStream->peek();
            ASSERT(!(IsFlagSet(OutputFlag::OutputBigEndian) && IsFlagSet(OutputFlag::OutputLitleEndian)), "[Util::ClassStructureDeserializer] Cannot input both big and litle endian");
            OutputUint8(_flags);
            if(!IsFlagSet(OutputFlag::ExcludeVersioning))
                OutputUint8(ENGINE_UTIL_CLASS_STRUCTURE_SERIALIZATION_VERSION);// version
            SerializeInternal<T>(PrettyNameOf<T>());
        }

    private:
        inline void OutputName(const std::string_view str) override {
            if(IsFlagSet(OutputFlag::ExcludeVariableNames)) return;
            uint16_t nameLength;
            _inputStream->read(reinterpret_cast<char*>(&nameLength), sizeof(uint16_t));
            if(!IsFlagSet(OutputFlag::CheckVariableNames)) {
                // Skip the name
                _inputStream->ignore(nameLength);
                return;
            }
            ASSERT(nameLength == str.length(), "[Util::ClassStructureDeserializer] Input bytes do not match the type this is checking (variable name is not the same)")
            for(const char c : str) {
                OutputUint8(static_cast<uint8_t>(c));
            }
        }
        inline void OutputUint8(uint8_t t) override {
            uint8_t input;
            _inputStream->read(reinterpret_cast<char*>(&input), sizeof(uint8_t));
            ASSERT(!_inputStream->fail() && !_inputStream->bad(), "[Util::ClassStructureDeserializer] Input buffer is not big enough")
            ASSERT(input == t, "[Util::ClassStructureDeserializer] Input bytes do not match the type this is checking")
        }
        inline void OutputUint16(uint16_t t) override {
            if(IsFlagSet(OutputFlag::OutputBigEndian)) t = Util::ToBigEndian(t);
            if(IsFlagSet(OutputFlag::OutputLitleEndian)) t = Util::ToLitleEndian(t);

            uint16_t input;
            _inputStream->read(reinterpret_cast<char*>(&input), sizeof(uint16_t));
            ASSERT(!_inputStream->fail() && !_inputStream->bad(), "[Util::ClassStructureDeserializer] Input buffer is not big enough")
            ASSERT(input == t, "[Util::ClassStructureDeserializer] Input bytes do not match the type this is checking")
        }
        inline void OutputUint32(uint32_t t) override {
            if(IsFlagSet(OutputFlag::OutputBigEndian)) t = Util::ToBigEndian(t);
            if(IsFlagSet(OutputFlag::OutputLitleEndian)) t = Util::ToLitleEndian(t);

            uint32_t input;
            _inputStream->read(reinterpret_cast<char*>(&input), sizeof(uint32_t));
            ASSERT(!_inputStream->fail() && !_inputStream->bad(), "[Util::ClassStructureDeserializer] Input buffer is not big enough")
            ASSERT(input == t, "[Util::ClassStructureDeserializer] Input bytes do not match the type this is checking")
        }
        inline bool IsFlagSet(const OutputFlag flag) const {
            return _flags & flag;
        }
    
        std::basic_istream<char>* _inputStream;
        uint8_t _flags = 0;
    };

}
}

#endif