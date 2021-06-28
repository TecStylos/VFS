#include "VFS.h"

#include <iostream>

int main()
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
			std::cout << "  Bot hashes are different!" << std::endl;
	}

	return 0;
}