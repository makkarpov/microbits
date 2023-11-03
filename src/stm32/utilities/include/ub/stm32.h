#ifndef UB_STM32_UTILITIES_STM32_H
#define UB_STM32_UTILITIES_STM32_H

/**
 * Universal STM32 include file which selects appropriate CMSIS and HAL includes based on currently active STM32 chip
 */

// CMSIS:

#if defined(STM32F0)
#include <stm32f0xx.h>
#elif defined(STM32F1)
#include <stm32f1xx.h>
#elif defined(STM32F2)
#include <stm32f2xx.h>
#elif defined(STM32F3)
#include <stm32f3xx.h>
#elif defined(STM32F4)
#include <stm32f4xx.h>
#elif defined(STM32F7)
#include <stm32f7xx.h>
#elif defined(STM32H7)
#include <stm32h7xx.h>
#elif defined(STM32L0)
#include <stm32l0xx.h>
#elif defined(STM32L1)
#include <stm32l1xx.h>
#elif defined(STM32L4)
#include <stm32l4xx.h>
#elif defined(STM32L5)
#include <stm32l5xx.h>
#elif defined(STM32G0)
#include <stm32g0xx.h>
#elif defined(STM32G4)
#include <stm32g4xx.h>
#elif defined(STM32WB)
#include <stm32wbxx.h>
#else
#error "STM32 family is not defined"
#endif

// HAL:

#if defined(USE_HAL_DRIVER)
#if defined(STM32F0)
#include <stm32f0xx_hal.h>
#elif defined(STM32F1)
#include <stm32f1xx_hal.h>
#elif defined(STM32F2)
#include <stm32f2xx_hal.h>
#elif defined(STM32F3)
#include <stm32f3xx_hal.h>
#elif defined(STM32F4)
#include <stm32f4xx_hal.h>
#elif defined(STM32F7)
#include <stm32f7xx_hal.h>
#elif defined(STM32H7)
#include <stm32h7xx_hal.h>
#elif defined(STM32L0)
#include <stm32l0xx_hal.h>
#elif defined(STM32L1)
#include <stm32l1xx_hal.h>
#elif defined(STM32L4)
#include <stm32l4xx_hal.h>
#elif defined(STM32L5)
#include <stm32l5xx_hal.h>
#elif defined(STM32G0)
#include <stm32g0xx_hal.h>
#elif defined(STM32G4)
#include <stm32g4xx_hal.h>
#elif defined(STM32WB)
#include <stm32wbxx_hal.h>
#else
#error "STM32 family is not defined"
#endif
#endif

#endif // UB_STM32_UTILITIES_STM32_H
