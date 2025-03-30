#ifndef ENGINE_UTILS_AREA_H
#define ENGINE_UTILS_AREA_H

#include "core/PCH.h"
#include "util/FundamentalType.h"

namespace Engine {
namespace Utils {

    template<FundamentalType T>
	class Area {
	public:
		Area() {}
		Area(const T value) : x(value), y(value), w(value), h(value) {}
		Area(const T x, const T y, const T w, const T h) : x(x), y(y), w(w), h(h) {}
		Area(const Area<T>& a) : x(a.x), y(a.y), w(a.w), h(a.h) {}
		Area(Area<T>&& a) : x(a.x), y(a.y), w(a.w), h(a.h) {}

		T x = 0;
		T y = 0;
        T w = 0;
		T h = 0;

		inline Area<T>& operator=(const Area<T>& a) {
			this->x = a.x;
			this->y = a.y;
            this->w = a.w;
            this->h = a.h;
			return *this;
		}

		template<FundamentalType OT>
		inline Area<T>& operator=(const Area<OT>& a) {
			this->x = (T)a.x;
			this->y = (T)a.y;
            this->w = (T)a.w;
            this->h = (T)a.h;
			return *this;
		}

		inline Area<T>& operator=(Area<T>&& a) {
			this->x = a.x;
			this->y = a.y;
            this->w = a.w;
            this->h = a.h;
			return *this;
		}

		template<FundamentalType OT>
		inline Area<T>& operator=(Area<OT>&& a) {
			this->x = (T)a.x;
			this->y = (T)a.y;
            this->w = (T)a.w;
            this->h = (T)a.h;
			return *this;
		}

		inline Area<T>& operator+=(const Area<T>& a) {
			this->x += a.x;
			this->y += a.y;
            this->w += a.w;
            this->h += a.h;
			return *this;
		}

		template<FundamentalType OT>
		inline Area<T>& operator+=(const Area<OT>& a) {
			this->x += (T)a.x;
			this->y += (T)a.y;
            this->w += (T)a.w;
            this->h += (T)a.h;
			return *this;
		}

		inline Area<T>& operator-=(const Area<T>& a) {
			this->x -= a.x;
			this->y -= a.y;
            this->w -= a.w;
            this->h -= a.h;
			return *this;
		}

		template<FundamentalType OT>
		inline Area<T>& operator-=(const Area<OT>& a) {
			this->x -= (T)a.x;
			this->y -= (T)a.y;
            this->w -= (T)a.w;
            this->h -= (T)a.h;
			return *this;
		}

		inline Area<T> operator+(const Area<T>& a) const {
			return Area<T>(this->x + a.x, this->y + a.y, this->w + a.w, this->h + a.h);
		}

		inline Area<T> operator-(const Area<T>& a) const {
			return Area<T>(this->x - a.x, this->y - a.y, this->w - a.w, this->h - a.h);
		}

		template<FundamentalType OT>
		inline Area<T> operator*(const OT a) const {
			return Area<T>(this->x * a, this->y * a, this->w * a, this.h * a);
		}

		
		inline bool operator==(const Area<T> a) const {
			return this->x == a.x && this->y == a.y && this->w == a.w && this->h == a.h;
		}

		inline bool operator!=(const Area<T> a) const {
			return !this->operator==(a);
		}

		// Calculates the distance between the 2 poins
		inline double Distance(const Area<T> a) const {
			return std::sqrt((this->x - a.x) * (this->x - a.x) + (this->y - a.y) * (this->y - a.y));
		}

		template<FundamentalType OT>
		inline operator Area<OT>() const {
			return Area<OT>((OT)x, (OT)y, (OT)h, (OT)h);
		}

	};

	typedef Area<double> AreaD;
	typedef Area<int> AreaI;
	typedef Area<int8_t> AreaI8;
	typedef Area<int16_t> AreaI16;
	typedef Area<int32_t> VAreaI32;
	typedef Area<uint8_t> AreaU8;
	typedef Area<uint16_t> AreaU16;
	typedef Area<uint32_t> AreaU32;
	typedef Area<float> AreaF;
	typedef Area<long> AreaL;

}
}

#endif