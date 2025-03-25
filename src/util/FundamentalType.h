#ifndef ENGINE_UTILS_FUNDAMENTALTYPE_H
#define ENGINE_UTILS_FUNDAMENTALTYPE_H

namespace Engine {
namespace Utils {
    
	template<class T>
	concept FundamentalType = std::is_fundamental<T>::value;

}
}

#endif