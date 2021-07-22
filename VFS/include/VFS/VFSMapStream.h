#pragma once

#include "VFSAbstractFileIO.h"
#include <set>
#include <functional>

namespace VFS {

	class MapStream
	{
	public:
		class Buffer
		{
		public:
			Buffer(void* buff) : m_buff((char*)buff), m_autoDelete(false) {}
			explicit Buffer(uint64_t size) : m_buff(new char[size]), m_autoDelete(true) {}
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
		enum class Type { Unspecified = 0, Key, Value, Elem };
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
	private:
		uint64_t findSorted(ConstKey key) const;
		uint64_t findUnsorted(ConstKey key) const;
		void read(Location location, Type type, uint64_t index, Buffer buff) const;
		void write(Location location, Type type, uint64_t index, ConstBuffer buff);
		uint64_t getOffsetInFile(Location location, Type type, uint64_t elemIndex) const;
		uint64_t getOffsetLocation(Location location) const;
		uint64_t getOffsetInElem(Type type) const;
		uint64_t size(Type type) const;
		void eraseFinal();
	private:
		bool compare(ConstKey leftKey, ConstKey rightKey) const;
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
		static constexpr uint64_t UNSORTED_INDEX_BIT = (1ull << (sizeof(uint64_t) * 8 - 1));
		std::set<uint64_t> m_toErase;
	};

