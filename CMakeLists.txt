cmake_minimum_required(VERSION 3.8)

project("Sandbox")

add_executable(Sandbox "Sandbox.cpp")

target_include_directories(
	Sandbox PUBLIC "VFS/include"
)