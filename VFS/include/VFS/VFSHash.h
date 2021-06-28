#pragma once

#include <cstdint>
#include <string>

#include "RotaryShift.h"

namespace VFS {

	typedef uint64_t Hash;

	Hash makeHash(const std::string& str)
	{
		constexpr uint64_t packSize = sizeof(str);

		auto combine = [](Hash& full, Hash part, uint64_t index)
		{
			full ^= rotaryShiftLeft64(part, (index * 2) % (sizeof(Hash) * CHAR_BIT));
		};

		const uint64_t nBytes = str.size();
		const uint64_t nPacks = nBytes / packSize;
		const uint64_t offsetLastPack = nPacks * packSize;
		const uint64_t nBytesLastPack = nBytes % packSize;

		Hash hashFull;

		for (uint64_t i = 0; i < nPacks; ++i)
			combine(hashFull, *(const Hash*)(str.data() + (i * nBytes)), i);

		if (nBytesLastPack)
		{
			Hash hashPart = 0;
			memcpy(&hashPart, *(const Hash*)(str.data() + offsetLastPack), nBytesLastPack);
			combine(hashFull, hashPart, nPacks);
		}

		return hashFull;
	}

} // namespace VFS