	MapStream::MapStream(const std::string& path, AbstractFileIORef afio, uint64_t& keySize, uint64_t& valSize)
		: m_path(path), m_afio(afio)
	{
		if (m_afio->exists(m_path))
		{
			m_afio->read(m_path, &m_header, sizeof(Header), 0);
			keySize = size(Type::Key);
			valSize = size(Type::Value);
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

		write(Location::Unsorted, Type::Key, m_header.nUnsorted, key);
		write(Location::Unsorted, Type::Value, m_header.nUnsorted, value);
		++m_header.nUnsorted;
	}

	uint64_t MapStream::find(ConstKey key) const
	{
		uint64_t index = 0;
		if ((index = findSorted(key)) != -1)
			return index;
		if ((index = findUnsorted(key)) != -1)
			return index;

		return -1;
	}

	void MapStream::getValue(uint64_t index, Val valBuff) const
	{
		read(
			(index & UNSORTED_INDEX_BIT) ? Location::Unsorted : Location::Sorted,
			Type::Value,
			(index & ~UNSORTED_INDEX_BIT),
			valBuff
		);
	}

	void MapStream::erase(ConstKey key)
	{
		uint64_t index = find(key);

		if (index == -1)
			return;

		m_toErase.insert(index);
	}

	void MapStream::optimize()
	{
		flush();

		Buffer buff(m_header.nUnsorted * size(Type::Elem));

		//std::qsort(*buff, m_header.nUnsorted, size(Type::Elem), m_comp); // TODO: Implement own sort algorithm

		// TODO: Implement optimization
	}

	float MapStream::currOptimization() const
	{
		return m_header.nSorted / (float)std::max(1ull, m_header.nSorted + m_header.nUnsorted);
	}

	void MapStream::flush()
	{
		eraseFinal();
		m_afio->write(m_path, &m_header, sizeof(Header), 0);
	}

	uint64_t MapStream::findSorted(ConstKey key) const
	{
		uint64_t stepSize = m_header.nSorted - 2;
		uint64_t index = stepSize;

		uint64_t result = -1;

		Key temp = makeKey();
		do
		{
			if (compare(temp, key))
				index += stepSize;
			else if (compare(key, temp))
				index -= stepSize;
			else
				result = index;

			stepSize /= 2;
		} while (stepSize > 0 && result == -1);

		return result;
	}

	uint64_t MapStream::findUnsorted(ConstKey key) const
	{
		uint64_t index = -1;
		Key temp = makeKey();
		for (uint64_t i = 0; i < m_header.nUnsorted; ++i)
		{
			read(Location::Unsorted, Type::Key, i, temp);
			if (!compare(temp, key) &&
				!compare(key, temp))
			{
				index = i;
				break;
			}
		}

		return index | UNSORTED_INDEX_BIT;
	}

	void MapStream::read(Location location, Type type, uint64_t index, Buffer buff) const
	{
		m_afio->read(
			m_path,
			*buff,
			size(type),
			getOffsetInFile(
				location,
				type,
				index
			)
		);
	}

	void MapStream::write(Location location, Type type, uint64_t index, ConstBuffer buff)
	{
		m_afio->write(
			m_path,
			*buff,
			size(type),
			getOffsetInFile(
				location,
				type,
				index
			)
		);
	}

	uint64_t MapStream::getOffsetInFile(Location location, Type type, uint64_t elemIndex) const
	{
		return
			getOffsetLocation(location) +
			elemIndex * size(Type::Elem) +
			getOffsetInElem(type);
	}

	uint64_t MapStream::getOffsetLocation(Location location) const
	{
		switch (location)
		{
		case Location::Sorted: return sizeof(Header);
		case Location::Unsorted: return sizeof(Header) + m_header.nSorted * (size(Type::Key) + size(Type::Value));
		}
		return -1;
	}

	uint64_t MapStream::getOffsetInElem(Type type) const
	{
		switch (type)
		{
		case Type::Key: return 0;
		case Type::Value: return size(Type::Key);
		}
		return -1;
	}

	uint64_t MapStream::size(Type type) const
	{
		switch (type)
		{
		case Type::Key: return m_header.keySize;
		case Type::Value: return m_header.valSize;
		case Type::Elem: return m_header.elemSize;
		}
		return 0;
	}

	void MapStream::eraseFinal()
	{
		uint64_t nErasedSorted = 0;
		uint64_t nErasedUnsorted = 0;

		if (!m_toErase.empty())
		{
			uint64_t endIndex = (m_header.nUnsorted | UNSORTED_INDEX_BIT);
			m_toErase.insert(endIndex);
		}

		for (auto it = m_toErase.begin(); it != m_toErase.end(); ++it)
		{
			uint64_t index = *it;
			bool isUnsorted = (index & UNSORTED_INDEX_BIT);
			index = (index & ~UNSORTED_INDEX_BIT);

			uint64_t blockBegin = getOffsetInFile(
				isUnsorted ? Location::Unsorted : Location::Sorted,
				Type::Key,
				index + 1
			);

			auto nextIt = std::next(it);

			if (nextIt == m_toErase.end())
				break;

			uint64_t blockSize = getOffsetInFile(
				isUnsorted ? Location::Unsorted : Location::Sorted,
				Type::Key,
				(*nextIt & ~UNSORTED_INDEX_BIT)
			) - blockBegin;

			uint64_t nRemaining = blockSize;
			constexpr uint64_t maxBuffSize = 16384;
			Buffer buffer(std::min(maxBuffSize, nRemaining));
			uint64_t blockBeginShifted = blockBegin - size(Type::Elem) * (nErasedSorted + nErasedUnsorted + 1);
			while (nRemaining != 0)
			{
				uint64_t nToMove = std::min(maxBuffSize, nRemaining);
				m_afio->read(m_path, *buffer, nToMove, blockBegin);
				m_afio->write(m_path, *buffer, nToMove, blockBeginShifted);

				blockBegin += nToMove;
				blockBeginShifted += nToMove;

				nRemaining -= nToMove;
			}

			++*(isUnsorted ? &nErasedUnsorted : &nErasedSorted);
		}
		m_header.nSorted -= nErasedSorted;
		m_header.nUnsorted -= nErasedUnsorted;
		m_toErase.clear();
	}

	bool MapStream::compare(ConstKey leftKey, ConstKey rightKey) const
	{
		auto l = (const char*)*leftKey;
		auto r = (const char*)*rightKey;

		for (uint64_t i = 0; i < size(Type::Key); ++i)
		{
			if (l[i] != r[i])
				return l[i] < r[i];
		}

		return false;
	}

	MapStream::Key MapStream::makeKey() const
	{
		return Key(size(Type::Key));
	}

	MapStream::Val MapStream::makeVal() const
	{
		return Val(size(Type::Value));
	}
}