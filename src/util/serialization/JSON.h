#ifndef ENGINE_UTIL_SERIALIZATION_JSON_H
#define ENGINE_UTIL_SERIALIZATION_JSON_H

#include "core/PCH.h"
#include "util/serialization/Serialization.h"

namespace Engine {
namespace Util {
    
    /**
     * @brief Will serialize a class into valid JSON
     * Because c++ does not support reflection the names of variables will be a number counting upwards
     * 
     */
    class JSONSerializer : public Serializer {
    public:
        template<class T>
        void Serialize(T& t, std::string& s) {
            std::stringstream stream = std::stringstream(s);
            Serialize(t, stream);
            s = stream.str();
        }
        template<class T>
        void Serialize(T& t, std::basic_ostream<char>& s) {
            _outputStream = &s;
            _state.clear();
            Serializer::Serialize(t);
        }

        inline void StartClass(const size_t amountMembers) override {
            if(_state.size()) StartVariable();
            AddString("{");
            _state.push_back({0, false});
        }
        inline void EndClass() override {
            AddString("}");
            _state.pop_back();
            if(_state.size()) EndVariable();
        }

        inline void StartPair() override {
            StartArray(2);
        }
        inline void EndPair() override {
            EndArray();
        }

        inline void StartArray(const size_t amountElements) override {
            StartVariable();
            AddString("[");
            _state.push_back({0, true});
        }
        inline void EndArray() override {
            AddString("]");
            _state.pop_back();
            EndVariable();
        }

        inline void AddVariable(const int8_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }
        inline void AddVariable(const int16_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }
        inline void AddVariable(const int32_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }
        inline void AddVariable(const int64_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }

        inline void AddVariable(const uint8_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }
        inline void AddVariable(const uint16_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }
        inline void AddVariable(const uint32_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }
        inline void AddVariable(const uint64_t v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }

        inline void AddVariable(const float v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }
        inline void AddVariable(const double v) override {
            StartVariable();
            AddString(std::to_string(v));
            EndVariable();
        }

        inline void AddVariable(const std::string& v) override {
            StartVariable();
            AddString("\"");
            AddString(v);
            AddString("\"");
            EndVariable();
        }
    private:
        inline void AddString(const std::string& s) {
            (*_outputStream) << s;
        }

        inline void StartVariable() {
            if(_state[_state.size()-1]._variableCount) {
                AddString(", ");
            }
            if(_state[_state.size()-1]._isArray) return;
            AddString("\"");
            AddString(std::to_string(_state[_state.size()-1]._variableCount));
            AddString("\": ");
        }
        inline void EndVariable() {
            _state[_state.size()-1]._variableCount++;
        }

        struct State {
            uint_fast32_t _variableCount;
            bool _isArray = false;
        };
        std::vector<State> _state;

        std::basic_ostream<char>* _outputStream = nullptr;
    };

    /**
     * @brief Will deserialize the JSON created by the JSONSerializer
     * This class makes heavy use of the template type that is passed to infer the JSON structure it must be.
     * If you want to give this JSON it should be formatted like it came out of the JSONSerializer class.
     * 
     * Yes, deserializing JSON is a lot more difficult than serializing it
     * 
     */
    class JSONDeserializer : public Deserializer {
    public:
        template<class T>
        void Deserialize(T& t, std::string& s) {
            std::stringstream stream = std::stringstream(s);
            Deserialize(t, stream);
            s = stream.str();
        }
        template<class T, Derived<std::basic_istream<char>> I>
        void Deserialize(T& t, I& s) {
            _inputStream = &s;

            char firstChar = ReadUntilNoSpace();
            ASSERT(firstChar == '{' || firstChar == '[', "[Util::JsonDeserializer] Received a first char that isn't either [ or {")
            PreDeserialize(_JSON);

            _state.clear();
            _state.push_back(State{0, &_JSON});
            Deserializer::Deserialize(t);
        }

