cmake_minimum_required(VERSION 3.6)

project(zvfs)

set(ZVFS_ROOT_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(ZVFS_ROOT_PROJECT ON)

    # Enable solution folder support
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

option(ZVFS_BUILD_TESTS "Build tests" ${ZVFS_ROOT_PROJECT})

# Visual Studio generator specific flags
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    add_compile_options(/MP)
endif()

#Set default C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_subdirectory(zvfs-core)

# Tests
if(ZVFS_BUILD_TESTS)
    add_subdirectory(zvfs-tests)
    enable_testing()
    add_test(NAME zvfs-tests COMMAND "$<TARGET_FILE:zvfs-tests>")
    
    # Set Visual Studio startup project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}-tests)
endif()
