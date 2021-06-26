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
			const std::string& asString() cosnt { return m_asString; }
		private:
			uint64_t m_asHash;
			std::string m_asString;
		};
		class Iterator : public std::vector<Element>::const_iterator
		{
		public:
			using std::vector<Element>::vector;
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

		bool operator==(const HashPath& other) const;
		bool operator!=(const HashPath& other) const;
	public:
		Iterator begin() const;
		Iterator end() const;
	private:
		std::vector<Element> m_elements;
	};

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
	}

	bool HashPath::operator==(const HashPath& other) const
	{
		// TODO: Implement equal-operator
	}

	bool HashPath::operator!=(const HashPath& other) const
	{
		// TODO: Implement non-equal-operator
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