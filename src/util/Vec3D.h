#ifndef ENGINE_UTILS_VEC3D_H
#define ENGINE_UTILS_VEC3D_H

#include "core/PCH.h"
#include "util/FundamentalType.h"

namespace Engine {
namespace Utils {

    template<FundamentalType T>
	class Vec3 {
	public:
		Vec3() {}
		Vec3(const T value) : x(value), y(value), z(value) {}
		Vec3(const T x, const T y, const T z) : x(x), y(y), z(z) {}
		Vec3(const Vec3<T>& v) : x(v.x), y(v.y), z(v.z) {}
		Vec3(Vec3<T>&& v) : x(v.x), y(v.y), z(v.z) {}

		T x = 0;
		T y = 0;
        T z = 0;

		inline Vec3<T> Normalized() {
			T length = std::sqrt(x * x + y * y + z * z);
			if (length == 0) {
				return Vec3<T>((T)x, (T)y, (T)z);
			}
			return Vec3<T>((T)x / length, (T)y / length, (T)z / length);
		}

		inline Vec3<T> CrossProduct(Vec3<T>& v) {
			return Vec3<T>(
                this->y * v.z - this->z * v.y,
                this->z * v.x - this->x * v.z,
                this->x * v.y - this->y * v.x
            );
		}

		inline Vec3<T>& operator=(const Vec3<T>& v) {
			this->x = v.x;
			this->y = v.y;
            this->z = v.z;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec3<T>& operator=(const Vec3<OT>& v) {
			this->x = (T)v.x;
			this->y = (T)v.y;
            this->z = (T)v.z;
			return *this;
		}

		inline Vec3<T>& operator=(Vec3<T>&& v) {
			this->x = v.x;
			this->y = v.y;
            this->z = v.z;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec3<T>& operator=(Vec3<OT>&& v) {
			this->x = (T)v.x;
			this->y = (T)v.y;
            this->z = (T)v.z;
			return *this;
		}

		inline Vec3<T>& operator+=(const Vec3<T>& v) {
			this->x += v.x;
			this->y += v.y;
            this->z += v.z;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec3<T>& operator+=(const Vec3<OT>& v) {
			this->x += (T)v.x;
			this->y += (T)v.y;
            this->z += (T)v.z;
			return *this;
		}

		inline Vec3<T>& operator-=(const Vec3<T>& v) {
			this->x -= v.x;
			this->y -= v.y;
            this->z -= v.z;
			return *this;
		}

		template<FundamentalType OT>
		inline Vec3<T>& operator-=(const Vec3<OT>& v) {
			this->x -= (T)v.x;
			this->y -= (T)v.y;
            this->z -= (T)v.z;
			return *this;
		}

		inline Vec3<T> operator+(const Vec3<T>& v) const {
			return Vec3<T>(this->x + v.x, this->y + v.y, this->z + v.z);
		}

		inline Vec3<T> operator-(const Vec3<T>& v) const {
			return Vec3<T>(this->x - v.x, this->y - v.y, this->z - v.z);
		}

		template<FundamentalType OT>
		inline Vec3<T> operator*(const OT v) const {
			return Vec3<T>(this->x * v, this->y * v, this->z * v);
		}

		
		inline bool operator==(const Vec3<T> v) const {
			return this->x == v.x && this->y == v.y && this->z == v.z;
		}

		inline bool operator!=(const Vec3<T> v) const {
			return !this->operator==(v);
		}

		// Calculates the distance between the 2 poins
		inline double Distance(const Vec3<T> v) const {
			return std::sqrt((this->x - v.x) * (this->x - v.x) + (this->y - v.y) * (this->y - v.y) + (this->z - v.z) * (this->z - v.z));
		}

		template<FundamentalType OT>
		inline operator Vec3<OT>() const {
			return Vec3<OT>((OT)x, (OT)y, (OT)z);
		}

	};

	typedef Vec3<double> Vec3D;
	typedef Vec3<int> Vec3I;
	typedef Vec3<int16_t> Vec3I16;
	typedef Vec3<int32_t> Vec3I32;
	typedef Vec3<float> Vec3F;
	typedef Vec3<long> Vec3L;

}
}

#endif