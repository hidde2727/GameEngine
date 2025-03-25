#include "util/Hashing.h"

namespace Engine {
namespace Util {
    
    inline int32_t leftshift(uint32_t value, size_t shift) {
        return (value << shift) | (value >> (32 - shift));
    }

    std::array<uint8_t, 20> SHA1(std::string input) {
        size_t buflen;
        uint32_t digest[5];
        uint8_t buf[64];

        buflen = 0;
        digest[0] = 0x67452301;
        digest[1] = 0xefcdab89;
        digest[2] = 0x98badcfe;
        digest[3] = 0x10325476;
        digest[4] = 0xc3d2e1f0;

        size_t size = input.size();
        uint64_t total_bits = size * 8;

        // Add the indicator that the data is 8-bit
        input += (char)0x80;
        size++;

        // Pad with 0's the data till it is size % 64 == 64 - 8 so we can fit the total lenght (8 bytes) after the data
        while (size % 64 != 64 - 8) {
            input += (char)0x00;
            size++;
        }

        // Add the total size to the input
        input += (char)((total_bits >> (32 + 24)) & 0xff);
        input += (char)((total_bits >> (32 + 16)) & 0xff);
        input += (char)((total_bits >> (32 + 8)) & 0xff);
        input += (char)((total_bits >> (32)) & 0xff);
        input += (char)((total_bits >> (24)) & 0xff);
        input += (char)((total_bits >> (16)) & 0xff);
        input += (char)((total_bits >> (8)) & 0xff);
        input += (char)((total_bits) & 0xff);
        size += 8;

        for (size_t i = 0; i < size / 64; i++) {
            std::uint32_t block[80];
            // Take the current block and convert it to 16 * 32 bit
            memcpy(buf, &input[i * 64], sizeof(buf));
            for (size_t j = 0; j < 16; j++) {
                block[j] =
                    ((uint32_t)(buf[4 * j + 3])) |
                    ((uint32_t)(buf[4 * j + 2])) << 8 |
                    ((uint32_t)(buf[4 * j + 1])) << 16 |
                    ((uint32_t)(buf[4 * j + 0])) << 24;
            }

            // Transform the block into the hash
            uint32_t a = digest[0];
            uint32_t b = digest[1];
            uint32_t c = digest[2];
            uint32_t d = digest[3];
            uint32_t e = digest[4];

            uint32_t f = 0;
            uint32_t k = 0;
            uint32_t temp = 0;

            for (size_t i = 16; i <= 79; i++) {
                block[i] = leftshift(block[i - 3] ^ block[i - 8] ^ block[i - 14] ^ block[i - 16], 1);
            }

            for (size_t i = 0; i <= 79; i++) {
                if (i >= 0 && i <= 19) {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                }
                else if (i >= 20 && i <= 39) {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                }
                else if (i >= 40 && i <= 59) {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                }
                else if (i >= 60 && i <= 79) {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                temp = leftshift(a, 5) + f + e + k + block[i];
                e = d;
                d = c;
                c = leftshift(b, 30);
                b = a;
                a = temp;
            }

            digest[0] += a;
            digest[1] += b;
            digest[2] += c;
            digest[3] += d;
            digest[4] += e;
        }

        std::array<uint8_t, 20> result;
        for (size_t i = 0; i < 20/4; i++) {
            result[i * 4 + 3] = digest[i] & 0xff;
            result[i * 4 + 2] = (digest[i] >> 8) & 0xff;
            result[i * 4 + 1] = (digest[i] >> 16) & 0xff;
            result[i * 4 + 0] = (digest[i] >> 24) & 0xff;
        }
        return result;
    } 
    
}
}