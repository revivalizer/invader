#include "pch.h"

namespace invader {

namespace complex
{
	uint32_t bitrev(uint32_t x, uint32_t numBits)
	{
		// http://stackoverflow.com/questions/746171/best-algorithm-for-bit-reversal-from-msb-lsb-to-lsb-msb-in-c
		// based on
		// http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious

		x <<= (32-numBits);

	    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	    return((x >> 16) | (x << 16));
	};
};

} // namespace invader