        inline void StartClass() override {
            if(_state.size() == 1 && !_state[0]._variableCount) {
                // This is the first call
                ASSERT(GetState()._currentNode->IsMap(), "[Util::JsonDeserializer] Given type started class, but JSON is not a class")
                return;
            }

            if(GetState()._currentNode->IsMap()) {
                ASSERT(GetState()._currentNode->GetMap().contains(std::to_string(GetState()._variableCount)), "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                GetState()._variableCount++;
                _state.push_back(State{0, &GetState()._currentNode->GetMap()[std::to_string(GetState()._variableCount - 1)]});
            } else {
                ASSERT(GetState()._currentNode->GetVector().size() >= GetState()._variableCount, "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                GetState()._variableCount++;
                _state.push_back(State{0, &GetState()._currentNode->GetVector()[GetState()._variableCount - 1]});
            }
            ASSERT(GetState()._currentNode->IsMap(), "[Util::JsonDeserializer] Given type started class, but JSON is not a class")
        }
        inline void EndClass() override {
            _state.pop_back();
        }

        inline void StartPair() override {
            if(_state.size() == 1 && !_state[0]._variableCount) {
                // This is the first call
                ASSERT(GetState()._currentNode->IsVector(), "[Util::JsonDeserializer] Given type started class, but JSON is not a class")
                ASSERT(GetState()._currentNode->GetVector().size() == 2, "[Util::JsonDeserializer] Given type started pair, but JSON is not a array of size=2")
                return;
            }

            if(GetState()._currentNode->IsMap()) {
                ASSERT(GetState()._currentNode->GetMap().contains(std::to_string(GetState()._variableCount)), "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                GetState()._variableCount++;
                _state.push_back(State{0, &GetState()._currentNode->GetMap()[std::to_string(GetState()._variableCount - 1)]});
            } else {
                ASSERT(GetState()._currentNode->GetVector().size() >= GetState()._variableCount, "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                GetState()._variableCount++;
                _state.push_back(State{0, &GetState()._currentNode->GetVector()[GetState()._variableCount - 1]});
            }
            ASSERT(GetState()._currentNode->IsVector(), "[Util::JsonDeserializer] Given type started array, but JSON is not a array")
            ASSERT(GetState()._currentNode->GetVector().size() == 2, "[Util::JsonDeserializer] Given type started pair, but JSON is not a array of size=2")
        }
        inline void EndPair() override {
            _state.pop_back();
        }

        inline void StartArray() override {
            if(_state.size() == 1 && !_state[0]._variableCount) {
                // This is the first call
                ASSERT(GetState()._currentNode->IsVector(), "[Util::JsonDeserializer] Given type started class, but JSON is not a class")
                return;
            }

            if(GetState()._currentNode->IsMap()) {
                ASSERT(GetState()._currentNode->GetMap().contains(std::to_string(GetState()._variableCount)), "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                GetState()._variableCount++;
                _state.push_back(State{0, &GetState()._currentNode->GetMap()[std::to_string(GetState()._variableCount - 1)]});
            } else {
                ASSERT(GetState()._currentNode->GetVector().size() >= GetState()._variableCount, "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                GetState()._variableCount++;
                _state.push_back(State{0, &GetState()._currentNode->GetVector()[GetState()._variableCount - 1]});
            }
            ASSERT(GetState()._currentNode->IsVector(), "[Util::JsonDeserializer] Given type started array, but JSON is not a array")
        }
        inline bool IsEndArray() override {
            return GetState()._currentNode->GetVector().size() == GetState()._variableCount;
        }
        inline void EndArray() override {
            _state.pop_back();
        }

        inline void GetVariable(int8_t& v) override {
            GetVariableInternal(v);
        }
        inline void GetVariable(int16_t& v) override {
            GetVariableInternal(v);
        }
        inline void GetVariable(int32_t& v) override {
            GetVariableInternal(v);
        }
        inline void GetVariable(int64_t& v) override {
            GetVariableInternal(v);
        }

        inline void GetVariable(uint8_t& v) override {
            GetVariableInternal(v);
        }
        inline void GetVariable(uint16_t& v) override {
            GetVariableInternal(v);
        }
        inline void GetVariable(uint32_t& v) override {
            GetVariableInternal(v);
        }
        inline void GetVariable(uint64_t& v) override {
            GetVariableInternal(v);
        }

        inline void GetVariable(float& v) override {
            GetVariableInternal(v);
        }
        inline void GetVariable(double& v) override {
            GetVariableInternal(v);
        }

        inline void GetVariable(std::string& v) override {
            GetVariableInternal(v);
        }
    private:
        inline void GetVariableInternal(std::string& v) {
            JsonData* node;
            if(GetState()._currentNode->IsMap()) {
                ASSERT(GetState()._currentNode->GetMap().contains(std::to_string(GetState()._variableCount)), "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                node = &GetState()._currentNode->GetMap()[std::to_string(GetState()._variableCount)];
            } else {
                ASSERT(GetState()._currentNode->GetVector().size() >= GetState()._variableCount, "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
                node = &GetState()._currentNode->GetVector()[GetState()._variableCount];
            }
            GetState()._variableCount++;
            ASSERT(node->IsString(), "[Util::JsonDeserializer] Trying to deserialize JSON that does not contain enough variables for the give type")
            v = node->GetString();
        }
        template<FundamentalType T>
        inline void StringToNumber(std::string& v, T& number) {
            auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), number);
            if (ec == std::errc::invalid_argument)
                THROW("[Util::JsonDeserializer] Trying to deserialize a number, but did not receive one")
            else if (ec == std::errc::result_out_of_range)
                THROW("[Util::JsonDeserializer] Trying to deserialize a number, but received a number that is too large")
            else if(ptr != NULL && ptr[0] != '\0')
                THROW("[Util::JsonDeserializer] Trying to deserialize a number, but received a number followed by some string '" + std::string(ptr) + "'")
        }
        template<FundamentalType T>
        inline void GetVariableInternal(T& t) {
            std::string value;
            GetVariableInternal(value);
            StringToNumber(value, t);
        }

        struct JsonData;
        struct State {
            uint_fast32_t _variableCount;
            JsonData* _currentNode = nullptr;
        };
        std::vector<State> _state;
        State& GetState() {
            return _state[_state.size()-1];
        }

        std::basic_istream<char>* _inputStream = nullptr;
        struct JsonData {
            std::variant<
                std::map<std::string, JsonData>,
                std::vector<JsonData>,
                std::string
            > _data;

            JsonData() {}
            JsonData(std::map<std::string, JsonData>& s) : _data(s) {}
            JsonData(std::vector<JsonData>& s) : _data(s) {}
            JsonData(std::string& s) : _data(s) {}

            JsonData& MakeMap() {
                _data = std::map<std::string, JsonData>();
                return *this;
            }
            JsonData& MakeVector() {
                _data = std::vector<JsonData>();
                return *this;
            }
            JsonData& MakeString() {
                _data = "";
                return *this;
            }

            std::map<std::string, JsonData>& GetMap() {
                return std::get<std::map<std::string, JsonData>>(_data);
            }
            std::vector<JsonData>& GetVector() {
                return std::get<std::vector<JsonData>>(_data);
            }
            std::string& GetString() {
                return std::get<std::string>(_data);
            }

            bool IsMap() {
                return std::holds_alternative<std::map<std::string, JsonData>>(_data);
            }
            bool IsVector() {
                return std::holds_alternative<std::vector<JsonData>>(_data);
            }
            bool IsString() {
                return std::holds_alternative<std::string>(_data);
            }
        };
        JsonData _JSON;

        /**
         * @brief Checks if JSON is correct and deserializes it into data
         * 
         */
        void PreDeserialize(JsonData& data) {
            char input;
            std::string currentVariableName = "";

            while(!_inputStream->eof()) {
                input = ReadUntilNoSpace();
                if(input == '}') {
                    ASSERT(data.IsMap(), "[Util::JSONDeserializer] Received } in a wrong place")
                    return;
                }
                else if(input == ']') {
                    ASSERT(data.IsVector(), "[Util::JSONDeserializer] Received ] in a wrong place")
                    return;
                }
                else if(input == ',') {
                    continue;
                }
                else if((input == '"'&&data.IsMap()) || data.IsVector()) {
                    currentVariableName = "";
                    if(data.IsMap()) {
                        currentVariableName = ReadString();
                        ASSERT(ReadUntilNoSpace() == ':', "[Util::JSONDeserializer] Found characters after variable name that is neither : or a whitespace")
                        input = ReadUntilNoSpace();
                    }
                    if(input == '{') {
                        // Start new map
                        if(data.IsMap()) PreDeserialize(data.GetMap()[currentVariableName].MakeMap());
                        else if(data.IsVector()) {
                            data.GetVector().push_back(JsonData());
                            JsonData& newData = data.GetVector()[data.GetVector().size()-1];
                            PreDeserialize(newData.MakeMap());
                        }
                        else THROW("[Util::JSONDeserializer] Got unsupported JsonData type")
                    } else if(input == '[') {
                        // Start new array
                        if(data.IsMap()) PreDeserialize(data.GetMap()[currentVariableName].MakeVector());
                        else if(data.IsVector()) {
                            data.GetVector().push_back(JsonData());
                            JsonData& newData = data.GetVector()[data.GetVector().size()-1];
                            PreDeserialize(newData.MakeVector());
                        }
                        else THROW("[Util::JSONDeserializer] Got unsupported JsonData type")
                    } else if(input == '"') {
                        // Read in the string
                        std::string str = ReadString();
                        if(data.IsMap()) data.GetMap()[currentVariableName] = JsonData(str);
                        else if(data.IsVector()) data.GetVector().push_back(str);
                        else THROW("[Util::JSONDeserializer] Got unsupported JsonData type")
                    } else if((input >= '0' && input <= '9') || input == '.') {
                        // Read in the number
                        std::string str = std::string(&input, 1);
                        while(!_inputStream->eof()) {
                            _inputStream->read(&input, 1);
                            if((input < '0' || input > '9') && input != '.') break;
                            str += input;
                        }
                        if(input == ']' || input == '}') return;
                        ASSERT(std::isspace(input) || input == ',', "[Util::JSONDeserializer] Received character after number that isn't a space or ,")
                        
                        if(data.IsMap()) data.GetMap()[currentVariableName] = JsonData(str);
                        else if(data.IsVector()) data.GetVector().push_back(str);
                        else THROW("[Util::JSONDeserializer] Got unsupported JsonData type")
                    } else {
                        THROW("[Util::JSONDeserializer] Received an unsupported character to start a data value")
                    }
                }
                else {
                    THROW("[Util::JSONDeserializer] Received an unsupported character")
                }
            }
        }
        char ReadUntilNoSpace() {
            char input;
            while(!_inputStream->eof()) {
                _inputStream->read(&input, 1);
                if(!std::isspace(input)) return input;
            }
        }
        std::string ReadString() {
            char input = ' ';
            char previousInput = ' ';
            std::string ret;
            while(!_inputStream->eof()) {
                previousInput = input;
                _inputStream->read(&input, 1);
                if(previousInput != '\\' && input == '"') return ret;
                ret += input;
            }
        }

    };

}
}

#endif