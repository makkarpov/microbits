#ifndef UB_TEST_DEVICE_TEST_H
#define UB_TEST_DEVICE_TEST_H

#include <cstdint>
#include <cstddef>

#define TEST_MAIN(ret, ...)         extern "C" ret test_main(__VA_ARGS__)
#define TEST_IO_VARIABLE(...)       extern "C" __VA_ARGS__; __attribute__((used)) __VA_ARGS__


#endif // UB_TEST_DEVICE_TEST_H
