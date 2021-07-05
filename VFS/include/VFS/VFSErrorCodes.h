#pragma once

namespace VFS {

	enum class ErrCode
	{
		Success = 0,
		PathNotFound,
		NotAFile,
		NotADirectory
	};

	operator bool(ErrCode ec)
	{
		return ec != ErrCode::Success;
	}
}