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
		void internalFindSorted();
		void internalFindUnsorted();
		void internalReadKey(uint64_t index, Key key, Location location);
		void internalReadVal(uint64_t index, Val value, Location location);
		void internalWriteVal(uint64_t index, ConstVal value, Location location);
	private:
		bool internalCompare(ConstKey leftKey, ConstKey rightKey) const;
	private:
		std::string m_path;
		AbstractFileIORef m_afio;
		Comparator m_comp = DefaultComparator;
	};

}