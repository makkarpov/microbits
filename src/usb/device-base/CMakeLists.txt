file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_library(ub_usbd_base INTERFACE)
add_library(UB::USBD::Base ALIAS ub_usbd_base)

target_sources(ub_usbd_base INTERFACE ${SOURCES})
target_include_directories(ub_usbd_base INTERFACE include)
target_link_libraries(ub_usbd_base INTERFACE UB::Utilities)

ub_install_library(UB::USBD::Base EXTRA_CMAKE_LISTS descriptors.cmake)
include(descriptors.cmake)
