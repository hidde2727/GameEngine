#ifndef ENGINE_UTILS_FUNDAMENTALTYPE_H
#define ENGINE_UTILS_FUNDAMENTALTYPE_H

namespace Engine {
namespace Util {
    
	template<class T>
	concept FundamentalType = std::is_fundamental<T>::value;
	
	template<class T>
	concept IntegralType = std::is_integral<T>::value;

	template<class T, class U>
	concept Derived = std::is_base_of<U, T>::value;
}
}

#endif