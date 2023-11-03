file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_library(ub_stm32_usb_device INTERFACE)
add_library(UB::STM32::USBDevice ALIAS ub_stm32_usb_device)

target_sources(ub_stm32_usb_device INTERFACE ${SOURCES})
target_include_directories(ub_stm32_usb_device INTERFACE include)

target_link_libraries(ub_stm32_usb_device INTERFACE UB::USBD::Base)
