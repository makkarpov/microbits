if (NOT DEFINED ENABLE_DEVICE_TESTING)
    set(ENABLE_DEVICE_TESTING OFF)
endif ()

if (NOT "${ENABLE_DEVICE_TESTING}")
    return()
endif ()

message(STATUS "Testing on device is enabled, setting toolchain to ARM")

# Toolchain file is from https://github.com/ObKo/stm32-cmake/blob/master/cmake/stm32_gcc.cmake
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(TARGET_TRIPLET arm-none-eabi)

find_program(CMAKE_C_COMPILER NAMES "${TARGET_TRIPLET}-gcc" HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_CXX_COMPILER NAMES "${TARGET_TRIPLET}-g++" HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_ASM_COMPILER NAMES "${TARGET_TRIPLET}-gcc" HINTS ${TOOLCHAIN_BIN_PATH})

get_filename_component(DEV_TOOLCHAIN_PATH "${CMAKE_C_COMPILER}" DIRECTORY)
get_filename_component(DEV_TOOLCHAIN_PATH "${DEV_TOOLCHAIN_PATH}" DIRECTORY)

set(CMAKE_EXECUTABLE_SUFFIX_C   .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)
set(CMAKE_SYSTEM_NAME           Generic)
set(CMAKE_SYSTEM_PROCESSOR      arm)
set(CMAKE_SYSROOT               "${DEV_TOOLCHAIN_PATH}/${TARGET_TRIPLET}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

function(configure_device_executable TARGET)
    target_compile_options("${TARGET}" PRIVATE
            "-mthumb" "-mcpu=cortex-m33" "-fno-unwind-tables" "-ffunction-sections" "-fdata-sections"
            "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>"
            "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>")

    target_link_options("${TARGET}" PRIVATE
            "-mthumb" "-mcpu=cortex-m33" "-Wl,--gc-sections"
            "--specs=nano.specs" "--specs=nosys.specs")
endfunction()

function(add_device_test)
    set(DT_FLAGS "")
    set(DT_OPTS_1 "NAME")
    set(DT_OPTS_N "SOURCES;CC_FLAGS;LD_FLAGS;DEFINES;LIBRARIES;RUNNER")
    cmake_parse_arguments(DT "${DT_FLAGS}" "${DT_OPTS_1}" "${DT_OPTS_N}" ${ARGN})

    if (NOT DEFINED DT_NAME)
        message(FATAL_ERROR "NAME option is required for add_device_test()")
        return()
    endif ()

    if (NOT DEFINED DT_SOURCES)
        message(FATAL_ERROR "SOURCES option is required for add_device_test()")
        return()
    endif ()

    set(TGT_NAME "test_dev_${DT_NAME}_bin")

    add_executable("${TGT_NAME}" ${DT_SOURCES})

    target_include_directories("${TGT_NAME}" PRIVATE "${UB_ROOT}/tools/test/device_lib")
    configure_device_executable("${TGT_NAME}")

    set(DT_LINKER_SCRIPT "${UB_ROOT}/tools/test/device_lib/stm32_ram.ld")
    target_link_options("${TGT_NAME}" PRIVATE "-T${DT_LINKER_SCRIPT}" "-nostartfiles")

    if (DEFINED DT_CC_FLAGS)
        target_compile_options("${TGT_NAME}" PRIVATE ${DT_CC_FLAGS})
    endif ()

    if (DEFINED DT_LD_FLAGS)
        target_link_options("${TGT_NAME}" PRIVATE ${DT_LD_FLAGS})
    endif ()

    if (DEFINED DT_DEFINES)
        target_compile_definitions("${TGT_NAME}" PRIVATE ${DT_DEFINES})
    endif ()

    if (DEFINED DT_LIBRARIES)
        target_link_libraries("${TGT_NAME}" PRIVATE ${DT_LIBRARIES})
    endif ()

    list(POP_FRONT DT_RUNNER DT_RUNNER_FILE)
    file(REAL_PATH "${DT_RUNNER_FILE}" DT_RUNNER_ABS)

    set(DT_PYTHON_LIB "${UB_ROOT}/tools/test/lib")

    set(DT_PYTHON_COMMAND "${CMAKE_COMMAND}" -E env
            "PYTHONPATH=${DT_PYTHON_LIB}"
            python3 -B "${DT_PYTHON_LIB}/devtest/application.py"
            --name "${DT_NAME}"
            --executable "$<TARGET_FILE:${TGT_NAME}>"
            --runner "${DT_RUNNER_ABS}"
            --runner_args ${DT_RUNNER})

    add_custom_target(
            "test_${DT_NAME}"
            DEPENDS "${TGT_NAME}"
            COMMAND ${DT_PYTHON_COMMAND}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_test(
            NAME "${DT_NAME}"
            COMMAND ${DT_PYTHON_COMMAND}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endfunction()
