#pragma once

#include "VFSAbstractFileIO.h"

namespace VFS {

	class MapStream
	{
	public:
		class Buffer
		{
		public:
			Buffer(void* buff) : m_buff((char*)buff), m_autoDelete(false) {}
			Buffer(uint64_t size) : m_buff(new char[size]), m_autoDelete(true) {}
			Buffer(const Buffer& other) : m_buff(other.m_buff), m_autoDelete(false) {}
			Buffer(Buffer&& other) : m_buff(other.m_buff), m_autoDelete(other.m_buff) { other.m_autoDelete = false; }
		public:
			void* operator*() const { return m_buff; }
		private:
			char* m_buff;
			bool m_autoDelete;
		};
		typedef Buffer Key, Val;
		typedef const Buffer ConstBuffer;
		typedef ConstBuffer ConstKey, ConstVal;
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
		void internalRead(Location location, Type type, uint64_t index, Buffer buff);
		void internalWrite(Location location, Type type, uint64_t index, ConstBuffer buff);
		uint64_t internalGetInFileOffset(Location location, Type type, uint64_t elemIndex);
		uint64_t internalGetLocationBegin(Location location);
		uint64_t internalGetInElemOffset(Type type);
		uint64_t internalGetTypeSize(Type type);
	private:
		bool internalCompare(ConstKey leftKey, ConstKey rightKey) const;
	private:
		Key makeKey() const;
		Val makeVal() const;
	private:
		std::string m_path;
		AbstractFileIORef m_afio;
		struct Header
		{
			char identifier[6] = { 'V', 'F', 'S', 'M', 'S', 'F' }; // VirtualFileSystem MapStreamFile
			uint64_t keySize = -1;
			uint64_t valSize = -1;
			uint64_t nSorted = 0;
			uint64_t nUnsorted = 0;
		} m_header;
		bool m_isFlushed = true;
		Comparator m_comp = DefaultComparator;
	};

	bool MapStream::DefaultComparator(ConstKey left, ConstKey right, uint64_t size)
	{
		auto l = (const char*)*left;
		auto r = (const char*)*right;

		for (uint64_t i = 0; i < size; ++i)
		{
			if (l[i] != r[i])
				return l[i] < r[i];
		}

		return false;
	}

	MapStream::MapStream(const std::string& path, AbstractFileIORef afio, uint64_t& keySize, uint64_t& valSize)
		: m_path(path), m_afio(afio)
	{
		// TODO: Implement MapStream constructor
		if (m_afio->exists(m_path))
		{
			m_afio->read(m_path, &m_header, sizeof(Header), 0);
			keySize = m_header.keySize;
			valSize = m_header.valSize;
		}
		else
		{
			m_afio->make(m_path);
			m_header.keySize = keySize;
			m_header.valSize = valSize;
			m_afio->write(path, &m_header, sizeof(Header), 0);
		}
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
		if ((index = internalFindSorted(key)) != -1)
			return index;
		if ((index = internalFindUnsorted(key)) != -1)
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
		return 0.0f;
	}

	void MapStream::flush()
	{
		// TODO: Implement MapStream::flush
		m_isFlushed = true;
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
		uint64_t stepSize = m_header.nSorted - 2;
		uint64_t index = stepSize;

		uint64_t result = -1;

		Key temp = makeKey();
		do
		{
			if (internalCompare(temp, key))
				index += stepSize;
			else if (internalCompare(key, temp))
				index -= stepSize;
			else
				result = index;

			stepSize /= 2;
		} while (stepSize > 0 && result == -1);

		return result;
	}

	uint64_t MapStream::internalFindUnsorted(ConstKey key)
	{
		uint64_t index = -1;
		Key temp = makeKey();
		for (uint64_t i = 0; i < m_header.nUnsorted; ++i)
		{
			internalRead(Location::Unsorted, Type::Key, i, temp);
			if (!internalCompare(temp, key) &&
				!internalCompare(key, temp))
			{
				index = i;
				break;
			}
		}

		return index;
	}

	void MapStream::internalRead(Location location, Type type, uint64_t index, Buffer buff)
	{
		m_afio->read(
			m_path,
			*buff,
			internalGetTypeSize(type),
			internalGetInFileOffset(
				location,
				type,
				index
			)
		);
	}

	void MapStream::internalWrite(Location location, Type type, uint64_t index, ConstBuffer buff)
	{
		m_afio->write(
			m_path,
			*buff,
			internalGetTypeSize(type),
			internalGetInFileOffset(
				location,
				type,
				index
			)
		);
	}

	uint64_t MapStream::internalGetInFileOffset(Location location, Type type, uint64_t elemIndex)
	{
		return
			internalGetLocationBegin(location) +
			elemIndex * (m_header.keySize + m_header.valSize) +
			internalGetInElemOffset(type);
	}

	uint64_t MapStream::internalGetLocationBegin(Location location)
	{
		switch (location)
		{
		case Location::Sorted: return sizeof(Header);
		case Location::Unsorted: return sizeof(Header) + m_header.nSorted * (m_header.keySize + m_header.valSize);
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
		switch (type)
		{
		case Type::Key: return m_header.keySize;
		case Type::Value: return m_header.valSize;
		}
		return 0;
	}

	bool MapStream::internalCompare(ConstKey leftKey, ConstKey rightKey) const
	{
		return m_comp(leftKey, rightKey, m_header.keySize);
	}

	MapStream::Key MapStream::makeKey() const
	{
		return Key(m_header.keySize);
	}

	MapStream::Val MapStream::makeVal() const
	{
		return Val(m_header.valSize);
	}
}