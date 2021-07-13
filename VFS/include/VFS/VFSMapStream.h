#pragma once

#include "VFSAbstractFileIO.h"

namespace VFS {

	class MapStream
	{
	public:
		typedef void* Key, * Val;
		typedef const void* ConstKey, * ConstVal;
		enum class Location { Unspecified = 0, Sorted, Unsorted };
		typedef bool (*Comparator)(ConstKey, ConstKey, uint64_t);
		static bool DefaultComparator(ConstKey left, ConstKey right, uint64_t size);
	public:
		MapStream(const std::string& path, AbstractFileIORef afio, uint64_t& keySize, uint64_t& valSize);
		~MapStream();
	public:
		void insert(ConstKey key, ConstVal value);
		bool find(ConstKey key, Val valBuff);
		void erase(ConstKey key);
		void optimize();
		float currOptimization();
		void flush();
	public:
		uint64_t getKeySize() const;
		uint64_t getValSize() const;
	private:
		bool internalFindSorted(ConstKey key, Val valBuff);
		bool internalFindUnsorted(ConstKey key, Val valBuff);
		void internalReadKey(uint64_t index, Key keyBuff, Location location);
		void internalReadVal(uint64_t index, Val valBuff, Location location);
		void internalWriteVal(uint64_t index, ConstVal value, Location location);
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

	bool MapStream::find(ConstKey key, Val valBuff)
	{
		if (internalFindSorted(key, valBuff))
			return true;
		if (internalFindUnsorted(key, valBuff))
			return true;

		return false;
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

	bool MapStream::internalFindSorted(ConstKey key, Val valBuff)
	{
		// TODO: Implement MapStream::internalFindSorted
	}

	bool MapStream::internalFindUnsorted(ConstKey key, Val valBuff)
	{
		bool found = false;
		Key temp = makeKey();
		for (uint64_t i = 0; i < m_header.nUnsorted; ++i)
		{
			internalReadKey(i, temp, Location::Unsorted);
			if (!internalCompare(temp, val) &&
				!internalCompare(val, temp))
			{
				found = true;
				goto findEnd;
			}
		}
	findEnd:
		destroyKey(temp);
		return found;
	}

	void MapStream::internalReadKey(uint64_t index, Key keyBuff, Location location)
	{
		// TODO: Implement MapStream::internalReadKey
	}

	void MapStream::internalReadVal(uint64_t index, Val valBuff, Location location)
	{
		// TODO: Implement MapStream::internalReadVal
	}

	void MapStream::internalWriteVal(uint64_t index, ConstVal value, Location location)
	{
		// TODO: Implement MapStream::internalWriteVal
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