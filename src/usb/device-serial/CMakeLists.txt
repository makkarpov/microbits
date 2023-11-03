file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_library(ub_usbd_serial INTERFACE)
add_library(UB::USBD::Serial ALIAS ub_usbd_serial)

target_sources(ub_usbd_serial INTERFACE ${SOURCES})
target_include_directories(ub_usbd_serial INTERFACE include)
target_link_libraries(ub_usbd_serial INTERFACE UB::Utilities UB::USBD::Base)

file(REAL_PATH "tools/descriptor.py" DESCRIPTOR_LIB)
set_target_properties(ub_usbd_serial PROPERTIES UB_USBD_DESCRIPTOR_LIB "${DESCRIPTOR_LIB}")
