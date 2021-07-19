#pragma once

#include "VFSAbstractFileIO.h"
#include <set>

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
			Buffer(Buffer&& other) noexcept : m_buff(other.m_buff), m_autoDelete(other.m_buff) { other.m_autoDelete = false; }
		public:
			void* operator*() const { return m_buff; }
			bool hasAutoDelete() const { return m_autoDelete; }
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
		uint64_t find(ConstKey key) const;
		void getValue (uint64_t index, Val valBuff) const;
		void erase(ConstKey key);
		void optimize();
		float currOptimization() const;
		void flush();
	public:
		uint64_t getKeySize() const;
		uint64_t getValSize() const;
	private:
		uint64_t internalFindSorted(ConstKey key) const;
		uint64_t internalFindUnsorted(ConstKey key) const;
		void internalRead(Location location, Type type, uint64_t index, Buffer buff) const;
		void internalWrite(Location location, Type type, uint64_t index, ConstBuffer buff);
		uint64_t internalGetInFileOffset(Location location, Type type, uint64_t elemIndex) const;
		uint64_t internalGetLocationBegin(Location location) const;
		uint64_t internalGetInElemOffset(Type type) const;
		uint64_t internalGetTypeSize(Type type) const;
		void internalErase();
	private:
		bool internalCompare(ConstKey leftKey, ConstKey rightKey) const;
	private:
		Key makeKey() const;
		Val makeVal() const;
	private:
		std::string m_path;
		AbstractFileIORef m_afio;
		#pragma pack(push, 1)
		struct Header
		{
			const char identifier[6] = { 'V', 'F', 'S', 'M', 'S', 'F' }; // VirtualFileSystem MapStreamFile
			uint64_t keySize = -1;
			uint64_t valSize = -1;
			uint64_t elemSize = -1;
			uint64_t nSorted = 0;
			uint64_t nUnsorted = 0;
		} m_header;
		#pragma pack(pop)
		Comparator m_comp = DefaultComparator;
		static constexpr uint64_t UNSORTED_INDEX_BIT = (1ull << (sizeof(uint64_t) * 8 - 1));
		std::set<uint64_t> m_toErase;
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
			m_header.elemSize = keySize + valSize;
			flush();
		}
	}

	MapStream::~MapStream()
	{
		flush();
	}

	void MapStream::insert(ConstKey key, ConstVal value)
	{
		if (find(key) != -1)
			return; // TODO: Implemenent proper handling if key already exists

		internalWrite(Location::Unsorted, Type::Key, m_header.nUnsorted, key);
		internalWrite(Location::Unsorted, Type::Value, m_header.nUnsorted, value);
		++m_header.nUnsorted;
	}

	uint64_t MapStream::find(ConstKey key) const
	{
		uint64_t index = 0;
		if ((index = internalFindSorted(key)) != -1)
			return index;
		if ((index = internalFindUnsorted(key)) != -1)
			return index;

		return -1;
	}

	void MapStream::getValue(uint64_t index, Val valBuff) const
	{
		internalRead(
			(index & UNSORTED_INDEX_BIT) ? Location::Unsorted : Location::Sorted,
			Type::Value,
			(index & ~UNSORTED_INDEX_BIT),
			valBuff
		);
	}

	void MapStream::erase(ConstKey key)
	{
		uint64_t index = find(key);
		m_toErase.insert(index);
	}

	void MapStream::optimize()
	{
		// TODO: Implement MapStream::optimize
	}

	float MapStream::currOptimization() const
	{
		return m_header.nSorted / (float)std::max(1ull, m_header.nSorted + m_header.nUnsorted);
	}

	void MapStream::flush()
	{
		internalErase();
		m_afio->write(m_path, &m_header, sizeof(Header), 0);
	}

	uint64_t MapStream::getKeySize() const
	{
		return m_header.keySize;
	}

	uint64_t MapStream::getValSize() const
	{
		return m_header.valSize;
	}

	uint64_t MapStream::internalFindSorted(ConstKey key) const
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

	uint64_t MapStream::internalFindUnsorted(ConstKey key) const
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

		return index | UNSORTED_INDEX_BIT;
	}

	void MapStream::internalRead(Location location, Type type, uint64_t index, Buffer buff) const
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

	uint64_t MapStream::internalGetInFileOffset(Location location, Type type, uint64_t elemIndex) const
	{
		return
			internalGetLocationBegin(location) +
			elemIndex * (m_header.keySize + m_header.valSize) +
			internalGetInElemOffset(type);
	}

	uint64_t MapStream::internalGetLocationBegin(Location location) const
	{
		switch (location)
		{
		case Location::Sorted: return sizeof(Header);
		case Location::Unsorted: return sizeof(Header) + m_header.nSorted * (m_header.keySize + m_header.valSize);
		}
		return -1;
	}

	uint64_t MapStream::internalGetInElemOffset(Type type) const
	{
		switch (type)
		{
		case Type::Key: return 0;
		case Type::Value: return internalGetTypeSize(Type::Key);
		}
		return -1;
	}

	uint64_t MapStream::internalGetTypeSize(Type type) const
	{
		switch (type)
		{
		case Type::Key: return m_header.keySize;
		case Type::Value: return m_header.valSize;
		}
		return 0;
	}

	void MapStream::internalErase()
	{
		// TODO: Implement MapStream::internalErase
		uint64_t nErasedSorted = 0;
		uint64_t nErasedUnsorted = 0;

		uint64_t lastByte = internalGetInFileOffset(Location::Unsorted, Type::Key, m_header.nUnsorted) - 1;
		for (auto it = m_toErase.begin(); it != m_toErase.end(); ++it)
		{
			uint64_t index = *it;

			bool isUnsorted = (index & UNSORTED_INDEX_BIT);

			uint64_t blockBegin = internalGetInFileOffset(
				isUnsorted ? Location::Unsorted : Location::Sorted,
				Type::Key,
				(index & ~UNSORTED_INDEX_BIT)
			);

			auto nextIt = std::next(it);

			uint64_t blockEnd = (nextIt != m_toErase.end()) ?
				internalGetInFileOffset(
					isUnsorted ? Location::Unsorted : Location::Sorted,
					Type::Key,
					(*nextIt & ~UNSORTED_INDEX_BIT)
				) :
				lastByte;

			uint64_t blockSize = blockEnd - blockBegin;
			Buffer buffer(blockSize);
			m_afio->read(m_path, *buffer, blockSize, blockBegin);
			m_afio->write(m_path, *buffer, blockSize, blockBegin - m_header.elemSize * (nErasedSorted + nErasedUnsorted));

			++*(isUnsorted ? &nErasedUnsorted : &nErasedSorted);
		}
		m_header.nSorted -= nErasedSorted;
		m_header.nUnsorted -= nErasedUnsorted;
		m_toErase.clear();
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