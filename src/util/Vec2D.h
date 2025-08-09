#ifndef ENGINE_UTILS_VEC2D_H
#define ENGINE_UTILS_VEC2D_H

#include "core/PCH.h"
#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

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

		inline T length() const {
			return std::sqrt(x * x + y* y);
		}
		inline Vec2<T> normalized() const {
			T len = length();
			if (len == 0) {
				return Vec2<T>((T)x, (T)y);
			}
			return Vec2<T>((T)x / len, (T)y / len);
		}
		
		inline T crossProduct(const Vec2<T> v) const {
			return this->x * v.y - this->y * v.x;
		}
		inline T dotProduct(const Vec2<T> v) const {
			return x*(T)v.x+y*(T)v.y;
		}

		inline Vec2<T> rotatedR() const {
			return Vec2<T>(y, -x);
		}
		inline Vec2<T> rotated90Degree() const {
			return Vec2<T>(y, -x);
		}

		inline Vec2<T> rotatedL() const {
			return Vec2<T>(-y, x);
		}
		inline Vec2<T> rotatedNegative90Degree() const {
			return Vec2<T>(-y, x);
		}
		inline Vec2<T> rotate(const float angle) const {
			return Vec2<T>(
				x*cos(angle) - y*sin(angle),
				x*sin(angle) + y*cos(angle)
			);
		}
		inline Vec2<T> rotate(const float angle, const Vec2<T> aroundPoint) const {
			return Vec2<T>(
				(x-aroundPoint.x)*cos(angle) - (y-aroundPoint.y)*sin(angle) + aroundPoint.x,
				(x-aroundPoint.x)*sin(angle) + (y-aroundPoint.y)*cos(angle) + aroundPoint.y
			);
			// 1. Translate to make aroundPoint 0,0
			// 2. Rotate the points
			// 3. Undo step 1 (translate back)
		}

		inline Vec2<T> rotated180Degree() const {
			return Vec2<T>(x*-1, y*-1);
		}

		inline T angleToY() const {
			ASSERT(x==0&&y==0, "Cannot determine the angle of a vector that is x=0&y=0");
			if(x<0) return (T)atan(y/x) + (T)2.0*(T)PI;
			else if(y<0) return (T)atan(y/x) + (T)3.0*(T)PI;
			else return (T)atan(y/x) + (T)1.0*(T)PI;
		}
		inline T angleToX() const {
			ASSERT(x==0&&y==0, "Cannot determine the angle of a vector that is x=0&y=0");
			if(x<0) return (T)atan(y/x) + (T)1.5*(T)PI;
			else if(y<0) return (T)atan(y/x) + (T)2.5*(T)PI;
			else return (T)atan(y/x) + (T)0.5*(T)PI;
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
		\
		template<FundamentalType OT>
		inline Vec2<T>& operator*=(const OT v) {
			this->x *= (T)v;
			this->y *= (T)v;
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
			return Vec2<T>(this->x * (T)v, this->y * (T)v);
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