#pragma once

#include <string>
#include <fstream>

namespace VFS {

	class AbstractFileIO
	{
	public:
		AbstractFileIO() = delete;
	public:
		static void read(const std::string& path, void* buffer, uint64_t size, uint64_t offset = 0);
		static void write(const std::string& path, const void* buffer, uint64_t size, uint64_t offset = 0);
		static void make(const std::string& path);
		static void remove(const std::string& path);
		static void resize(const std::string& path, uint64_t newSize);
	private:
		;
	};
}