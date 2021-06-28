#pragma once

#include <cstdint>
#include <limits.h>

namespace VFS {
	static inline uint64_t rotaryShiftLeft64(uint64_t n, unsigned int c)
	{
		const uint64_t mask = (CHAR_BIT * sizeof(n) - 1);  // assumes width is a power of 2.

		// assert ( (c<=mask) &&"rotate by type width or more");
		c &= mask;
		return (n << c) | (n >> ((-c) & mask));
	}

	static inline uint64_t rotaryShiftRight64(uint64_t n, unsigned int c)
	{
		const uint64_t mask = (CHAR_BIT * sizeof(n) - 1);

		// assert ( (c<=mask) &&"rotate by type width or more");
		c &= mask;
		return (n >> c) | (n << ((-c) & mask));
	}
}