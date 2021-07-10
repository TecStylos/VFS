#pragma once

#include <string>
#include <vector>

#include "VFSPlatform.h"
#include "VFSHash.h"

namespace VFS {

	class HashPath
	{
	public:
		struct Element
		{
		public:
			Element(const std::string& name);
		public:
			uint64_t asHash() const { return m_asHash; }
			const std::string& asString() const { return m_asString; }
		public:
			bool operator==(const Element& other) const;
			bool operator!=(const Element& other) const;
		private:
			uint64_t m_asHash;
			std::string m_asString;
		};
		class Iterator
		{
		public:
			Iterator(std::vector<Element>::const_iterator it);
		public:
			bool operator!=(const Iterator& other) const;
			Iterator& operator++();
			const Element& operator*() const;
		private:
			std::vector<Element>::const_iterator m_it;
		};
	public:
		HashPath() = default;
		HashPath(const std::string& path);
	public:
		uint64_t depth() const;
		HashPath parent() const;
		HashPath child(const Element& elem) const;
		std::string getRealPath(const std::string& basePath = "") const;
		static std::string formatPath(std::string path);
	public:
		HashPath operator+(const HashPath& other) const;
		HashPath& operator+=(const HashPath& other);
		HashPath operator+(const Element& elem) const;
		HashPath& operator+=(const Element& elem);
		HashPath operator-(uint64_t depthDiff) const;
		HashPath& operator-=(uint64_t depthDiff);

		const Element& operator[](uint64_t index) const;

		bool operator==(const HashPath& other) const;
		bool operator!=(const HashPath& other) const;
	public:
		Iterator begin() const;
		Iterator end() const;
	private:
		std::vector<Element> m_elements;
	};

	typedef const HashPath& HashPathRef;

	HashPath::Element::Element(const std::string& name)
		: m_asString(name), m_asHash(makeHash(name))
	{
	}

	bool HashPath::Element::operator==(const Element& other) const
	{
		return m_asHash == other.m_asHash;
	}

	bool HashPath::Element::operator!=(const Element& other) const
	{
		return !(*this == other);
	}

	HashPath::Iterator::Iterator(std::vector<Element>::const_iterator it)
		: m_it(it)
	{
	}

	bool HashPath::Iterator::operator!=(const Iterator& other) const
	{
		return m_it != other.m_it;
	}

	HashPath::Iterator& HashPath::Iterator::operator++()
	{
		++m_it;
		return *this;
	}

	const HashPath::Element& HashPath::Iterator::operator*() const
	{
		return *m_it;
	}

	HashPath::HashPath(const std::string& path)
	{
		auto formatted = formatPath(path);
		uint64_t begin = 0;
		uint64_t end = 0;
		do
		{
			end = formatted.find_first_of("/", begin + 1);
			uint64_t count = end - begin;
			if (count > 0)
				*this += Element(formatted.substr(begin, count));
			begin = end + 1;
		} while (end != std::string::npos);
	}

	uint64_t HashPath::depth() const
	{
		return m_elements.size();
	}

	HashPath HashPath::parent() const
	{
		return *this - 1;
	}

	HashPath HashPath::child(const Element& elem) const
	{
		return *this + elem;
	}

	std::string HashPath::getRealPath(const std::string& basePath) const
	{
		// TODO: Implement getRealPath
		return "";
	}

	std::string HashPath::formatPath(std::string path)
	{
		uint64_t i = 0;
		while ((i = path.find('\\')) != std::string::npos)
			path[i] = '/';
		while ((i = path.find("//")) != std::string::npos)
			path.erase(path.begin() + i + 1);
		return path;
	}

	HashPath HashPath::operator+(const HashPath& other) const
	{
		HashPath copy = *this;
		copy += other;
		return copy;
	}

	HashPath& HashPath::operator+=(const HashPath& other)
	{
		
		for (auto& elem : other)
			*this += elem;
		return *this;
	}

	HashPath HashPath::operator+(const Element& elem) const
	{
		HashPath copy = *this;
		copy += elem;
		return copy;
	}

	HashPath& HashPath::operator+=(const Element& elem)
	{
		static const auto stepOutElem = Element("..");

		if (elem == stepOutElem && depth() > 0)
			*this -= 1;
		else
			m_elements.push_back(elem);

		return *this;
	}

	HashPath HashPath::operator-(uint64_t depthDiff) const
	{
		HashPath copy = *this;
		copy -= depthDiff;
		return copy;
	}

	HashPath& HashPath::operator-=(uint64_t depthDiff)
	{
		while (depthDiff-- > 0)
			m_elements.pop_back();
		return *this;
	}

	const HashPath::Element& HashPath::operator[](uint64_t index) const
	{
		return m_elements[index];
	}

	bool HashPath::operator==(const HashPath& other) const
	{
		uint64_t maxDepth = std::min(depth(), other.depth());
		for (uint64_t i = 0; i < maxDepth; ++i)
			if ((*this)[i] != other[i])
				return false;

		return true;
	}

	bool HashPath::operator!=(const HashPath& other) const
	{
		return !(*this == other);
	}

	HashPath::Iterator HashPath::begin() const
	{
		return m_elements.begin();
	}

	HashPath::Iterator HashPath::end() const
	{
		return m_elements.end();
	}
}