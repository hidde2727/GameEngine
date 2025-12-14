#ifndef ENGINE_UTIL_SERIALIZATION_BINARY_H
#define ENGINE_UTIL_SERIALIZATION_BINARY_H

#include "core/PCH.h"
#include "util/serialization/Serialization.h"
#include "util/Endianess.h"
#include "util/VectorStreamBuffer.h"
#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    #ifndef ENGINE_UTIL_BINARY_SERIALISATION_VERSION
    #define ENGINE_UTIL_BINARY_SERIALISATION_VERSION 1
    #endif
    
    /**
     * @brief Will serialize a class into a binary format
     * The binary format can be used to send data over a websocket or to store a class on the disk
     * 
     */
    class BinarySerializer : public Serializer {
    public:
        enum OutputFlag {
            IncludeTypeInfo     = 1,// Defaults to not include type info
            OutputBigEndian     = 2,// Default is system endianess
            OutputLitleEndian   = 4 // Default is system endianess
        };
        enum Types {
            Class       = 0,
            Pair        = 1,
            Array       = 2,

            UInt8       = 10,
            UInt16      = 11,
            UInt32      = 12,
            UInt64      = 13,
            Int8        = 20,
            Int16       = 21,
            Int32       = 22,
            Int64       = 23,

            Float       = 30,
            Double      = 31,

            String      = 40
        };
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
            ASSERT(!(IsFlagSet(OutputFlag::OutputBigEndian) && IsFlagSet(OutputFlag::OutputLitleEndian)), "[Util::BinarySerializer] Cannot output both big and litle endian");
            if(!IsFlagSet(OutputFlag::OutputBigEndian) || !IsFlagSet(OutputFlag::OutputLitleEndian)) {
                if(isSystemBigEndian) flags |= OutputBigEndian;
                else flags |= OutputLitleEndian;
            }
            Output<uint8_t>(ENGINE_UTIL_BINARY_SERIALISATION_VERSION);// version
            Output<uint8_t>(flags);// flags
            Serializer::Serialize(obj);
        }


        inline void StartClass(const size_t amountMembers) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Class);
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint32_t)amountMembers);
        }
        inline void EndClass() override {

        }

        inline void StartPair() override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Pair);

        }
        inline void EndPair() override {

        }

        inline void StartArray(const size_t amountElements) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Array);
            ASSERT(amountElements < UINT32_MAX, "[Util::BinarySerializer] Cannot serialize arrays with a size larger than UINT32_MAX")
            Output((uint32_t)amountElements);
        }
        inline void EndArray() override {

        }

        inline void AddVariable(const int8_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Int8);
            Output(v);
        }
        inline void AddVariable(const int16_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Int16);
            Output(v);
        }
        inline void AddVariable(const int32_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Int32);
            Output(v);
        }
        inline void AddVariable(const int64_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Int64);
            Output(v);
        }

        inline void AddVariable(const uint8_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::UInt8);
            Output(v);
        }
        inline void AddVariable(const uint16_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::UInt16);
            Output(v);
        }
        inline void AddVariable(const uint32_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::UInt32);
            Output(v);
        }
        inline void AddVariable(const uint64_t v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::UInt64);
            Output(v);
        }

        inline void AddVariable(const float v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Float);
            if(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4) {
                // It is already stored in the correct format
                float copy = v;
                Output(*reinterpret_cast<uint32_t*>(&copy));
            } else {
                THROW("[Util::BinarySerializer] Binary serializing floats that are not stored according to IEEE 754 is not implemented yet")
            }
        }
        inline void AddVariable(const double v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::Double);
            if(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8) {
                // It is already stored in the correct format
                double copy = v;
                Output(*reinterpret_cast<uint64_t*>(&copy));
            } else {
                THROW("[Util::BinarySerializer] Binary serializing doubles that are not stored according to IEEE 754 is not implemented yet")
            }
        }

        inline void AddVariable(const std::string& v) override {
            if(IsFlagSet(OutputFlag::IncludeTypeInfo)) Output((uint8_t)Types::String);
            ASSERT(v.size() <= UINT16_MAX, "[Util::BinarySerializer] Cannot serialize string with a size bigger than UINT16_MAX")
            Output<uint16_t>(v.size());
            for(size_t i = 0; i < v.size(); i++) {
                Output(v[i]);
            }
        }
    private:
        template<IntegralType T>
        inline void Output(T t) {
            if(IsFlagSet(OutputFlag::OutputBigEndian)) t = Util::ToBigEndian(t);
            if(IsFlagSet(OutputFlag::OutputLitleEndian)) t = Util::ToLitleEndian(t);

            _outputStream->write(reinterpret_cast<char*>(&t), sizeof(T));
            ASSERT(!_outputStream->fail() && !_outputStream->bad(), "[Util::BinaryDeserializer] Not enough bytes in the input for deserialisation")
        }
        inline bool IsFlagSet(const OutputFlag flag) const {
            return _flags & flag;
        }

        std::basic_ostream<char>* _outputStream;
        uint8_t _flags = 0;
    };

    /**
     * @brief Deserializes the binary format created by BinarySerializer
     * 
     */
    class BinaryDeserializer : public Deserializer {
    public:

        template<class T, class Q> requires (sizeof(Q) == 1)
        void Deserialize(T& obj, std::vector<Q>& vector) {
            VectorStreamBuffer<Q, char> buffer(vector);
            std::istream tempStream(&buffer);
            Deserialize(obj, tempStream);
        }
        template<class T>
        void Deserialize(T& obj, std::basic_istream<char>& stream) {
            _inputStream = &stream;
            ASSERT(GetFromInput<uint8_t>() == ENGINE_UTIL_BINARY_SERIALISATION_VERSION, "[Util::BinaryDeserializer] Reading wrong version of the binary serialisation")// version
            _flags = GetFromInput<uint8_t>();
            Deserializer::Deserialize(obj);
        }


        inline void StartClass() override {
            CheckTypeInfo(BinarySerializer::Types::Class);
            OnVariableStart();
            size_t expectedAmountMembers = 1000000000000000;
            if(IsFlagSet(BinarySerializer::OutputFlag::IncludeTypeInfo)) expectedAmountMembers = GetFromInput<uint32_t>();
            _state.push_back({false, expectedAmountMembers});
        }
        inline void EndClass() override {
            if(IsFlagSet(BinarySerializer::OutputFlag::IncludeTypeInfo)) {
                ASSERT(GetState()._expectedAmountElements == 0, "[Util::BinaryDeserializer] Expected amount of members not met")
            }
            _state.pop_back();
        }

        inline void StartPair() override {
            CheckTypeInfo(BinarySerializer::Types::Pair);
            OnVariableStart();
            _state.push_back({false, 2});
        }
        inline void EndPair() override {
            ASSERT(GetState()._expectedAmountElements == 0, "[Util::BinaryDeserializer] There weren't 2 elements inside a pair")
            _state.pop_back();
        }

        inline void StartArray() override {
            CheckTypeInfo(BinarySerializer::Types::Array);
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

        inline void GetVariable(int8_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::Int8);
            OnVariableStart();
            v = GetFromInput<int8_t>();
        }
        inline void GetVariable(int16_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::Int16);
            OnVariableStart();
            v = GetFromInput<int16_t>();
        }
        inline void GetVariable(int32_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::Int32);
            OnVariableStart();
            v = GetFromInput<int32_t>();
        }
        inline void GetVariable(int64_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::Int64);
            OnVariableStart();
            v = GetFromInput<int64_t>();
        }

        inline void GetVariable(uint8_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::UInt8);
            OnVariableStart();
            v = GetFromInput<uint8_t>();
        }
        inline void GetVariable(uint16_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::UInt16);
            OnVariableStart();
            v = GetFromInput<uint16_t>();
        }
        inline void GetVariable(uint32_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::UInt32);
            OnVariableStart();
            v = GetFromInput<uint32_t>();
        }
        inline void GetVariable(uint64_t& v) override {
            CheckTypeInfo(BinarySerializer::Types::UInt64);
            OnVariableStart();
            v = GetFromInput<uint64_t>();
        }

        inline void GetVariable(float& v) override {
            CheckTypeInfo(BinarySerializer::Types::Float);
            OnVariableStart();
            if(std::numeric_limits<float>::is_iec559 && sizeof(float) == 8) {
                // It is already stored in the correct format
                uint32_t copy = GetFromInput<uint32_t>();
                v = *reinterpret_cast<float*>(&copy);
            } else {
                THROW("[Util::BinaryDeserializer] Binary deserializing floats that are not stored according to IEEE 754 is not implemented yet")
            }            
        }
        inline void GetVariable(double& v) override {
            CheckTypeInfo(BinarySerializer::Types::Double);
            OnVariableStart();
            if(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8) {
                // It is already stored in the correct format
                uint64_t copy = GetFromInput<uint64_t>();
                v = *reinterpret_cast<double*>(&copy);
            } else {
                THROW("[Util::BinaryDeserializer] Binary deserializing doubles that are not stored according to IEEE 754 is not implemented yet")
            }
        }

        inline void GetVariable(std::string& v) override {
            CheckTypeInfo(BinarySerializer::Types::String);
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
            if(IsFlagSet(BinarySerializer::OutputFlag::OutputBigEndian)) t = Util::FromBigEndian(t);
            else if(IsFlagSet(BinarySerializer::OutputFlag::OutputLitleEndian)) t = Util::FromLitleEndian(t);
            // Endianess only matters if the amount of bytes > 1
            else if(sizeof(T) > 1) THROW("[Util::BinaryDeserializer] No flag for endianness set in the input")
            return t;
        }
        inline bool IsFlagSet(const BinarySerializer::OutputFlag flag) {
            return _flags & flag;
        }
        void CheckTypeInfo(BinarySerializer::Types type) {
            if(!IsFlagSet(BinarySerializer::OutputFlag::IncludeTypeInfo)) return;
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