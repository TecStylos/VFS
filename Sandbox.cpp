#include "VFS.h"

#include <iostream>

#include <filesystem>
#include <unordered_map>

void compareInputStrings()
{
	std::string str1, str2;
	VFS::Hash hash1, hash2;

	while (true)
	{
		std::cout << "Enter string #1: ";
		std::getline(std::cin, str1);
		std::cout << "Enter string #2: ";
		std::getline(std::cin, str2);

		hash1 = VFS::makeHash(str1);
		hash2 = VFS::makeHash(str2);

		if (hash1 == hash2)
			std::cout << "  Both hashes are equal!" << std::endl;
		else
			std::cout << "  Both hashes are different!" << std::endl;
	};
}

void scanDrivePathsAndCompare()
{
	std::unordered_map<VFS::Hash, std::string> hashMap;

	uint64_t nClashes = 0;
	uint64_t nScanned = 0;
	uint64_t nSameName = 0;

	auto checkName = [&nScanned, &hashMap, &nClashes, &nSameName](const std::string& name)
	{
		++nScanned;

		auto hash = VFS::makeHash(name);

		auto mapIt = hashMap.find(hash);
		if (mapIt != hashMap.end())
		{
			if (name != mapIt->second)
			{
				std::cout << "  Found clash: '" << name << "' && '" << mapIt->second << "' --> " << hash << std::endl;
				++nClashes;
			}
			else
			{
				++nSameName;
			}
			return;
		}

		hashMap.insert(std::make_pair(hash, name));
	};

	auto scanDrive = [checkName](const std::string& drivepath)
	{
		try
		{
			auto it = std::filesystem::recursive_directory_iterator(drivepath, std::filesystem::directory_options::skip_permission_denied);
			while (it != std::filesystem::recursive_directory_iterator())
			{
				checkName(it->path().filename().u8string());
				try
				{
					++it;
				}
				catch (...)
				{
					;
				}
			}
		}
		catch (...)
		{
			;
		}
	};

	std::error_code ec;
	std::cout << "Scanning drive 'C:\\' ..." << std::endl;
	scanDrive("C:\\");
	std::cout << "Scanning drive 'D:\\' ..." << std::endl;
	scanDrive("D:\\");
	std::cout << "Scanning drive 'E:\\' ..." << std::endl;
	scanDrive("E:\\");

	std::cout << "////////////////////////////////////////////////////////////" << std::endl;
	std::cout << "-- Scanned " << nScanned << " file-/dirnames." << std::endl;
	std::cout << "-- Found " << nClashes << " clashes." << std::endl;
	std::cout << "-- #DiffHashes: " << hashMap.size() << std::endl;
	std::cout << "-- #SameName: " << nSameName << std::endl;
}

void testAFIO()
{
	auto afio = VFS::AbstractFileIO::create(2);
	auto f1Str = "C:\\dev\\proj\\VFS\\testdir\\file1.txt";
	auto f2Str = "C:\\dev\\proj\\VFS\\testdir\\file2.txt";
	auto f3Str = "C:\\dev\\proj\\VFS\\testdir\\file3.txt";
	if (!afio->exists(f1Str))
		afio->make(f1Str);
	afio->write(f1Str, "Hello world 1", 13);
	if (!afio->exists(f2Str))
		afio->make(f2Str);
	afio->write(f2Str, "Hello world 2", 13);
	if (!afio->exists(f3Str))
		afio->make(f3Str);
	afio->write(f3Str, "Hello world 3", 13);

	char buff[14] = { 0 };
	afio->read("C:\\dev\\proj\\VFS\\testdir\\file1.txt", buff, 13);
	std::cout << "Text read from file1.txt: " << buff << std::endl;
}

void testMapStream()
{
	auto afio = VFS::AbstractFileIO::create(2);
	uint64_t keySize = sizeof(uint64_t);
	uint64_t valSize = sizeof(uint64_t);
	VFS::MapStream ms("C:\\dev\\proj\\VFS\\testdir\\MapStreamTest.msf", afio, keySize, valSize);

	for (uint64_t key = 0; key < 1000; ++key)
	{
		uint64_t value = key * key;
		ms.insert(&key, &value);
	}
	ms.optimize();
	ms.flush();

	uint64_t keyToSearchFor = 500;
	uint64_t index = ms.find(&keyToSearchFor);
	uint64_t retrievedValue;
	ms.getValue(index, &retrievedValue);
	std::cout << "Found key at index " << index << " with value " << retrievedValue << std::endl;
}

int main()
{
	//compareInputStrings();

	//scanDrivePathsAndCompare();

	//testAFIO();

	testMapStream();

	return 0;
}