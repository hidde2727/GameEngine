#ifndef ENGINE_UTIL_MATRIX_H
#define ENGINE_UTIL_MATRIX_H

#include <array>
#include <vector>
#include <algorithm>

#include "util/TemplateConcepts.h"

namespace Engine {
namespace Util {

    // Heap version
    template<FundamentalType T, unsigned int Rows, unsigned int Cols>
    class Matrix {
        static constexpr bool IS_BIG_MATRIX = Rows*Cols > 100;
    public:
        /**
         * @brief Construct a new Matrix object
         * @warning Does not initialize the matrix, the matrix will be random
         */
        Matrix() {
            if constexpr(IS_BIG_MATRIX) _matrix.resize(Rows*Cols);
        }
        /**
         * @brief Construct a new Matrix object
         * @param initializer Sets all entries in the matrix to this number
         */
        Matrix(const T initializer) {
            if constexpr(IS_BIG_MATRIX) _matrix.resize(Rows*Cols);
            std::fill(_matrix.begin(), _matrix.end(), initializer);
        }
        /**
         * @brief Construct a new Matrix object
         * @param m Array
         */
        Matrix(std::array<T, Rows*Cols>& m) {
            if constexpr(IS_BIG_MATRIX) 
                _matrix = std::move(std::vector<T>(std::make_move_iterator(m.begin()), std::make_move_iterator(m.end())));
            else
                _matrix = std::move(m);
        }
        /**
         * @brief Construct a new Matrix object
         * @param m Copies this matrix
         */
        template<FundamentalType OT>
        Matrix(const Matrix<OT, Rows, Cols>& m) {
            _matrix.resize(Rows*Cols);
			for(size_t i = 0; i < Rows*Cols; i++) {
                _matrix[i] = (T)m[i];
            }
        }
        /**
         * @brief Construct a new Matrix object
         * @param m Yanks this matrix
         */
        Matrix(Matrix<T, Rows, Cols>&& m) {
            _matrix = std::move(m._matrix);
        }

        
		inline Matrix<T, Rows, Cols>& operator=(const Matrix<T, Rows, Cols>& v) {
			_matrix = v._matrix;
			return *this;
		}

		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols>& operator=(const Matrix<OT, Rows, Cols>& v) {
			for(size_t i = 0; i < Rows*Cols; i++) {
                _matrix[i] = (T)v[i];
            }
			return *this;
		}

		inline Matrix<T, Rows, Cols>& operator=(Matrix<T, Rows, Cols>&& v) {
			_matrix = std::move(v._matrix);
			return *this;
		}

		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols>& operator=(Matrix<OT, Rows, Cols>&& v) {
			for(size_t i = 0; i < Rows*Cols; i++) {
                _matrix[i] = (T)v[i];
            }
			return *this;
		}

		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols>& operator+=(const Matrix<OT, Rows, Cols>& v) {
			for(size_t i = 0; i < Rows*Cols; i++) {
                _matrix[i] += (T)v[i];
            }
			return *this;
		}

		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols>& operator-=(const Matrix<OT, Rows, Cols>& v) {
			for(size_t i = 0; i < Rows*Cols; i++) {
                _matrix[i] -= (T)v[i];
            }
			return *this;
		}
		
		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols>& operator*=(const OT v) {
			for(size_t i = 0; i < Rows*Cols; i++) {
                _matrix[i] *= (T)v;
            }
			return *this;
		}

		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols>& operator/=(const OT v) {
			for(size_t i = 0; i < Rows*Cols; i++) {
                _matrix[i] /= (T)v;
            }
			return *this;
		}

        template<FundamentalType OT>
		inline Matrix<T, Rows, Cols> operator+(const OT v) const {
            Matrix<T, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                ret[i] = _matrix[i] + (T)v;
            }
			return ret;
		}

        template<FundamentalType OT>
		inline Matrix<T, Rows, Cols> operator+(const Matrix<OT, Rows, Cols>& v) const {
            Matrix<T, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                ret[i] = _matrix[i] + (T)v[i];
            }
			return ret;
		}

