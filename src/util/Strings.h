#ifndef ENGINE_UTIL_STRINGS_H
#define ENGINE_UTIL_STRINGS_H

#include <string>
#include <cstring>

namespace Engine {
namespace Util {
    /**
     * @brief std::toupper()
     */
    inline char ToUpper(const char in) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(in)));
    }
    /**
     * @brief ToUpper but without locale support (and is constexpr)
     */
    constexpr char ToUpperConst(const char in) {
        if(in >= 'a' && in <= 'z') return in - 32;
        else return in;
    }
    /**
     * @brief std::tolower()
     */
    inline char ToLower(const char in) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(in)));
    }
    /**
     * @brief ToLLower but without locale support (and is constexpr)
     */
    constexpr char ToLowerConst(const char in) {
        if(in >= 'A' && in <= 'Z') return in + 32;
        else return in;
    }

    /**
     * @brief Returns if the character is already in upper case
     * Checks if ToUpper changes the character
     */
    inline bool IsUpper(const char in) {
        char upper = ToUpper(in);
        return upper == in;
    }
    /**
     * @brief Same as IsUpper, but doesn't use the locale and is constexpr
     * Checks if ToUpperConst changes the character
     */
    constexpr bool IsUpperConst(const char in) {
        char upper = ToUpperConst(in);
        return upper == in;
    }
    /**
     * @brief Returns if the character is already in lower case
     * Checks if ToLower changes the character
     */
    inline bool IsLower(const char in) {
        char lower = ToLower(in);
        return lower == in;
    }
    /**
     * @brief Same as IsLower, but doesn't use the locale and is constexpr
     * Checks if ToLowerConst changes the character
     */
    constexpr bool IsLowerConst(const char in) {
        char lower = ToLowerConst(in);
        return lower == in;
    }

    /**
     * @brief Convert a string in snake case to camel case
     * Keeps the upper/lower case, only changes the characters located directly after an _
     */
    inline std::string SnakeCaseToCamelCase(const std::string_view in) {
        if(in.size() == 0) return "";
        std::string output;
        output.reserve(in.size());

        bool upperCaseNextCharacter = false;
        bool firstCharacter = false;
        for(size_t i = 1; i < in.size(); i++) {
            if(in[i] == '_') upperCaseNextCharacter = true;
            else {
                if(firstCharacter && upperCaseNextCharacter) output.push_back(ToUpper(in[i]));
                else output.push_back(in[i]);
                firstCharacter = true;
                upperCaseNextCharacter = false;
            }
        }

        return output;
    }

    /**
     * @brief Convert a string in camel case to snake case
     */
    inline std::string CamelCaseToSnakeCase(const std::string_view in) {
        if(in.size() == 0) return "";
        std::string output;
        output.reserve(in.size());

        for(const char c : in) {
            if(IsUpper(c)) output.push_back('_');
            output.push_back(ToLower(c));
        }

        return output;
    }

    /**
     * @brief Convert a string in snake case to pascal case
     * Keeps the upper/lower case, only changes the characters located directly after an _
     */
    inline std::string SnakeCaseToPascalCase(const std::string_view in) {
        std::string out = SnakeCaseToCamelCase(in);
        out[0] = ToUpper(out[0]);
        return out;
    }

    /**
     * @brief Convert a string in pascal case to snake case
     */
    inline std::string PascalCaseToSnakeCase(const std::string_view in) {
        return CamelCaseToSnakeCase(in);
    }

}
}

#endif