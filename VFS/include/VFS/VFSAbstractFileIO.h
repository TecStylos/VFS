#pragma once

#include <string>
#include <fstream>
#include <unordered_map>
#include <mutex>

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

		class LockedStream;

		struct LockableStream
		{
		public:
			LockableStream(std::fstream&& stream)
			{
				m_stream.swap(stream);
			}
			LockableStream(LockableStream&& other)
			{
				m_stream.swap(other.m_stream);
			}
		private:
			std::mutex m_mtx;
			std::fstream m_stream;
		public:
			friend LockedStream;
		};

		class LockedStream
		{
		public:
			LockedStream(LockableStream* stream)
				: m_pStream(stream) { if (m_pStream) m_pStream->m_mtx.lock(); }
			~LockedStream() { if (m_pStream) m_pStream->m_mtx.unlock(); }
		public:
			std::fstream* operator->() { return &m_pStream->m_stream; }
		public:
			operator bool() const { return m_pStream != nullptr; }
		private:
			LockableStream* m_pStream;
		};
	private:
		typedef std::fstream* StreamPtr;
	public:
		AbstractFileIO(uint64_t nConcurrentStreams = 1);
	public:
		Error read(const std::string& path, void* buffer, uint64_t size, uint64_t offset = 0);
		Error write(const std::string& path, const void* buffer, uint64_t size, uint64_t offset = 0);
		uint64_t closeMatchingStreams(const std::string& path);
	private:
		LockedStream getStream(const std::string& path);
	private:
		std::unordered_map<std::string, LockableStream> m_streams;
		std::mutex m_mtxStreams;
		const uint64_t m_nMaxStreams;
	};

	AbstractFileIO::AbstractFileIO(uint64_t nConcurrentStreams)
		: m_nMaxStreams(nConcurrentStreams)
	{}

	AbstractFileIO::Error AbstractFileIO::read(const std::string& path, void* buffer, uint64_t size, uint64_t offset)
	{
		LockedStream stream = getStream(path);
		if (!stream)
			return ErrCode::CannotAccessFile;

		stream->seekg(offset);
		stream->read((char*)buffer, size);
		// TODO: Check for read errors

		return ErrCode::Success;
	}

	AbstractFileIO::Error AbstractFileIO::write(const std::string& path, const void* buffer, uint64_t size, uint64_t offset)
	{
		LockedStream stream = getStream(path);
		if (!stream)
			return ErrCode::CannotAccessFile;

		stream->seekp(offset);
		stream->write((const char*)buffer, size);
		// TODO: Check for write errors

		return ErrCode::Success;
	}

	uint64_t AbstractFileIO::closeMatchingStreams(const std::string& path)
	{
		uint64_t nClosed = 0;

		std::lock_guard lock(m_mtxStreams);

		for (auto& it : m_streams)
		{
			if (it.first.find(path) == 0)
			{
				m_streams.erase(it.first);
				++nClosed;
			}
		}

		return nClosed;
	}

	AbstractFileIO::LockedStream AbstractFileIO::getStream(const std::string& path)
	{
		{
			std::lock_guard lock(m_mtxStreams);

			auto it = m_streams.find(path);
			if (it != m_streams.end())
				return &it->second;

			if (m_streams.size() == m_nMaxStreams)
				m_streams.erase(m_streams.begin());

			LockableStream stream(std::fstream(path, std::ios::binary | std::ios::in | std::ios::out));
			if (!LockedStream(&stream)->good())
				return nullptr;

			m_streams.insert(std::make_pair(path, stream));
		}

		return getStream(path);
	}
}