        template<FundamentalType OT>
		inline Matrix<T, Rows, Cols> operator-(const OT& v) const {
            Matrix<T, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                ret[i] = _matrix[i] - (T)v;
            }
            return ret;
		}

        template<FundamentalType OT>
		inline Matrix<T, Rows, Cols> operator-(const Matrix<OT, Rows, Cols>& v) const {
            Matrix<T, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                ret[i] = _matrix[i] - (T)v[i];
            }
            return ret;
		}

		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols> operator*(const OT v) const {
            Matrix<T, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                ret[i] = _matrix[i] * (T)v;
            }
            return ret;
		}

        template<size_t OtherRows>
		inline Matrix<T, OtherRows, Cols> operator*(const Matrix<T, OtherRows, Rows>& v) const {
            Matrix<T, OtherRows, Cols> ret(0);
            // V1
            // for(size_t row = 0; row<OtherRows; row++) {
            //     for(size_t column=0; column<Cols; column++) {
            //         for(size_t i=0; i<Rows; i++) {
            //             ret[row][column] += _matrix[i +column*Rows]*v._matrix[row + i*OtherRows];
            //         }
            //     }
            // }

            // V2
            size_t thisPos = 0;
            for(size_t resultColumn=0; resultColumn<Cols*OtherRows; resultColumn += OtherRows) {
                for(size_t otherCols = 0; otherCols<Rows*OtherRows; otherCols += Rows) {
                    size_t otherPos = otherCols;
                    for(size_t resultRow=resultColumn; resultRow<resultColumn+OtherRows; resultRow++) {
                        ret._matrix[resultRow] += _matrix[thisPos]*v._matrix[otherPos];
                        otherPos++;
                    }
                    thisPos++;
                }
            }
            // V3
            // const T* thisPos = _matrix.data();
            // for(size_t resultColumn=0; resultColumn<Cols*OtherRows; resultColumn += OtherRows) {
            //     for(size_t otherCols = 0; otherCols<Rows*OtherRows; otherCols += Rows) {
            //         const T* otherPos = v._matrix.data() + otherCols;
            //         for(T* result=ret._matrix.data() + resultColumn; result<ret._matrix.data()+resultColumn+OtherRows; result++) {
            //             (*result) += (*thisPos)*(*otherPos);
            //             otherPos++;
            //         }
            //         thisPos++;
            //     }
            // }
            // V4
            // Just use the c++26 feature for this (when available)
            return ret;
		}

		template<FundamentalType OT>
		inline Matrix<T, Rows, Cols> operator/(const OT v) const {
            Matrix<T, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                ret[i] = _matrix[i] / (T)v;
            }
            return ret;
		}

		inline bool operator==(const Matrix<T, Rows, Cols> v) const {
            Matrix<T, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                if(_matrix[i] != v[i]) return false;
            }
            return true;
		}

		inline bool operator!=(const Matrix<T, Rows, Cols> v) const {
			return !this->operator==(v);
		}

        /**
         * @brief A pointer inside the matrix at a certain x-position
         * 
         * @warning Do not store, this is only used to give a nicer looking way to access a matrix 
         */
        struct ArrayPointer {
            T& operator[](const size_t y) {
                ASSERT(y < Cols, "[Util::Matrix] Cannot retrieve a position outside of the matrix")
                return _pointer[y*Rows];// _pointer is at the correct x-pos, set it to the correct y pos
            }
            T* _pointer;
        };
        /**
         * @brief A nice looking way of accessing the matrix
         * When the matrix is one dimensional, you can access it like it is one dimensional:
         * ```
         * Matrix<float, 1, 3> m;
         * m[1] = 1.0f;
         * // But with 2 dimensions:
         * Matrix<float, 2, 2> M;
         * int x=1, y=0;
         * M[x][y] = 1.0f;
         * ```
         * 
         * @param x The coordinate you want to access
         * @return ArrayPointer (when matrix is 2-dimensional) or T& (when matrix is 1-dimensional)
         */
        std::conditional<Rows==1 || Cols==1, T&, ArrayPointer> operator[](const size_t x) {
            ASSERT(x < Rows, "[Util::Matrix] Cannot retrieve a position outside of the matrix")
            if constexpr(Rows==1 || Cols==1) return _matrix[x];
            else return ArrayPointer{&_matrix[x]};// Matrix at the correct x pos
        }

		template<FundamentalType OT>
		inline operator Matrix<OT, Rows, Cols>() const {
            Matrix<OT, Rows, Cols> ret;
			for(size_t i = 0; i < Rows*Cols; i++) {
                ret[i] = (OT)_matrix[i];
            }
            return ret;
		}
        inline operator std::string() const {
            std::string ret;
            ret += "{";
            size_t i = 0;
            for(size_t y = 0; y < Cols; y++) {
                if(y!=0) ret += ",";
                ret += "{";
                for(size_t x = 0; x < Rows; x++) {
                    if(x!=0) ret+= ",";
                    ret += std::to_string(_matrix[i]);
                    i++;
                }
                ret += "}";
            }
            ret += "}";
            return ret;
        }

        float Determinant() const {
            std::vector<T> copy(Rows*Cols);
            memcpy(copy.data(), _matrix.data(), Rows*Cols);
            // Make it a upper traingular matrix

            // Calculate the determinant as a sum of the diagonal
        }

    private:
        std::conditional<IS_BIG_MATRIX, std::array<T, Rows*Cols>, std::vector<T>> _matrix;
    };

    /**
     * @brief Solves A*x=b, returns x
     * 
     * @tparam T 
     * @tparam Rows 
     * @tparam Cols 
     * @param a 
     * @param b 
     * @return Matrix<T, 1, Cols> 
     */
    template<FundamentalType T, unsigned int Rows, unsigned int Cols>
    Matrix<T, 1, Cols> Solve(Matrix<T, Rows, Cols> a, Matrix<T, 1, Cols> b) {

    }

}
}

#endif