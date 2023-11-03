file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_library(ub_usbd_base INTERFACE)
add_library(UB::USBD::Base ALIAS ub_usbd_base)

target_sources(ub_usbd_base INTERFACE ${SOURCES})
target_include_directories(ub_usbd_base INTERFACE include)
target_link_libraries(ub_usbd_base INTERFACE UB::Utilities)

file(REAL_PATH "tools/descriptor.py" DESCRIPTOR_GEN)
set_target_properties(ub_usbd_base PROPERTIES UB_USBD_DESCRIPTOR_GEN "${DESCRIPTOR_GEN}")

include(tools/descriptor.cmake)
