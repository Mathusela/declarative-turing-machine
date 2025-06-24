cmake_minimum_required(VERSION 3.28.0)
project(decl-tm LANGUAGES CXX)

add_executable(unary_to_binary)

add_executable(flip_least_significant)

set_target_properties(unary_to_binary flip_least_significant PROPERTIES
	CXX_STANDARD 26
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(unary_to_binary PRIVATE "include/")
target_include_directories(flip_least_significant PRIVATE "include/")

add_subdirectory("src/")
