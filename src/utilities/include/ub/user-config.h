#ifndef UB_UTILITIES_USER_CONFIG_H
#define UB_UTILITIES_USER_CONFIG_H

/**
 * Locates and includes user-provided configuration file.
 */

// Primary configuration file:

#if defined(UB_USER_CONFIG_FILE)
#include UB_USER_CONFIG_FILE
#endif

// Multiple extra slots for convenience in multi-project builds

#if defined(UB_USER_CONFIG_FILE_00)
#include UB_USER_CONFIG_FILE_00
#endif

#if defined(UB_USER_CONFIG_FILE_01)
#include UB_USER_CONFIG_FILE_01
#endif

#if defined(UB_USER_CONFIG_FILE_02)
#include UB_USER_CONFIG_FILE_02
#endif

#if defined(UB_USER_CONFIG_FILE_03)
#include UB_USER_CONFIG_FILE_03
#endif

#if defined(UB_USER_CONFIG_FILE_04)
#include UB_USER_CONFIG_FILE_04
#endif

#if defined(UB_USER_CONFIG_FILE_05)
#include UB_USER_CONFIG_FILE_05
#endif

#if defined(UB_USER_CONFIG_FILE_06)
#include UB_USER_CONFIG_FILE_06
#endif

#if defined(UB_USER_CONFIG_FILE_07)
#include UB_USER_CONFIG_FILE_07
#endif

#if defined(UB_USER_CONFIG_FILE_08)
#include UB_USER_CONFIG_FILE_08
#endif

#if defined(UB_USER_CONFIG_FILE_09)
#include UB_USER_CONFIG_FILE_09
#endif

#endif // UB_UTILITIES_USER_CONFIG_H
