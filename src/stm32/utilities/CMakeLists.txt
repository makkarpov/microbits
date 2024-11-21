file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_library(ub_stm32_utilities INTERFACE)
add_library(UB::STM32::Utilities ALIAS ub_stm32_utilities)

target_sources(ub_stm32_utilities INTERFACE ${SOURCES})
target_include_directories(ub_stm32_utilities INTERFACE include)

ub_install_library(UB::STM32::Utilities)
