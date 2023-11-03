function(ub_usbd_generate_descriptor TARGET SPECIFICATION)
    file(REAL_PATH "${SPECIFICATION}" SPECIFICATION)
    get_filename_component(SPECIFICATION_NAME "${SPECIFICATION}" NAME)
    get_filename_component(SPECIFICATION_BASENAME "${SPECIFICATION}" NAME_WLE)

    function(list_add_if_not_present list elem)
        list(FIND "${list}" "${elem}" exists)
        if(exists EQUAL -1)
            list(APPEND "${list}" "${elem}")
            set("${list}" "${${list}}" PARENT_SCOPE)
        endif()
    endfunction()

    macro(_target_get_linked_libraries_in _target _outlist)
        list_add_if_not_present("${_outlist}" "${_target}")

        # get libraries
        get_target_property(target_type "${_target}" TYPE)
        if (${target_type} STREQUAL "INTERFACE_LIBRARY")
            get_target_property(libs "${_target}" INTERFACE_LINK_LIBRARIES)
        else()
            get_target_property(libs "${_target}" LINK_LIBRARIES)
        endif()

        foreach(lib IN LISTS libs)
            if(NOT TARGET "${lib}")
                continue()
            endif()

            list(FIND "${_outlist}" "${lib}" exists)
            if(NOT exists EQUAL -1)
                continue()
            endif()

            _target_get_linked_libraries_in("${lib}" "${_outlist}")
        endforeach()
    endmacro()

    set(DEPENDENCIES "${TARGET}")
    _target_get_linked_libraries_in("${TARGET}" DEPENDENCIES)

    foreach(lib IN LISTS DEPENDENCIES)
        get_target_property(desc_lib "${lib}" UB_USBD_DESCRIPTOR_LIB)
        if (NOT ("${desc_lib}" STREQUAL "desc_lib-NOTFOUND"))
            list(APPEND PYTHON_PLUGINS "${desc_lib}")
        endif ()

        get_target_property(desc_gen "${lib}" UB_USBD_DESCRIPTOR_GEN)
        if (NOT ("${desc_gen}" STREQUAL "desc_gen-NOTFOUND"))
            set(PYTHON_FILE "${desc_gen}")
        endif ()
    endforeach ()

    if (NOT DEFINED PYTHON_FILE)
        message(FATAL_ERROR "No USB library is detected for target ${TARGET}, unable to generate descriptor")
    endif ()

    get_filename_component(PYTHON_DIR "${PYTHON_FILE}" DIRECTORY)
    file(GLOB_RECURSE PYTHON_LIB_FILES "${PYTHON_DIR}/*.py")

    set(GENERATED_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SPECIFICATION_BASENAME}.cpp")

    add_custom_command(
            OUTPUT "${GENERATED_FILE}"
            COMMAND python3 -B "${PYTHON_FILE}"
                --out "${GENERATED_FILE}"
                --spec "${SPECIFICATION}"
                --lib ${PYTHON_PLUGINS}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            DEPENDS ${PYTHON_LIB_FILES} "${SPECIFICATION}" ${PYTHON_PLUGINS}
            COMMENT "Generating USB descriptor set from ${SPECIFICATION_NAME} for target ${TARGET}..."
    )

    target_sources("${TARGET}" PRIVATE "${GENERATED_FILE}")
endfunction()
