cmake_minimum_required(VERSION 3.23)

set(UB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(UB_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

if ("${ENABLE_PLAYGROUND}" AND (EXISTS "${UB_ROOT}/playground/toolchain.cmake"))
    include("${UB_ROOT}/playground/toolchain.cmake")
endif ()

define_property(GLOBAL PROPERTY UB_ROOT)
set_property(GLOBAL PROPERTY UB_ROOT "${UB_ROOT}")

include(cmake/device_testing.cmake)
include(cmake/tools.cmake)
include(cmake/install.cmake)

project(microbits2
        LANGUAGES C CXX ASM
        VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX "")

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

add_subdirectory(src/utilities)

add_subdirectory(src/crypto)

add_subdirectory(src/usb/device-base)
add_subdirectory(src/usb/device-serial)

add_subdirectory(src/stm32/utilities)
add_subdirectory(src/stm32/usb-device)

ub_install_finalize()

if ("${ENABLE_PLAYGROUND}")
    add_subdirectory(playground)
endif ()
