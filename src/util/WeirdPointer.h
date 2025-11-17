#ifndef ENGINE_UTIL_WEAKPOINTER_H
#define ENGINE_UTIL_WEAKPOINTER_H

#include "core/PCH.h"

namespace Engine {
namespace Util {

    /**
     * This is a class that is either a std::shared_ptr (owning), or a normal pointer (non-owning).
     * Usefull for features to give a user both options (shared or normal), but not make the code more difficult to write.
     * 
     * AKA SchrodingersPointer.
     * 
     * @warning When the user decides to use a normal pointer, it should be kept alive for the same time WeirdPointer is alive.
     */
    template<class T>
    class WeirdPointer {
    public:

        WeirdPointer() {
            _pointer = nullptr;
        }
        WeirdPointer(const std::shared_ptr<T> pointer) {
            _pointer = pointer;
        }
        /// @warning When using a normal pointer, it should be kept alive until this WeirdPointer is destroyed.
        WeirdPointer(T* pointer) {
            _pointer = pointer;
        }

        inline T& operator=(const std::shared_ptr<T> pointer) {
            _pointer = pointer;
            return *Get();
        }
        /// @warning When using a normal pointer, it should be kept alive until this WeirdPointer is destroyed.
        inline T& operator=(T* pointer) {
            _pointer = pointer;
            return *Get();
        }
        inline T* Get() const {
            if(std::holds_alternative<T*>(_pointer)) return std::get<T*>(_pointer);
            else return std::get<std::shared_ptr<T>>(_pointer).get();
        }
        inline T& operator*() const {
            return *Get();
        }
        inline T* operator->() const {
            return Get();
        }
		inline operator T*() const {
			return Get();
		}
        template<class S>
        inline operator WeirdPointer<S>() {
            if(std::holds_alternative<T*>(_pointer)) {
                T* pointer = std::get<T*>(_pointer);
                return static_cast<S*>(pointer);
            }
            else {
                std::shared_ptr<T> pointer = std::get<std::shared_ptr<T>>(_pointer);
                return std::static_pointer_cast<S>(pointer);
            }
        }


    private:
        std::variant<T*, std::shared_ptr<T>> _pointer;
    };

    /**
     * Void specialization.
     * 
     * This is a class that is either a std::shared_ptr (owning), or a normal pointer (non-owning).
     * Usefull for features to give a user both options (shared or normal), but not make the code more difficult to write.
     * 
     * AKA SchrodingersPointer.
     * 
     * @warning When the user decides to use a normal pointer, it should be kept alive for the same time WeirdPointer is alive.
     */
    template<>
    class WeirdPointer<void> {
    public:

        WeirdPointer() {
            _pointer = nullptr;
        }
        WeirdPointer(const std::shared_ptr<void> pointer) {
            _pointer = pointer;
        }
        /// @warning When using a normal pointer, it should be kept alive until this WeirdPointer is destroyed.
        WeirdPointer(void* pointer) {
            _pointer = pointer;
        }

        inline void operator=(const std::shared_ptr<void> pointer) {
            _pointer = pointer;
        }
        /// @warning When using a normal pointer, it should be kept alive until this WeirdPointer is destroyed.
        inline void operator=(void* pointer) {
            _pointer = pointer;
        }
        inline void* Get() const {
            if(std::holds_alternative<void*>(_pointer)) return std::get<void*>(_pointer);
            else return std::get<std::shared_ptr<void>>(_pointer).get();
        }
        inline void* operator->() const {
            return Get();
        }
		inline operator void*() const {
			return Get();
		}
        template<class S>
        inline operator WeirdPointer<S>() {
            if(std::holds_alternative<void*>(_pointer)) {
                void* pointer = std::get<void*>(_pointer);
                return static_cast<S*>(pointer);
            }
            else {
                std::shared_ptr<void> pointer = std::get<std::shared_ptr<void>>(_pointer);
                return std::static_pointer_cast<S>(pointer);
            }
        }


    private:
        std::variant<void*, std::shared_ptr<void>> _pointer;
    };

}
}

#endif