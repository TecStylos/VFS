#pragma once

namespace VFS {

	enum class Platform
	{
		None,
		Windows,
		Unix
	};

	class PlatformManager
	{
	private:
		PlatformManager();
	private:
		static PlatformManager s_man;
		static inline Platform s_platform = Platform::None;
		friend Platform getPlatform();
	};

	Platform getPlatform()
	{
		return PlatformManager::s_platform;
	}

}