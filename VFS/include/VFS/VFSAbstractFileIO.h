#pragma once

#include <string>
#include <fstream>
#include <unordered_map>

namespace VFS {

	class AbstractFileIO
	{
	public:
		enum class ErrCode
		{
			Success = 0,
			CannotAccessFile
		};
		struct Error
		{
			ErrCode code;
			union
			{
				uint64_t nRead;
				uint64_t nWritten;
			} value;
		public:
			Error(ErrCode ec)
				: code(ec)
			{}
		};
	private:
		typedef std::fstream* StreamPtr;
	public:
		AbstractFileIO(uint64_t nConcurrentStreams = 1);
	public:
		Error read(const std::string& path, void* buffer, uint64_t size, uint64_t offset = 0);
		Error write(const std::string& path, const void* buffer, uint64_t size, uint64_t offset = 0);
	private:
		StreamPtr getStream(const std::string& path);
	private:
		std::unordered_map<std::string, std::fstream> m_streams;
		const uint64_t m_nMaxStreams;
	};

	AbstractFileIO::AbstractFileIO(uint64_t nConcurrentStreams)
		: m_nMaxStreams(nConcurrentStreams)
	{}

	AbstractFileIO::Error AbstractFileIO::read(const std::string& path, void* buffer, uint64_t size, uint64_t offset)
	{
		StreamPtr stream = getStream(path);
		if (!stream)
			return ErrCode::CannotAccessFile;

		stream->seekg(offset);
		stream->read((char*)buffer, size);
		// TODO: Check for read errors

		return ErrCode::Success;
	}

	AbstractFileIO::Error AbstractFileIO::write(const std::string& path, const void* buffer, uint64_t size, uint64_t offset)
	{
		StreamPtr stream = getStream(path);
		if (!stream)
			return ErrCode::CannotAccessFile;

		stream->seekp(offset);
		stream->write((const char*)buffer, size);
		// TODO: Check for write errors

		return ErrCode::Success;
	}

	AbstractFileIO::StreamPtr AbstractFileIO::getStream(const std::string& path)
	{
		auto it = m_streams.find(path);
		if (it != m_streams.end())
			return &it->second;

		if (m_streams.size() == m_nMaxStreams)
			m_streams.erase(m_streams.begin());

		std::fstream s(path, std::ios::binary | std::ios::in | std::ios::out);
		if (!s.good())
			return nullptr;

		m_streams.insert(std::make_pair(path, s));

		return getStream(path);
	}
}