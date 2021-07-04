#pragma once

#include "VFSHashPath.h"

namespace VFS {

	class FileSystem
	{
	public:
		enum class ErrCode
		{
			Success,
			PathNotFound,
			NotAFile,
			NotADirectory
		};
	public:
		;
	public:
		ErrCode createFile(HashPathRef path);
		ErrCode deleteFile(HashPathRef path);
		ErrCode createDirectory(HashPathRef path);
		ErrCode deleteDirectory(HashPathRef path);
		ErrCode setLastModTime(HashPathRef path, uint64_t modTime);
		ErrCode writeFile(HashPathRef path, void* buffer, uint64_t count, uint64_t offset = 0);
		ErrCode readFile(HashPathRef path, void* buffer, uint64_t count, uint64_t offset = 0);
		ErrCode resizeFile(HashPathRef path, uint64_t newSize);
	public:
		void setBasePath(const std::string& basePath);
		const std::string& getBasePath() const;
	protected:
		virtual std::string getRealPath(HashPathRef path) = 0;
	private:
		std::string m_basePath;
	};
}