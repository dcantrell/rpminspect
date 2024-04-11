/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/*
 * This header includes the helper functions and macros for librpminspect.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _LIBRPMINSPECT_HELPERS_H
#define _LIBRPMINSPECT_HELPERS_H

/* Macros */
#ifdef NDEBUG
/* Don't create unused variables if not using assert() */
#define xasprintf(dest, ...) {                         \
    *(dest) = NULL;                                    \
    asprintf((dest), __VA_ARGS__);                     \
}
#else
#define xasprintf(dest, ...) {                         \
    int _xasprintf_result;                             \
    *(dest) = NULL;                                    \
    _xasprintf_result = asprintf((dest), __VA_ARGS__); \
    assert(_xasprintf_result != -1);                   \
}
#endif

#endif

#ifdef __cplusplus
}
#endif
