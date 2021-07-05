#pragma once

#include "VFSHashPath.h"
#include "VFSFileHandle.h"

namespace VFS {

	class FileSystem
	{
	public:
		FileSystem() = delete;
		FileSystem(const std::string& basePath);
	public:
		ErrCode createFile(HashPathRef path);
		ErrCode deleteFile(HashPathRef path);
		ErrCode createDirectory(HashPathRef path);
		ErrCode deleteDirectory(HashPathRef path);
		FileHandle getFileHandle(HashPathRef path);
	public:
		void setBasePath(const std::string& basePath);
		const std::string& getBasePath() const;
	protected:
		virtual std::string getRealPath(HashPathRef path) = 0;
	private:
		std::string m_basePath;
	};
}