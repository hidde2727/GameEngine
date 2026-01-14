#ifndef ENGINE_UTILS_FUNDAMENTALTYPE_H
#define ENGINE_UTILS_FUNDAMENTALTYPE_H

#include <concepts>

#include <tuple>
#include <map>
#include <vector>
#include <string>
#include <chrono>

#include "util/Reflection.h"

namespace Engine {
namespace Util {
	
	template<class T>
	concept IntegralType = std::is_integral<T>::value;
	template <typename T>
	concept FloatingPointType = std::floating_point<T>;

	template<class T>
	concept FundamentalType = IntegralType<T> || FloatingPointType<T>;
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

	template<class T, class SAME_AS>
	concept IsType = std::is_same<std::remove_cv_t<T>, SAME_AS>::value;

	template<class T, std::meta::info SAME_AS>
	concept IsTemplatedType = ([]() -> bool {
		if constexpr(!std::meta::has_template_arguments(^^T)) return false;
		else return std::meta::template_of(^^T) == SAME_AS;
	})();

	/// Only for std::queue, std::stack, and std::priority_queue this doesn't hold (at the time of writing)
	template<class T>
	concept IterableSTL = IsInNamespace<^^T, ^^std>() && requires (T& x) {
		{ x.begin() } -> std::input_or_output_iterator;
		{ x.end()   } -> std::sentinel_for<decltype(x.begin())>;
	};

	/// Only for std::array, std::queue, std::stack, std::forward_list and std::priority_queue this doesn't hold (at the time of writing)
	template<class T>
	concept InsertableSTL = IsInNamespace<^^T, ^^std>() && requires (T& x) {
		typename T::value_type;
		{ x.insert(x.end(), typename T::value_type()) };
	};

}
}

#endif