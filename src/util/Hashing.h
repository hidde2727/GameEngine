#ifndef ENGINE_UTIL_HASHING_H
#define ENGINE_UTIL_HASHING_H

#include "core/PCH.h"

namespace Engine {
namespace Util {

    std::array<uint8_t, 20> SHA1(std::string input);

    template<size_t S>
	std::string Base64Encode(std::array<std::uint8_t, S>input) {
		constexpr char base64[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
		std::string out;
		for (size_t i = 0; i < S / 3; i++) {
			out += base64[(input[3 * i] & 0xfc) >> 2];
			out += base64[((input[3 * i] & 0x03) << 4)	   + ((input[3 * i + 1] & 0xf0) >> 4)];
			out += base64[((input[3 * i + 2] & 0xc0) >> 6) + ((input[3 * i + 1] & 0x0f) << 2)];
			out += base64[input[3 * i + 2] & 0x3f];
		}
		if (S % 3 == 2) {
			out += base64[( input[S - 2] & 0xfc) >> 2];
			out += base64[((input[S - 2] & 0x03) << 4) + ((input[S - 1] & 0xf0) >> 4)];
			out += base64[							     ((input[S - 1] & 0x0f) << 2)];
			out += '=';
		}
		else if (S % 3 == 1) {
			out += base64[(input[S - 1] & 0xfc) >> 2];
			out += base64[(input[S - 1] & 0x03) << 4];
			out += '=';
			out += '=';
		}
		return out;
	}
	
	// The exact same as base64 encode but with the / changed to -
	template<size_t S>
	std::string Base64FileEncode(std::array<std::uint8_t, S>input) {
		constexpr char base64[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-" };
		std::string out;
		for (size_t i = 0; i < S / 3; i++) {
			out += base64[(input[3 * i] & 0xfc) >> 2];
			out += base64[((input[3 * i] & 0x03) << 4)	   + ((input[3 * i + 1] & 0xf0) >> 4)];
			out += base64[((input[3 * i + 2] & 0xc0) >> 6) + ((input[3 * i + 1] & 0x0f) << 2)];
			out += base64[input[3 * i + 2] & 0x3f];
		}
		if (S % 3 == 2) {
			out += base64[( input[S - 2] & 0xfc) >> 2];
			out += base64[((input[S - 2] & 0x03) << 4) + ((input[S - 1] & 0xf0) >> 4)];
			out += base64[							     ((input[S - 1] & 0x0f) << 2)];
			out += '=';
		}
		else if (S % 3 == 1) {
			out += base64[(input[S - 1] & 0xfc) >> 2];
			out += base64[(input[S - 1] & 0x03) << 4];
			out += '=';
			out += '=';
		}
		return out;
	}

}
}

#endif