cmake_minimum_required(VERSION 3.28.0)
project(decl-tm LANGUAGES CXX)

add_executable(unary_to_binary)

set_target_properties(unary_to_binary PROPERTIES
	CXX_STANDARD 26
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(unary_to_binary PRIVATE "include/")

add_subdirectory("src/")
