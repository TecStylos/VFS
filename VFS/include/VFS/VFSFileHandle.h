#pragma once

#include "VFSErrorCodes.h"
#include "VFSHashPath.h"

namespace VFS {

	class FileHandle
	{
	public:
		FileHandle() = default;
		FileHandle(HashPathRef path);
	public:
		ErrCode setLastModTime(uint64_t modTime);
		ErrCode writeFile(void* buffer, uint64_t count, uint64_t offset = 0);
		ErrCode readFile(void* buffer, uint64_t count, uint64_t offset = 0);
		ErrCode resizeFile(uint64_t newSize);
	public:
		operator bool() const;
	private:
		bool m_isValid = false;
		HashPath m_path;
	};

	FileHandle::FileHandle(HashPathRef path)
		: m_path(path)
	{
	}
}