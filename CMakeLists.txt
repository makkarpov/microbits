cmake_minimum_required(VERSION 3.25)

set(UB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(UB_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

include(cmake/device_testing.cmake)

project(microbits2 LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED ENABLE_TESTING)
    set(ENABLE_TESTING OFF)
endif ()

if ("${ENABLE_DEVICE_TESTING}")
    if ("${ENABLE_TESTING}")
        message(WARNING "ENABLE_DEVICE_TESTING is set, disabling ENABLE_TESTING")
        set(ENABLE_TESTING OFF)
    endif ()

    enable_testing()
endif ()

if ("${ENABLE_TESTING}")
    set(TEST_PYTHON_LIB "${CMAKE_CURRENT_SOURCE_DIR}/tools/test/lib")
    file(GLOB_RECURSE TEST_PYTHON_FILES "${TEST_PYTHON_LIB}/*.py")
    enable_testing()
endif ()

add_subdirectory(src/crypto)

if ("${ENABLE_PLAYGROUND}")
    add_subdirectory(playground)
endif ()
