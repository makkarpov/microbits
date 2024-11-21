cmake_minimum_required(VERSION @CMAKE_MINIMUM_REQUIRED_VERSION@)

project(microbits
        LANGUAGES C CXX ASM
        VERSION @CMAKE_PROJECT_VERSION@)

# UB_ROOT property holds the base directory of the microbits library
define_property(GLOBAL PROPERTY UB_ROOT)
set_property(GLOBAL PROPERTY UB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
