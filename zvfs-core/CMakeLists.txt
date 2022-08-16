cmake_minimum_required(VERSION 3.6)

project(zvfs-core)

# Visual Studio generator specific flags
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    add_compile_options(/MP)
endif()

#Set default C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

#set(CMAKE_FOLDER "")

# Project target
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp *.hpp)

# Build as static library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Add include directory path
target_include_directories(${PROJECT_NAME} INTERFACE includes)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES})

# Enable all warnings and make warnings errors
if(MSVC)	
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()	
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
