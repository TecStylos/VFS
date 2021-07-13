cmake_minimum_required(VERSION 3.8)

project("Sandbox")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(Sandbox "Sandbox.cpp" "VFS/include/VFS/VFSAbstractFileIO.h" "VFS/include/VFS/VFSMapStream.h")

target_include_directories(
	Sandbox PUBLIC "VFS/include"
)