#include "VFS.h"

#include <iostream>

#include <filesystem>
#include <unordered_map>

int main()
{
	//std::string str1, str2;
	//VFS::Hash hash1, hash2;

	//while (true)
	//{
	//	std::cout << "Enter string #1: ";
	//	std::getline(std::cin, str1);
	//	std::cout << "Enter string #2: ";
	//	std::getline(std::cin, str2);

	//	hash1 = VFS::makeHash(str1);
	//	hash2 = VFS::makeHash(str2);

	//	if (hash1 == hash2)
	//		std::cout << "  Both hashes are equal!" << std::endl;
	//	else
	//		std::cout << "  Bot hashes are different!" << std::endl;
	//};

	std::unordered_map<VFS::Hash, std::string> hashMap;

	uint64_t nClashes = 0;
	uint64_t nScanned = 0;
	uint64_t nSameName = 0;

	auto checkName = [&nScanned, &hashMap, &nClashes, &nSameName](const std::filesystem::directory_entry& entry)
	{
		++nScanned;

		auto name = entry.path().filename().u8string();
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
		auto it = std::filesystem::recursive_directory_iterator(drivepath, std::filesystem::directory_options::skip_permission_denied);
		while (it != std::filesystem::recursive_directory_iterator())
		{
			checkName(*it);
			try
			{
				++it;
			}
			catch (...)
			{
				;
			}
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

	return 0;
}