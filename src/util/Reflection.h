#ifndef ENGINE_UTIL_REFLECTION_H
#define ENGINE_UTIL_REFLECTION_H

#ifdef __has_include
#  if __has_include(<meta>)
#    include <meta>
#  elif __has_include(<experimental/meta>)
#    include <experimental/meta>
#  else
#     error "Missing <meta>"
#  endif
#else
#     error "Missing __has_include"
#endif
#include <type_traits>
#include <memory>// std::point_traits

namespace Engine {
namespace Util {

    /**
     * @brief Check if the thing reflected by info has an annotation
     * Example annotation:
     * ```
     * constexpr struct AnnotationType {} annotation;
     * [[=annotation]] int _something;
     * ```
     * 
     * @tparam T The type the annotation must have
     * @param info The thing to reflect if it has an annotation
     * @return bool 
     */
    template<class T>
    consteval bool HasAnnotation(const std::meta::info info) {
        return !std::meta::annotations_of(info, ^^T).empty();
    }

    /**
     * @brief Returns true if T is either a std::string or a const char[]
     * 
     * @tparam T The type to check
     * @return True if T is either a std::string or a const char[] 
     */
    template<class T>
    consteval bool IsStringType() {
        if(std::meta::remove_cv(^^T) == ^^std::string) return true;
        if(!std::meta::is_array_type(^^T)) return false;
        auto elementType = std::meta::remove_extent(^^T);
        if(!((elementType == ^^char) && std::meta::is_const(elementType))) return false;
        // return std::is_string_literal(^^T);
        return true;
    }

    /**
     * @brief Checks if INFO is the global namespace
     * 
     * @tparam INFO 
     * @return bool 
     */
    template<std::meta::info INFO>
    consteval bool IsGlobalNamespace() {
        if constexpr(!std::meta::is_namespace(INFO)) return false;
        else return INFO == ^^::;
    }

