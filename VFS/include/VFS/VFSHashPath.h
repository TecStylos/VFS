#pragma once

#include <string>
#include <vector>

namespace VFS {

	class HashPath
	{
	public:
		struct Element
		{
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
		private:
			std::vector<Element>::const_iterator m_it;
		};
	public:
		uint64_t depth() const;
		HashPath parent() const;
		HashPath child(const Element& elem) const;
		std::string getRealPath(const std::string& basePath = "") const;
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

	HashPath HashPath::operator+(const HashPath& other) const
	{
		HashPath copy = *this;
		copy += other;
		return copy;
	}

	HashPath& HashPath::operator+=(const HashPath& other)
	{
		// TODO: Implement hash path concatenation
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
		// TODO: Implement hash path - element concatenation
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
		// TODO: Implement hash path reduction
		return *this;
	}

	const HashPath::Element& HashPath::operator[](uint64_t index) const
	{
		// TODO: Implement index op
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