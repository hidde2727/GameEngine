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

		template<FundamentalType OT>
		inline T cross(const Vec2<OT> v) const {
			return this->x * (T)v.y - this->y * (T)v.x;
		}

		template<FundamentalType OT>
		inline T dot(const Vec2<OT> v) const {
			return (this->x * v.x) + (this->y * v.y);
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
			ASSERT(x!=0||y!=0, "[Util::Vec2] angleToY() called on zero vector");
			if(x==0) { return (y<0)? (T)1.5*(T)PI : (T)0.5*(T)PI;}
			if(x<0) return (T)atan(y/x);
			else return (T)atan(y/x) + (T)1.0*(T)PI;
		}
		inline T angleToX() const {
			ASSERT(x!=0||y!=0, "[Util::Vec2] angleToX() called on zero vector");
			if(x==0) { return (y<0)? (T)PI : (T)0;}
			if(x<0) return (T)atan(y/x) + (T)1.5*(T)PI;
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
		
		template<FundamentalType OT>
		inline Vec2<T>& operator*=(const OT v) {
			this->x *= (T)v;
			this->y *= (T)v;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec2<T>& operator/=(const OT v) {
			this->x /= (T)v;
			this->y /= (T)v;
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

		template<FundamentalType OT>
		inline Vec2<T> operator/(const OT v) const {
			return Vec2<T>(this->x / (T)v, this->y / (T)v);
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
		inline operator std::string() const {
			return "{ x: " + std::to_string(x) + ", y: " + std::to_string(y) + " }";
		}

	};

	template<class T>
	inline Vec2<T> Cross(const Vec2<T> v, const T a) {
		return Vec2( a * v.y, -a * v.x );
	}
	template<class T>
	inline Vec2<T> Cross(const T a, const Vec2<T> v) {
		return Vec2( -a * v.y, a * v.x );
	}
	template<class T>
	inline Vec2<T> Cross(const Vec2<T> a, const Vec2<T> b) {
		return a.cross(b);
	}

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