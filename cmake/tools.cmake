set(USB_COMPILER_ROOT "${UB_ROOT}/tools/usb-descriptor-compiler")
set(USB_COMPILER_BIN "${UB_BINARY_DIR}/usb-descriptor-compiler.jar")

file(GLOB_RECURSE USB_COMPILER_SOURCES
        "${USB_COMPILER_ROOT}/*.gradle*"
        "${USB_COMPILER_ROOT}/src/*.java")

add_custom_command(
        OUTPUT "${USB_COMPILER_BIN}"
        DEPENDS "${USB_COMPILER_SOURCES}"
        WORKING_DIRECTORY "${USB_COMPILER_ROOT}"
        COMMENT "Compiling USB descriptor compiler classes..."
        COMMAND "./gradlew" "-PcmakeOutputJar=${USB_COMPILER_BIN}" "cmakeJar"
)

add_custom_target(ub_usb_descriptor_compiler ALL DEPENDS "${USB_COMPILER_BIN}")
install(FILES "${USB_COMPILER_BIN}" TYPE BIN)

define_property(GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER)
set_property(GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER "${USB_COMPILER_BIN}")

define_property(GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER_TARGET)
set_property(GLOBAL PROPERTY UB_USB_DESCRIPTOR_COMPILER_TARGET ub_usb_descriptor_compiler)