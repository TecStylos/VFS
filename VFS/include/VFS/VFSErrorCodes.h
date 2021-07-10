#pragma once

namespace VFS {

	enum class ErrCode
	{
		Success = 0,
		PathNotFound,
		NotAFile,
		NotADirectory
	};
}