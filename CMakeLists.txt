cmake_minimum_required(VERSION 3.25)
project(microbits2)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_TESTS ON)
set(TEST_PYTHON_LIB "${CMAKE_CURRENT_SOURCE_DIR}/tools/test/lib")
file(GLOB_RECURSE TEST_PYTHON_FILES "${TEST_PYTHON_LIB}/*.py")
enable_testing()

add_subdirectory(src/crypto)
