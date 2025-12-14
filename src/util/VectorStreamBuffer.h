#ifndef ENGINE_UTIL_STREAMBUFFER_H
#define ENGINE_UTIL_STREAMBUFFER_H

#include "core/PCH.h"

namespace Engine {
namespace Util {

    template<class T, class Q=char> requires (sizeof(T)==1 && sizeof(Q) == 1)
    class VectorStreamBuffer : public std::basic_streambuf<Q> {
    public:

        VectorStreamBuffer(std::vector<T>& vec) : _vec(vec) {
            Parent::setg(begin(), begin(), end());
            Parent::setp(end(), end());
        }
    
    protected:
        typedef std::basic_streambuf<Q>             Parent;
        typedef Parent::traits_type					traits_type;
        typedef Parent::int_type             		int_type;

        constexpr int_type eof() {
            return traits_type::eof();
        }

        /// @name Read area
        ///@{
        /**
         * @brief Return next character without consuming it
         * 
         * @return int 
         */
        int_type underflow() override {
            if(Parent::gptr() < Parent::egptr()) return *Parent::gptr();
            return eof();
        }

        /**
         * @brief Return next character and consume one char
         * 
         * @return int 
         */
        int_type uflow() override {
            if(Parent::gptr() < Parent::egptr()) {
                Parent::setg(begin(), Parent::gptr()+1, end());
                return *(Parent::gptr()-1);
            }
            return eof();
        }

        /**
         * @brief Put character back into input sequence
         * 
         * @param ch The character to input (eof will just move the position)
         * @return int eof() if failed, else the character that was inputted
         */
        int_type pbackfail(int_type ch = traits_type::eof()) override {
            if(Parent::gptr() <= Parent::eback()) return eof();
            Parent::setg(begin(), Parent::gptr()-1, end());
            if(ch != eof()) *Parent::gptr() = traits_type::to_char_type(ch);
            return ch;
        }

        /**
         * @brief Read multiple chars into the buffer
         * 
         * @param s The buffer
         * @param n The amount to read into the buffer
         * @return std::streamsize The amount actually read into the buffer
         */
        std::streamsize xsgetn(Q* s, std::streamsize n) override {
            size_t remainingBytes = Parent::egptr() - Parent::gptr();
            size_t readingBytes = Util::min<size_t>(remainingBytes, n);
            memcpy(s, Parent::gptr(), readingBytes);
            Parent::setg(begin(), Parent::gptr() + readingBytes, end());
            return readingBytes;
        }

        // showmanyc??
        ///@}

        /// @name Put area
        ///@{

        /**
         * @brief Put multiple chars into the buffer
         * 
         * @param s The buffer
         * @param n The amount to put into the buffer
         * @return std::streamsize The amount actually put into the buffer
         */
        std::streamsize xsputn(const Q* s, std::streamsize n) override {
            // Always put it at the end of the vector
            size_t readingOffset = Parent::gptr() - Parent::eback();
            _vec.insert(_vec.end(), s, s+n);
            Parent::setg(begin(), begin() + readingOffset, end());
            Parent::setp(end(), end());
            return n;
        }

        /**
         * @brief Write a single character
         * 
         * @param ch The character to write
         * @return int The character if written, else eof()
         */
        int_type overflow(int_type ch) override {
            Q converted = traits_type::to_char_type(ch);
            if(xsputn(&converted, 1) != eof()) return ch;
            return eof();
        }
        ///@}

        /// @name Positioning
        /// All not implemented
        ///@{
        std::streambuf* setbuf(Q* s, std::streamsize n) override {
            return this; // ignore
        }
        int_type sync() override {
            return 0; // success
        }
        std::streampos seekoff(
            std::streamoff off,
            std::ios_base::seekdir way,
            std::ios_base::openmode which = std::ios_base::in | std::ios_base::out
        ) override {
            return std::streampos(std::streamoff(-1));  // error
        }
        std::streampos seekpos(
            std::streampos pos,
            std::ios_base::openmode which = std::ios_base::in | std::ios_base::out
        ) override {
            return std::streampos(std::streamoff(-1)); // error
        }
        ///@}

    private:
        Q* begin() {
            // ASSERT(_vec.size() == 0, "[Util::VectorStreamBuffer] Vector must have at least a size of 1")
            return reinterpret_cast<Q*>(_vec.data());
        }
        Q* end() {
            // ASSERT(_vec.size() == 0, "[Util::VectorStreamBuffer] Vector must have at least a size of 1")
            return reinterpret_cast<Q*>(_vec.data() + _vec.size());
        }

        std::vector<T>& _vec;
    };

}
}

#endif