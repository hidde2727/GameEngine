#ifndef ENGINE_UTIL_SERIALIZATION_BINARY_H
#define ENGINE_UTIL_SERIALIZATION_BINARY_H

#include "core/PCH.h"
#include "util/serialization/Serialization.h"
#include "util/serialization/ClassStructure.h"
#include "util/Endianess.h"
#include "util/VectorStreamBuffer.h"
#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    #ifndef ENGINE_UTIL_BINARY_SERIALIZATION_VERSION
    #define ENGINE_UTIL_BINARY_SERIALIZATION_VERSION 4
    #endif
    
    enum BinarySerializationOutputFlag : uint8_t {
        IncludeTypeInfo      = 1, // Defaults to not include type info
        OutputBigEndian      = 2, // Default is system endianess
        OutputLitleEndian    = 4, // Default is system endianess
        ExcludeVersioning    = 8, // Defaults to including the versioning data
        ExcludeVariableNames = 16,// Defaults to include variable names (is only output if IncludeTypeInfo=true)
        CheckVariableNames   = 32 // Defaults to no  (is only checked if IncludeTypeInfo=true)
    };
    
    /**
     * @brief Will serialize a class into a binary format
     * The binary format can be used to send data over a websocket or to store a class on the disk
     * 
     * @warning This class cannot magically access private members, if you want private members serialized, you need to add ```friend class Util::Serializer<Util::BinarySerializer>```
     */
    class BinarySerializer : public Serializer<BinarySerializer> {
    public:
        template<class T, class Q> requires(sizeof(Q) == 1)
        void Serialize(T& obj, std::vector<Q>& vector, uint8_t flags = 0) {
            VectorStreamBuffer<Q, char> buffer(vector);
            std::ostream tempStream(&buffer);
            Serialize(obj, tempStream, flags);
        }
        template<class T>
        void Serialize(T& obj, std::basic_ostream<char>& stream, uint8_t flags = 0) {
            _outputStream = &stream;
            _flags = flags;
            ASSERT(!(IsFlagSet(BinarySerializationOutputFlag::OutputBigEndian) && IsFlagSet(BinarySerializationOutputFlag::OutputLitleEndian)), "[Util::BinarySerializer] Cannot output both big and litle endian");
            if(!IsFlagSet(BinarySerializationOutputFlag::OutputBigEndian) && !IsFlagSet(BinarySerializationOutputFlag::OutputLitleEndian)) {
                if(isSystemBigEndian) flags |= BinarySerializationOutputFlag::OutputBigEndian;
                else flags |= BinarySerializationOutputFlag::OutputLitleEndian;
            }
            Output<uint8_t>(flags);// flags
            if(!IsFlagSet(BinarySerializationOutputFlag::ExcludeVersioning))
                Output<uint8_t>(ENGINE_UTIL_BINARY_SERIALIZATION_VERSION);// version
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) {
                ClassStructureSerializer serializer;
                serializer.Serialize<T>(*_outputStream, _flags);
            }
            Serializer<BinarySerializer>::Serialize(obj);
        }


        inline void StartClass(const size_t amountMembers, const std::string_view name="") override { }
        inline void EndClass() override { }

        template<class T>
        inline void StartSTLContainer(const size_t amountElements, const std::string_view name="") {
            ASSERT(amountElements < UINT32_MAX, "[Util::BinarySerializer] Cannot serialize arrays with a size larger than UINT32_MAX")
            Output((uint32_t)amountElements);
        }
        template<class T>
        inline void EndSTLContainer() { }


        template<class T>
        inline void AddVariable(const T v, const std::string_view name="") {
            Output(v);
        }
        template<>
        inline void AddVariable<bool>(const bool v, const std::string_view name) {
            Output<uint8_t>(v);
        }

        template<>
        inline void AddVariable<float>(const float v, const std::string_view name) {
            if(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4) {
                // It is already stored in the correct format
                float copy = v;
                Output(*reinterpret_cast<uint32_t*>(&copy));
            } else {
                THROW("[Util::BinarySerializer] Binary serializing floats that are not stored according to IEEE 754 is not implemented yet")
            }
        }
        template<>
        inline void AddVariable<double>(const double v, const std::string_view name) {
            if(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8) {
                // It is already stored in the correct format
                double copy = v;
                Output(*reinterpret_cast<uint64_t*>(&copy));
            } else {
                THROW("[Util::BinarySerializer] Binary serializing doubles that are not stored according to IEEE 754 is not implemented yet")
            }
        }

        inline void AddVariable(const std::string& v, const std::string_view name="") override {
            ASSERT(v.size() <= UINT16_MAX, "[Util::BinarySerializer] Cannot serialize string with a size bigger than UINT16_MAX")
            Output<uint16_t>(v.size());
            for(size_t i = 0; i < v.size(); i++) {
                Output(v[i]);
            }
        }
    private:
        template<IntegralType T>
        inline void Output(T t) {
            if(IsFlagSet(BinarySerializationOutputFlag::OutputBigEndian)) t = Util::ToBigEndian(t);
            if(IsFlagSet(BinarySerializationOutputFlag::OutputLitleEndian)) t = Util::ToLitleEndian(t);

            _outputStream->write(reinterpret_cast<char*>(&t), sizeof(T));
            ASSERT(!_outputStream->fail() && !_outputStream->bad(), "[Util::BinaryDeserializer] Not enough bytes in the input for deserialisation")
        }
        inline bool IsFlagSet(const BinarySerializationOutputFlag flag) const {
            return _flags & flag;
        }

        std::basic_ostream<char>* _outputStream;
        uint8_t _flags = 0;
    };

    /**
     * @brief Deserializes the binary format created by BinarySerializer
     * 
     * @warning This class cannot magically access private members, if you want private members deserialized, you need to add ```friend class Util::Deserializer<Util::BinaryDeserializer>```
     */
    class BinaryDeserializer : public Deserializer<BinaryDeserializer> {
    public:

        template<class T, class Q> requires (sizeof(Q) == 1)
        void Deserialize(T& obj, std::vector<Q>& vector, const size_t startingOffset=0) {
            VectorStreamBuffer<Q, char> buffer(vector, startingOffset);
            std::istream tempStream(&buffer);
            Deserialize(obj, tempStream);
        }
        template<class T>
        void Deserialize(T& obj, std::basic_istream<char>& stream) {
            _inputStream = &stream;
            _flags = GetFromInput<uint8_t>();
            if(!IsFlagSet(BinarySerializationOutputFlag::ExcludeVersioning))
                ASSERT(GetFromInput<uint8_t>() == ENGINE_UTIL_BINARY_SERIALIZATION_VERSION, "[Util::BinaryDeserializer] Reading wrong version of the binary serialisation")// version
            
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) {
                ClassStructureDeserializer serializer;
                serializer.CheckStructure<T>(*_inputStream);
            }
            Deserializer<BinaryDeserializer>::Deserialize(obj);
        }


        inline void StartClass(const size_t members, const std::string_view name="") override {
            OnVariableStart();
            _state.push_back({false, members});
        }
        inline void EndClass() override {
            ASSERT(GetState()._expectedAmountElements == 0, "[Util::BinaryDeserializer] Expected amount of members not met")
            _state.pop_back();
        }

        template<class T>
        inline void StartSTLContainer(const std::string_view name="") {
            OnVariableStart();
            size_t expectedAmountElements = GetFromInput<uint32_t>();
            _state.push_back({true, expectedAmountElements});
        }
        inline bool IsSTLEnd() override {
            return GetState()._expectedAmountElements == 0;
        }
        template<class T>
        inline void EndSTLContainer() {
            _state.pop_back();
        }

        template<FundamentalType T>
        inline void GetVariable(T& v, const std::string_view name="") {
            OnVariableStart();
            v = GetFromInput<T>();
        }

        template<>
        inline void GetVariable<bool>(bool& v, const std::string_view name) {
            OnVariableStart();
            v = GetFromInput<uint8_t>();
        }

        template<>
        inline void GetVariable<float>(float& v, const std::string_view name) {
            OnVariableStart();
            if constexpr (std::numeric_limits<float>::is_iec559 && sizeof(float) == 4) {
                // It is already stored in the correct format
                uint32_t copy = GetFromInput<uint32_t>();
                v = *reinterpret_cast<float*>(&copy);
            } else {
                THROW("[Util::BinaryDeserializer] Binary deserializing doubles that are not stored according to IEEE 754 is not implemented yet");
            }            
        }
        template<>
        inline void GetVariable<double>(double& v, const std::string_view name) {
            OnVariableStart();
            if constexpr (std::numeric_limits<double>::is_iec559 && sizeof(double) == 8) {
                // It is already stored in the correct format
                uint64_t copy = GetFromInput<uint64_t>();
                v = *reinterpret_cast<double*>(&copy);
            } else {
                THROW("[Util::BinaryDeserializer] Binary deserializing doubles that are not stored according to IEEE 754 is not implemented yet");
            }
        }

        inline void GetVariable(std::string& v, const std::string_view name="") override {
            OnVariableStart();
            size_t amountCharacters = GetFromInput<uint16_t>();
            v.reserve(amountCharacters);
            for(size_t i = 0; i < amountCharacters; i++) {
                v.push_back(GetFromInput<char>());
            }
        }
    private:
        template<IntegralType T>
        T GetFromInput() {
            T t;
            _inputStream->read(reinterpret_cast<char*>(&t), sizeof(T));
            ASSERT(!_inputStream->fail() && !_inputStream->eof(), "[Util::BinaryDeserializer] Not enough bytes in the input for deserialisation")
            if(IsFlagSet(BinarySerializationOutputFlag::OutputBigEndian)) t = Util::FromBigEndian(t);
            else if(IsFlagSet(BinarySerializationOutputFlag::OutputLitleEndian)) t = Util::FromLitleEndian(t);
            // Endianess only matters if the amount of bytes > 1
            else if(sizeof(T) > 1) THROW("[Util::BinaryDeserializer] No flag for endianness set in the input")
            return t;
        }
        inline bool IsFlagSet(const BinarySerializationOutputFlag flag) {
            return _flags & flag;
        }

        struct State {
            bool _isArray;
            size_t _expectedAmountElements = 0;
        };
        std::vector<State> _state;
        State& GetState() {
            return _state[_state.size() - 1];
        }
        void OnVariableStart() {
            if(!_state.size()) return;
            GetState()._expectedAmountElements--;
        }

        std::basic_istream<char>* _inputStream;
        uint8_t _flags = 0;
    };

}
}

#endif