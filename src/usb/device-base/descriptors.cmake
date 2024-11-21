# UB_USB_DESCRIPTOR_COMPILER_PLUGINS contains list of JAR files that should be loaded into the USB descriptor compiler.
# This list is initially empty and is intended to be populated by other libraries.
define_property(GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER_PLUGINS)

function(ub_usbd_generate_descriptor TARGET SPECIFICATION)
    get_property(compiler_jar GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER)
    get_property(ub_root GLOBAL PROPERTY UB_ROOT)

    if ("${compiler_jar}" STREQUAL "")
        set(compiler_jar "${ub_root}/bin/usb-descriptor-compiler.jar")
    endif ()

    file(REAL_PATH "${SPECIFICATION}" SPECIFICATION)
    get_filename_component(specification_name "${SPECIFICATION}" NAME)
    get_filename_component(specification_basename "${SPECIFICATION}" NAME_WLE)

    set(generated_file "${CMAKE_CURRENT_BINARY_DIR}/${specification_basename}.cpp")
    get_property(compiler_plugins GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER_PLUGINS)

    if (NOT ("${compiler_plugins}" STREQUAL ""))
        list(JOIN compiler_plugins ":" compiler_plugins)
        list(PREPEND compiler_plugins "--plugins")
    endif ()

    add_custom_command(
            OUTPUT "${generated_file}"
            COMMAND java -jar "${compiler_jar}" "${compiler_plugins}"
                --specification "${SPECIFICATION}"
                --out "${generated_file}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            DEPENDS "${compiler_jar}" "${SPECIFICATION}"
            COMMENT "Generating USB descriptor set from ${specification_name} for target ${TARGET}..."
    )

    target_sources("${TARGET}" PRIVATE "${generated_file}")

    get_property(compiler_tgt GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER_TARGET)
    if (NOT ("${compiler_tgt}" STREQUAL ""))
        add_dependencies("${TARGET}" "${compiler_tgt}")
    endif ()
endfunction()

function(ub_usbd_register_compiler_plugin FILE)
    cmake_path(ABSOLUTE_PATH FILE)
    set_property(GLOBAL APPEND PROPERTY UB_USB_DESCRIPTOR_COMPILER_PLUGINS "${FILE}")
endfunction()
