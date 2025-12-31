#ifndef ENGINE_UTILS_FUNDAMENTALTYPE_H
#define ENGINE_UTILS_FUNDAMENTALTYPE_H

#include <concepts>

#include <tuple>
#include <map>
#include <vector>
#include <string>
#include <chrono>

namespace Engine {
namespace Util {
	
	template<class T>
	concept IntegralType = std::is_integral<T>::value;
	template <typename T>
	concept FloatPointType = std::floating_point<T>;

	template<class T>
	concept FundamentalType = IntegralType<T> || FloatPointType<T>;
	template<class T>
	concept NotFundamentalType = !FundamentalType<T>;

	template<class T>
	concept PointerType = std::is_pointer<T>::value;
	template<class T>
	concept NotPointerType = !std::is_pointer<T>::value;

	template<class T>
	concept ArrayType = std::is_array<T>::value;
	template<class T>
	concept NotArrayType = !std::is_array<T>::value;

	template<class T, class U>
	concept Derived = std::is_base_of<U, std::remove_cv_t<T>>::value;

	template<class T>
	concept ClassType = NotFundamentalType<T> && NotPointerType<T> && NotArrayType<T>;

	template<class T>
	concept ContinuosStorageType = std::ranges::contiguous_range<T>;

	template<template<class ...> class, class ...T>
	struct IsTemplateType : std::false_type {};
	template<template<class ...> class Container, class ...T>
	struct IsTemplateType<Container, Container<T...>> : std::true_type {};

	template<class T>
	concept StdVectorType = IsTemplateType<std::vector, std::remove_cv_t<T>>::value;
	template<class T>
	concept StdMapType = IsTemplateType<std::map, std::remove_cv_t<T>>::value;
	template<class T>
	concept StdStringType = std::is_same<std::remove_cv_t<T>, std::string>::value;
	template<class T>
	concept StdTimePoint = IsTemplateType<std::chrono::time_point, std::remove_cv_t<T>>::value;
	template<class T>
	concept StdSharedPtr = IsTemplateType<std::shared_ptr, std::remove_cv_t<T>>::value;

}
}

#endif