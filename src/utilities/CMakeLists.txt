file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_library(ub_utilities INTERFACE)
add_library(UB::Utilities ALIAS ub_utilities)

target_sources(ub_utilities INTERFACE ${SOURCES})
target_include_directories(ub_utilities INTERFACE include)

ub_install_library(UB::Utilities)
