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

    template<std::meta::info INFO>
    consteval bool IsGlobalNamespace() {
        if constexpr(!std::meta::is_namespace(INFO)) return false;
        else return INFO == ^^::;
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
    // template<class T, bool IgnoreTemplateArgs=true>
    // std::string PrettyNameOf() {
    //     if constexpr (!std::meta::is_namespace_member(^^T)) return std::string(std::meta::display_string_of(^^T));
    //     // else if constexpr (!std::meta::is_complete_type(^^T)) return std::string(std::meta::display_string_of(^^T));
    //     else if constexpr (std::meta::is_array_type(^^T)) return PrettyNameOf<typename [: std::meta::remove_extent(^^T) :]>() + "[]";
    //     else if constexpr (std::meta::has_template_arguments(^^T)) {
    //         constexpr auto withoutTemplate = std::meta::template_of(^^T);
    //         // return PrettyNameOf<typename [: withoutTemplate :]>();
    //         return std::string(std::meta::display_string_of(withoutTemplate)); 
    //     }
    //     // else if constexpr (!IgnoreTemplateArgs && std::meta::has_template_arguments(^^T)) {
    //     //     std::string ret = PrettyNameOf<typename [: std::meta::template_of(^^T) :]>() + "<";
    //     //     bool isFirst = true;
    //     //     template for(constexpr std::meta::info templateType : std::meta::template_arguments_of(^^T)) {
    //     //         if(!isFirst) ret += ", ";
    //     //         isFirst = false;
    //     //         ret += PrettyNameOf<typename [: templateType :]>();
    //     //     }
    //     //     ret += ">";
    //     //     return ret;
    //     // }
    //     else return PrettyNameOf<typename [: std::meta::parent_of(^^T) :]>() + "::" + std::string(std::meta::display_string_of(^^T));
    // }

}
}

#endif