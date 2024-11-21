define_property(GLOBAL PROPERTY UB_INSTALLED_LIBS)
set(UB_CMAKE_TMP_DIR "${UB_BINARY_DIR}/cmake-install-generated")
file(MAKE_DIRECTORY "${UB_CMAKE_TMP_DIR}")

function(_ub_write_file_list out_var target files)
    set(out "target_sources(${target} INTERFACE\n")
    set(first 1)

    list(SORT files)

    foreach(f IN LISTS files)
        if ("${first}" EQUAL 0)
            string(APPEND out "\n")
        endif ()

        cmake_path(RELATIVE_PATH f OUTPUT_VARIABLE fr)
        string(APPEND out "    \"${fr}\"")
        set(first 0)
    endforeach ()

    string(APPEND out ")\n\n")
    set("${out_var}" "${${out_var}}${out}" PARENT_SCOPE)
endfunction()

function(_ub_append_extra_lists out_var files)
    set(out "")

    foreach(f IN LISTS files)
        file(READ "${f}" fc)
        string(STRIP "${fc}" fc)
        string(APPEND out "# === ${f} ===\n${fc}\n\n")
    endforeach ()

    set("${out_var}" "${${out_var}}${out}" PARENT_SCOPE)
endfunction()

function(_ub_make_lib_path out_var ns_name)
    # underscore regex logic:
    #  1. it must convert CamelCase into a snake_case
    #  2. it must split out abbreviations, i.e. "USBDevice" -> "usb_device"
    #  3. it must not introduce stray underscores at the edges, i.e. no "USB" -> "USB_" or "_USB"
    #  4. it must not introduce underscores before numerals, i.e. no "STM32" -> "STM_32"

    string(REGEX REPLACE "^UB::" "" path "${ns_name}") # everything here is UB::
    string(REPLACE "::" "/" path "${path}")
    string(REGEX REPLACE "([^/])([A-Z][a-z])" "\\1_\\2" path "${path}")
    string(TOLOWER "${path}" path)
    set("${out_var}" "lib/${path}" PARENT_SCOPE)
endfunction()

function(ub_install_library ns_target)
    cmake_parse_arguments(arg "" "" "EXTRA_CMAKE_LISTS" ${ARGN})

    get_property(target TARGET "${ns_target}" PROPERTY ALIASED_TARGET)
    _ub_make_lib_path(target_path "${ns_target}")

    set(cmake_tmp_file "${UB_CMAKE_TMP_DIR}/${target}.cmake")
    set(cmake_tmp "add_library(${target} INTERFACE)\nadd_library(${ns_target} ALIAS ${target})\n")

    set(inc_dir "${CMAKE_CURRENT_LIST_DIR}/include")
    install(DIRECTORY "${inc_dir}" DESTINATION "${target_path}")
    string(APPEND cmake_tmp "target_include_directories(${target} INTERFACE include)\n")

    get_property(deps TARGET "${target}" PROPERTY INTERFACE_LINK_LIBRARIES)
    if (NOT ("${deps}" STREQUAL ""))
        string(JOIN " " deps_str "${deps}")
        string(APPEND cmake_tmp "target_link_libraries(${target} INTERFACE ${deps_str})\n")
    endif ()

    set(src_dir "${CMAKE_CURRENT_LIST_DIR}/src")
    if (EXISTS "${src_dir}")
        install(DIRECTORY "${src_dir}" DESTINATION "${target_path}")
        get_property(src_files TARGET "${target}" PROPERTY INTERFACE_SOURCES)
        string(APPEND cmake_tmp "\n")
        _ub_write_file_list(cmake_tmp "${target}" "${src_files}")
    endif ()

    if (DEFINED arg_EXTRA_CMAKE_LISTS)
        _ub_append_extra_lists(cmake_tmp "${arg_EXTRA_CMAKE_LISTS}")
    endif ()

    file(WRITE "${cmake_tmp_file}" "${cmake_tmp}")
    install(FILES "${cmake_tmp_file}" DESTINATION "${target_path}" RENAME "CMakeLists.txt")
    set_property(GLOBAL APPEND PROPERTY UB_INSTALLED_LIBS "${target_path}")
endfunction()

function(ub_install_finalize)
    get_property(installed_libs GLOBAL PROPERTY UB_INSTALLED_LIBS)
    list(SORT installed_libs COMPARE NATURAL)

    file(READ "${UB_ROOT}/cmake/installed_cmakelists.cmake" cmake_tmp)
    string(STRIP "${cmake_tmp}" cmake_tmp)
    string(APPEND cmake_tmp "\n\n")
    string(CONFIGURE "${cmake_tmp}" cmake_tmp @ONLY)

    foreach(f IN LISTS installed_libs)
        string(APPEND cmake_tmp "add_subdirectory(${f})\n")
    endforeach ()

    set(cmake_file "${UB_CMAKE_TMP_DIR}/root_cmakelists.cmake")
    file(WRITE "${cmake_file}" "${cmake_tmp}")
    install(FILES "${cmake_file}" DESTINATION "." RENAME "CMakeLists.txt")
endfunction()