    /**
     * @brief Convert a string to the equally named value in an enum.
     * 
     * @param name The name to find inside the Enum
     * @tparam Enum The enum type to try and find the value
     */
    template<class Enum> requires std::is_enum_v<Enum>
    constexpr std::optional<Enum> StringToEnum(const std::string_view name) {
        static_assert(std::meta::is_enumerable_type(^^Enum), "[Util::StringToEnum] Cannot use StringToEnum on an enum that is not enumerable");
        template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^Enum)))
            if (name.compare(std::meta::identifier_of(e)))
                return [: e :];
        return std::nullopt;
    }

    /**
     * @brief COnvert an enum to a string
     * 
     * @param value To be converted
     * @tparam Enum The enum to use to convert the value to a stringz
     */
    template<class Enum> requires std::is_enum_v<Enum>
    constexpr std::string_view EnumToString(const Enum value) {
        static_assert(std::meta::is_enumerable_type(^^Enum), "[Util::StringToEnum] Cannot use EnumToString on an enum that is not enumerable");
        template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^Enum)))
            if (value == [: e :])
                return std::meta::identifier_of(e);
        return "[Util::EnumToString] Illegal enum value, could not find it inside the enum";
    }

    /**
     * @brief Compares two objects to see if they represent the same namespace
     * Just calling == on the std::meta::info with two namespace retrieved with for example ^^std and std::meta::parent_of(^^std::vector) will result
     * in a false negative.
     * This function compares the identifier of all the parents of the namespace to see if they are the same
     * 
     * @tparam T 
     * @tparam SAME_AS 
     * @return bool 
     */
    template<std::meta::info T, std::meta::info SAME_AS>
    consteval bool IsSameNamespace() {
        if constexpr (!std::meta::is_namespace(T) || !std::meta::is_namespace(SAME_AS)) return false;
        else if constexpr (IsGlobalNamespace<T>() && IsGlobalNamespace<SAME_AS>()) return true;
        else if constexpr (IsGlobalNamespace<T>() || IsGlobalNamespace<SAME_AS>()) return false;
        else if constexpr (std::meta::identifier_of(T) != std::meta::identifier_of(SAME_AS)) return false;
        else return IsSameNamespace<std::meta::parent_of(T), std::meta::parent_of(SAME_AS)>();
    }

    /**
     * @brief Check if NAMESPACE is a parent of T
     * 
     * @tparam T The type to check if it is part of NAMESPACE
     * @tparam NAMESPACE 
     * @return bool
     */
    template<std::meta::info T, std::meta::info NAMESPACE>
    consteval bool IsInNamespace() {
        if constexpr (std::meta::is_type(T)) {
            if constexpr (std::meta::has_template_arguments(T)) return IsInNamespace<std::meta::template_of(T), NAMESPACE>();
            else if constexpr (std::meta::is_array_type(T)) return IsInNamespace<std::meta::remove_extent(T), NAMESPACE>();
            else if constexpr (std::is_integral_v<typename [: T :]>) return false;
            else if constexpr (std::is_floating_point_v<typename [: T :]>) return false;
            else if constexpr (std::is_pointer_v<typename [: T :]>) return IsInNamespace<^^std::pointer_traits<typename [: T :]>::element_type, NAMESPACE>();
            else return IsInNamespace<std::meta::parent_of(T), NAMESPACE>();
        } else {
            if constexpr (IsGlobalNamespace<T>()) return IsSameNamespace<T, NAMESPACE>();
            else if constexpr (IsSameNamespace<T, NAMESPACE>()) return true;
            else return IsInNamespace<std::meta::parent_of(T), NAMESPACE>();
        }
    }


    /**
     * @brief A function to get a nicely formatted name of a type that includes all its namespaces
     * example formatting: Namespace1::Namespace2::ClassType
     * 
     * @param info The type info
     * @return The identifier
     */
    template<std::meta::info INFO, bool IgnoreTemplateArgs=false>
    std::string PrettyNameOfImpl() {
        if constexpr (IsGlobalNamespace<INFO>()) return "";
        else if constexpr (std::meta::is_namespace(INFO) || std::meta::is_template(INFO)) {
            if constexpr (!IsGlobalNamespace<std::meta::parent_of(INFO)>()) {
                return PrettyNameOfImpl<std::meta::parent_of(INFO)>() + "::" + std::string(std::meta::display_string_of(INFO));
            } else {
                return std::string(std::meta::display_string_of(INFO));
            }
        }
        else if constexpr (!std::meta::is_type(INFO)) return "Do not use PrettyNameOfImpl with std::meta::info that aren't types";
        else if constexpr (std::is_integral_v<typename [: INFO :]>) return std::string(std::meta::display_string_of(INFO));
        else if constexpr (std::is_floating_point_v<typename [: INFO :]>) return std::string(std::meta::display_string_of(INFO));
        else if constexpr (std::is_pointer_v<typename [: INFO :]>) return PrettyNameOfImpl<^^std::pointer_traits<typename [: INFO :]>::element_type>() + "*";
        else if constexpr (std::meta::is_array_type(INFO)) {
            return PrettyNameOfImpl<std::meta::remove_extent(INFO)>() + "[]";
        }
        else if constexpr (IgnoreTemplateArgs && std::meta::has_template_arguments(INFO)) {
            return PrettyNameOfImpl<std::meta::template_of(INFO)>();
        }
        else if constexpr (!IgnoreTemplateArgs && std::meta::has_template_arguments(INFO)) {
            std::string ret = PrettyNameOfImpl<std::meta::template_of(INFO)>() + "<";
            bool isFirst = true;
            constexpr auto templateArgs = std::define_static_array(std::meta::template_arguments_of(INFO));
            template for(constexpr std::meta::info templateType : templateArgs) {
                if(!isFirst) ret += ", ";
                isFirst = false;
                ret += std::string(std::meta::display_string_of(templateType));
            }
            ret += ">";
            return ret;
        }
        else if constexpr (!IsGlobalNamespace<std::meta::parent_of(INFO)>()) {
            return PrettyNameOfImpl<std::meta::parent_of(INFO)>() + "::" + std::string(std::meta::display_string_of(INFO));
        }
        return std::string(std::meta::display_string_of(INFO));
    }
    template<class T>
    std::string PrettyNameOf() {
        return PrettyNameOfImpl<^^T>();
    }
}
}

#endif