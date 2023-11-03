#ifndef UB_UTILITIES_USER_CONFIG_H
#define UB_UTILITIES_USER_CONFIG_H

/**
 * Locates and includes user-provided configuration file.
 */

#if defined(UB_USER_CONFIG_FILE)
#include UB_USER_CONFIG_FILE
#endif

#endif // UB_UTILITIES_USER_CONFIG_H
