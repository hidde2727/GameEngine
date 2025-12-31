#ifndef ENGINE_UTIL_LOG_H
#define ENGINE_UTIL_LOG_H

/// @warning Including this file, right now, triggers an error in the clang compiler

#include "util/BasicLog.h"
#include "util/serialization/Serialization.h"

namespace Engine {
namespace Util {

    /**
     * @brief Will print a class type in an easy to read format (JSON like)
     * 
     * @warning This class cannot magically access private members, if you want private members serialized, you need to add ```friend class Util::Serializer```
     */
    class ClassPrinter : public Serializer {
    public:

        template<class T>
        static void Print(T& t, const std::string color) {
            ClassPrinter printer;
            printer.PrintInternal<T>(t, color);
        }

        template<class T>
        void PrintInternal(T& t, const std::string color) {
            std::cout << GetTime() << color;
            _startLine = ResetLogColor() + "\n" + GetTime() + color;
            _state.push_back({false, true});
            Serializer::Serialize(t, PrettyNameOf<T>());
            std::cout << "\n" << ResetLogColor();
            std::cout.flush();
        }

        inline void StartClass(const size_t amountMembers, const std::string_view name="") override {
            StartVariable(name);
            _state.push_back({false, true});
            AddPadding();
            std::cout << "{" << _startLine;
        }
        inline void EndClass() override {
            _state.pop_back();
            RemovePadding();
            std::cout << _startLine << "}";
        }

        inline void StartPair(const std::string_view name="") override {
            StartVariable(name);
            _state.push_back({true, true});
            AddPadding();
            std::cout << "(" << _startLine;
        }
        inline void EndPair() override {
            _state.pop_back();
            RemovePadding();
            std::cout << _startLine << ")";
        }

        inline void StartArray(const size_t amountElements, const std::string_view name="") override {
            StartVariable(name);
            _state.push_back({true, true});
            AddPadding();
            std::cout << "[" << _startLine;
        }
        inline void EndArray() override {
            _state.pop_back();
            RemovePadding();
            std::cout << _startLine << "]";
        }

        inline void AddVariable(const int8_t v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }
        inline void AddVariable(const int16_t v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }
        inline void AddVariable(const int32_t v, const std::string_view name="") override {
            std::cout << std::to_string(v);
        }
        inline void AddVariable(const int64_t v, const std::string_view name="") override {
        StartVariable(name);
            std::cout << std::to_string(v);
        }

        inline void AddVariable(const uint8_t v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }
        inline void AddVariable(const uint16_t v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }
        inline void AddVariable(const uint32_t v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }
        inline void AddVariable(const uint64_t v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }

        inline void AddVariable(const bool v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::boolalpha << v << std::noboolalpha;
        }

        inline void AddVariable(const float v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }
        inline void AddVariable(const double v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << std::to_string(v);
        }

        inline void AddVariable(const std::string& v, const std::string_view name="") override {
            StartVariable(name);
            std::cout << '"' << v << '"';
        }
    private:
        struct State {
            bool _isArray = false; 
            bool _isFirstVar = true;
        };

        State& GetState() {
            return _state[_state.size() - 1];
        }
        inline void StartVariable(const std::string_view name) {
            if(!GetState()._isFirstVar) std::cout << "," << _startLine;
            if(!GetState()._isArray) std::cout << '"' << name << "\": ";
            GetState()._isFirstVar = false;
        }
        inline void AddPadding() {
            for(int i = 0; i < 3; i++) _startLine.push_back(' ');
        }
        inline void RemovePadding() {
            for(int i = 0; i < 3; i++) _startLine.pop_back();
        }

        std::string _startLine;// What to print at a newline
        std::vector<State> _state;
    };

    /**
     * @brief Will print an object to the terminal (with the LOG color)
     * 
     * @tparam T The object type
     * @param message The object to print
     */
    template<class T>
    void Log(T& message) {
        if constexpr (IsStringType<T>()) Log(std::string(message));
        else ClassPrinter::Print(message, SetLogColor());
    }


    /**
     * @brief Will print an object to the terminal (with the INFO color)
     * 
     * @tparam T The object type
     * @param message The object to print
     */
    template<class T>
    void Info(T& message) {
        if constexpr (IsStringType<T>()) Info(std::string(message));
        else ClassPrinter::Print(message, SetInfoColor());
    }
    
    /**
     * @brief Will print an object to the terminal (with the ERROR color)
     * 
     * @tparam T The object type
     * @param message The object to print
     */
    template<class T>
    void Error(T& message) {
        if constexpr (IsStringType<T>()) Error(std::string(message));
        else ClassPrinter::Print(message, SetErrorColor());
    }

    /**
     * @brief Will print an object to the terminal (with the THROW color)
     * 
     * @tparam T The object type
     * @param message The object to print
     */
    template<class T>
    void Throw(T& message) {
        if constexpr (IsStringType<T>()) Throw(std::string(message));
        else ClassPrinter::Print(message, SetThrowColor());
    }

}
}

#endif