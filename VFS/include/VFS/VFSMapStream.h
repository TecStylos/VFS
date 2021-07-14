#pragma once

#include "VFSAbstractFileIO.h"

namespace VFS {

	class MapStream
	{
	public:
		typedef void* Key, * Val;
		typedef const void* ConstKey, * ConstVal;
		enum class Location { Unspecified = 0, Sorted, Unsorted };
		enum class Type { Unspecified = 0, Key, Value };
		typedef bool (*Comparator)(ConstKey, ConstKey, uint64_t);
		static bool DefaultComparator(ConstKey left, ConstKey right, uint64_t size);
	public:
		MapStream(const std::string& path, AbstractFileIORef afio, uint64_t& keySize, uint64_t& valSize);
		~MapStream();
	public:
		void insert(ConstKey key, ConstVal value);
		uint64_t find(ConstKey key);
		void erase(ConstKey key);
		void optimize();
		float currOptimization();
		void flush();
	public:
		uint64_t getKeySize() const;
		uint64_t getValSize() const;
	private:
		uint64_t internalFindSorted(ConstKey key);
		uint64_t internalFindUnsorted(ConstKey key);
		void internalRead(Location location, Type type, uint64_t index, Val valBuff);
		void internalWrite(Location location, Type type, uint64_t index, ConstVal value);
		uint64_t internalCalcInFileOffset(Location location, Type type, uint64_t elemIndex);
		uint64_t internalGetInListOffset(Location location);
		uint64_t internalGetInElemOffset(Type type);
		uint64_t internalGetTypeSize(Type type);
	private:
		bool internalCompare(ConstKey leftKey, ConstKey rightKey) const;
	private:
		Key makeKey() const;
		Val makeVal() const;
		void destroyKey(Key key) const;
		void destroyVal(Val value) const;
	private:
		std::string m_path;
		AbstractFileIORef m_afio;
		struct Header
		{
			const char[6] = { 'V', 'F', 'S', 'M', 'S', 'F' }; // VirtualFileSystem MapStreamFile
			uint64_t keySize = 4;
			uint64_t valSize = 16;
			uint64_t nSorted = 0;
			uint64_t nUnsorted = 0;
			uint64_t sortedOffset = 0;
			uint64_t unsortedOffset = 0;
		} m_header;
		Comparator m_comp = DefaultComparator;
	};

	bool MapStream::DefaultComparator(ConstKey left, ConstKey right, uint64_t size)
	{
		const char* l = left;
		const char* r = right;

		for (uint64_t i = 0; i < size; ++i)
		{
			if (l[i] != r[i])
				return l[i] < r[i];
		}

		return false;
	}

	MapStream::MapStream(const std::string& path, AbstractFileIORef afio, uint64_t& keySize, uint64_t& valSize)
	{
		// TODO: Implement MapStream constructor
	}

	MapStream::~MapStream()
	{
		flush();
	}

	void MapStream::insert(ConstKey key, ConstVal value)
	{
		// TODO: Implement MapStream::insert
	}

	uint64_t MapStream::find(ConstKey key)
	{
		uint64_t index = 0;
		if (index = internalFindSorted(key) != -1)
			return index;
		if (index = internalFindUnsorted(key) != -1)
			return index;

		return -1;
	}

	void MapStream::erase(ConstKey key)
	{
		// TODO: Implement MapStream::erase
	}

	void MapStream::optimize()
	{
		// TODO: Implement MapStream::optimize
	}

	float MapStream::currOptimization()
	{
		// TODO: Implement MapStream::currOptimization
	}

	void MapStream::flush()
	{
		// TODO: Implement MapStream::currOptimization
	}

	uint64_t MapStream::getKeySize() const
	{
		return m_header.keySize;
	}

	uint64_t MapStream::getValSize() const
	{
		return m_header.valSize;
	}

	uint64_t MapStream::internalFindSorted(ConstKey key)
	{
		// TODO: Implement MapStream::internalFindSorted
	}

	uint64_t MapStream::internalFindUnsorted(ConstKey key)
	{
		uint64_t index = -1;
		Key temp = makeKey();
		for (uint64_t i = 0; i < m_header.nUnsorted; ++i)
		{
			internalReadKey(i, temp, Location::Unsorted);
			if (!internalCompare(temp, val) &&
				!internalCompare(val, temp))
			{
				index = i;
				break;
			}
		}

		destroyKey(temp);
		return index;
	}

	void MapStream::internalRead(Location location, Type type, uint64_t index, Val valBuff)
	{
		m_afio->read(
			m_path,
			valBuff,
			internalGetTypeSize(type),
			internalCalcInFileOffset(
				location,
				index,
				type
			)
		);
	}

	void MapStream::internalWrite(Location location, Type type, uint64_t index, ConstVal value)
	{
		m_afio->write(
			m_path,
			value,
			internalGetTypeSize(type),
			internalCalcInFileOffset(
				location,
				index,
				type
			)
		);
	}

	uint64_t MapStream::internalCalcInFileOffset(Location location, Type type, uint64_t elemIndex)
	{
		return
			internalGetInListOffset(location) +
			elemIndex * (m_header.keySize + m_header.valSize) +
			internalGetInElemOffset(type);
	}

	uint64_t MapStream::internalGetInListOffset(Location location)
	{
		switch (location)
		{
		case Location::Sorted: return m_header.sortedOffset;
		case Location::Unsorted: return m_header.unsortedOffset;
		}
		return -1;
	}

	uint64_t MapStream::internalGetInElemOffset(Type type)
	{
		switch (type)
		{
		case Type::Key: return 0;
		case Type::Value: return internalGetTypeSize(Type::Key);
		}
		return -1;
	}

	uint64_t MapStream::internalGetTypeSize(Type type)
	{
		switch (Type type)
		{
		case Type::Key: return m_header.keySize;
		case Type::Value: return m_header.valSize;
		}
		return 0;
	}

	bool MapStream::internalCompare(ConstKey leftKey, ConstKey rightKey) const
	{
		return m_comp(leftKey, rightKey, m_keySize);
	}

	Key MapStream::makeKey() const
	{
		return new char[m_header.keySize];
	}

	Val MapStream::makeVal() const
	{
		return new char[m_header.valSize];
	}

	void MapStream::destroyKey(Key key) const
	{
		delete (char*)key;
	}
	void MapStream::destroyVal(Val value) const
	{
		delete (char*)value;
	}
}