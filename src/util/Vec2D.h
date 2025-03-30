#ifndef ENGINE_UTILS_VEC2D_H
#define ENGINE_UTILS_VEC2D_H

#include "core/PCH.h"
#include "util/FundamentalType.h"

namespace Engine {
namespace Utils {

    template<FundamentalType T>
	class Vec2 {
	public:
		Vec2() {}
		Vec2(const T value) : x(value), y(value) {}
		Vec2(const T x, const T y) : x(x), y(y) {}
		Vec2(const Vec2<T>& v) : x(v.x), y(v.y) {}
		Vec2(Vec2<T>&& v) : x(v.x), y(v.y) {}

		T x = 0;
		T y = 0;

		inline Vec2<T> normalized() {
			T length = std::sqrt(x * x + y * y);
			if (length == 0) {
				return Vec2<T>((T)x, (T)y);
			}
			return Vec2<T>((T)x / length, (T)y / length);
		}

		inline T crossProduct(Vec2<T>& v) {
			return this->x * v.y - this->y * v.x;
		}

		inline Vec2<T> rotated90Degree() {
			return Vec2<T>(y, -x);
		}

		inline Vec2<T> rotatedNegative90Degree() {
			return Vec2<T>(-y, x);
		}

		inline Vec2<T>& operator=(const Vec2<T>& v) {
			this->x = v.x;
			this->y = v.y;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec2<T>& operator=(const Vec2<OT>& v) {
			this->x = (T)v.x;
			this->y = (T)v.y;
			return *this;
		}

		inline Vec2<T>& operator=(Vec2<T>&& v) {
			this->x = v.x;
			this->y = v.y;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec2<T>& operator=(Vec2<OT>&& v) {
			this->x = (T)v.x;
			this->y = (T)v.y;
			return *this;
		}

		inline Vec2<T>& operator+=(const Vec2<T>& v) {
			this->x += v.x;
			this->y += v.y;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec2<T>& operator+=(const Vec2<OT>& v) {
			this->x += (T)v.x;
			this->y += (T)v.y;
			return *this;
		}

		inline Vec2<T>& operator-=(const Vec2<T>& v) {
			this->x -= v.x;
			this->y -= v.y;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec2<T>& operator-=(const Vec2<OT>& v) {
			this->x -= (T)v.x;
			this->y -= (T)v.y;
			return *this;
		}

		inline Vec2<T> operator+(const Vec2<T>& v) const {
			return Vec2<T>(this->x + v.x, this->y + v.y);
		}

		inline Vec2<T> operator-(const Vec2<T>& v) const {
			return Vec2<T>(this->x - v.x, this->y - v.y);
		}

		template<FundamentalType OT>
		inline Vec2<T> operator*(const OT v) const {
			return Vec2<T>(this->x * v, this->y * v);
		}

		inline T operator*(const Vec2<T>& v) const {
			return (this->x * v.x) + (this->y * v.y);
		}

		
		inline bool operator==(const Vec2<T> v) const {
			return this->x == v.x && this->y == v.y;
		}

		inline bool operator!=(const Vec2<T> v) const {
			return !this->operator==(v);
		}

		// Calculates the distance between the 2 poins
		inline double distance(const Vec2<T> v) const {
			return std::sqrt((this->x - v.x) * (this->x - v.x) + (this->y - v.y) * (this->y - v.y));
		}

		template<FundamentalType OT>
		inline operator Vec2<OT>() const {
			return Vec2<OT>((OT)x, (OT)y);
		}

	};

	typedef Vec2<double> Vec2D;
	typedef Vec2<int> Vec2I;
	typedef Vec2<int8_t> Vec2I8;
	typedef Vec2<int16_t> Vec2I16;
	typedef Vec2<int32_t> Vec2I32;
	typedef Vec2<uint8_t> Vec2U8;
	typedef Vec2<uint16_t> Vec2U16;
	typedef Vec2<uint32_t> Vec2U32;
	typedef Vec2<float> Vec2F;
	typedef Vec2<long> Vec2L;

}
}

#endif