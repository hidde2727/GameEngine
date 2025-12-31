#ifndef ENGINE_UTIL_SERIALIZATION_BINARY_H
#define ENGINE_UTIL_SERIALIZATION_BINARY_H

#include "core/PCH.h"
#include "util/serialization/Serialization.h"
#include "util/Endianess.h"
#include "util/VectorStreamBuffer.h"
#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    #ifndef ENGINE_UTIL_BINARY_SERIALIZATION_VERSION
    #define ENGINE_UTIL_BINARY_SERIALIZATION_VERSION 3
    #endif
    
    /**
     * @brief Will serialize a class into a binary format
     * The binary format can be used to send data over a websocket or to store a class on the disk
     * 
     * @warning This class cannot magically access private members, if you want private members serialized, you need to add ```friend class Util::Serializer```
     */
    class BinarySerializer : public Serializer {
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
            Serializer::Serialize(obj);
        }


        inline void StartClass(const size_t amountMembers, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Class);
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint32_t)amountMembers);
        }
        inline void EndClass() override {

        }

        inline void StartPair(const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Pair);
        }
        inline void EndPair() override {

        }

        inline void StartArray(const size_t amountElements, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Array);
            ASSERT(amountElements < UINT32_MAX, "[Util::BinarySerializer] Cannot serialize arrays with a size larger than UINT32_MAX")
            Output((uint32_t)amountElements);
        }
        inline void EndArray() override {

        }
        
        inline void StartMap(const size_t amountElements, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Map);
            ASSERT(amountElements < UINT32_MAX, "[Util::BinarySerializer] Cannot serialize maps with a size larger than UINT32_MAX")
            Output((uint32_t)amountElements);
        }
        inline void EndMap() override {

        }

        inline void AddVariable(const int8_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Int8);
            Output(v);
        }
        inline void AddVariable(const int16_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Int16);
            Output(v);
        }
        inline void AddVariable(const int32_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Int32);
            Output(v);
        }
        inline void AddVariable(const int64_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Int64);
            Output(v);
        }

        inline void AddVariable(const uint8_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::UInt8);
            Output(v);
        }
        inline void AddVariable(const uint16_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::UInt16);
            Output(v);
        }
        inline void AddVariable(const uint32_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::UInt32);
            Output(v);
        }
        inline void AddVariable(const uint64_t v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::UInt64);
            Output(v);
        }

        inline void AddVariable(const bool v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Bool);
            Output<uint8_t>(v);
        }

        inline void AddVariable(const float v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Float);
            if(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4) {
                // It is already stored in the correct format
                float copy = v;
                Output(*reinterpret_cast<uint32_t*>(&copy));
            } else {
                THROW("[Util::BinarySerializer] Binary serializing floats that are not stored according to IEEE 754 is not implemented yet")
            }
        }
        inline void AddVariable(const double v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::Double);
            if(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8) {
                // It is already stored in the correct format
                double copy = v;
                Output(*reinterpret_cast<uint64_t*>(&copy));
            } else {
                THROW("[Util::BinarySerializer] Binary serializing doubles that are not stored according to IEEE 754 is not implemented yet")
            }
        }

        inline void AddVariable(const std::string& v, const std::string_view name="") override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) Output((uint8_t)SerializationTypes::String);
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
     * @warning This class cannot magically access private members, if you want private members deserialized, you need to add ```friend class Util::Deserializer```
     */
    class BinaryDeserializer : public Deserializer {
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
            Deserializer::Deserialize(obj);
        }


        inline void StartClass(const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Class);
            OnVariableStart();
            size_t expectedAmountMembers = 1000000000000000;
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) expectedAmountMembers = GetFromInput<uint32_t>();
            _state.push_back({false, expectedAmountMembers});
        }
        inline void EndClass() override {
            if(IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) {
                ASSERT(GetState()._expectedAmountElements == 0, "[Util::BinaryDeserializer] Expected amount of members not met")
            }
            _state.pop_back();
        }

        inline void StartPair(const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Pair);
            OnVariableStart();
            _state.push_back({false, 2});
        }
        inline void EndPair() override {
            ASSERT(GetState()._expectedAmountElements == 0, "[Util::BinaryDeserializer] There weren't 2 elements inside a pair")
            _state.pop_back();
        }

        inline void StartArray(const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Array);
            OnVariableStart();
            size_t expectedAmountElements = GetFromInput<uint32_t>();
            _state.push_back({true, expectedAmountElements});
        }
        inline bool IsEndArray() override {
            return GetState()._expectedAmountElements == 0;
        }
        inline void EndArray() override {
            _state.pop_back();            
        }

        inline void StartMap(const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Map);
            OnVariableStart();
            size_t expectedAmountElements = GetFromInput<uint32_t>();
            _state.push_back({true, expectedAmountElements});
        }
        inline bool IsEndMap() override {
            return GetState()._expectedAmountElements == 0;            
        }
        inline void EndMap() override {
            _state.pop_back();  
        }

        inline void GetVariable(int8_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Int8);
            OnVariableStart();
            v = GetFromInput<int8_t>();
        }
        inline void GetVariable(int16_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Int16);
            OnVariableStart();
            v = GetFromInput<int16_t>();
        }
        inline void GetVariable(int32_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Int32);
            OnVariableStart();
            v = GetFromInput<int32_t>();
        }
        inline void GetVariable(int64_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Int64);
            OnVariableStart();
            v = GetFromInput<int64_t>();
        }

        inline void GetVariable(uint8_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::UInt8);
            OnVariableStart();
            v = GetFromInput<uint8_t>();
        }
        inline void GetVariable(uint16_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::UInt16);
            OnVariableStart();
            v = GetFromInput<uint16_t>();
        }
        inline void GetVariable(uint32_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::UInt32);
            OnVariableStart();
            v = GetFromInput<uint32_t>();
        }
        inline void GetVariable(uint64_t& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::UInt64);
            OnVariableStart();
            v = GetFromInput<uint64_t>();
        }

        inline void GetVariable(bool& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Bool);
            OnVariableStart();
            v = GetFromInput<uint8_t>();
        }

        inline void GetVariable(float& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Float);
            OnVariableStart();
            if constexpr (std::numeric_limits<float>::is_iec559 && sizeof(float) == 4) {
                // It is already stored in the correct format
                uint32_t copy = GetFromInput<uint32_t>();
                v = *reinterpret_cast<float*>(&copy);
            } else {
                THROW("[Util::BinaryDeserializer] Binary deserializing doubles that are not stored according to IEEE 754 is not implemented yet");
            }            
        }
        inline void GetVariable(double& v, const std::string_view name="") override {
            CheckTypeInfo(SerializationTypes::Double);
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
            CheckTypeInfo(SerializationTypes::String);
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
        void CheckTypeInfo(SerializationTypes type) {
            if(!IsFlagSet(BinarySerializationOutputFlag::IncludeTypeInfo)) return;
            ASSERT(GetFromInput<uint8_t>() == type, "[Util::BinaryDeserializer] Found the wrong type in the binary vs the one expected")
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