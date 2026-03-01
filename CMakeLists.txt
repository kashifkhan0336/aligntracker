cmake_minimum_required(VERSION 3.20)
project(
	aligntracker
	VERSION 1.0
	LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(src)
add_subdirectory(prototype)