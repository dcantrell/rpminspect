/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#ifndef _LIBRPMINSPECT_MOFILE_H
#define _LIBRPMINSPECT_MOFILE_H

#define MO_BE 0x950412de
#define MO_LE 0xde120495

typedef struct _mo_file_t {
    uint32_t magic;            /* MO magic number - either MO_BE or MO_LE */
    uint32_t ver;              /* file format revision */
    uint32_t numstrings;       /* number of strings in the file */
    uint32_t origoffset;       /* offset to original strings table */
    uint32_t transoffset;      /* offset to translated strings table */
    uint32_t htsz;             /* hash table size (unused here) */
    uint32_t htoffset;         /* hash table offset (unused here) */

    /* these arrays are size 'numstrings' */
    char **original;           /* array of original strings */
    char **translated;         /* array of translated strings */
} mo_file_t;

typedef struct _mo_entry_t {
    uint32_t length;           /* length of the string */
    uint32_t offset;           /* position in the file */
} mo_entry_t;

#endif /* _LIBRPMINSPECT_MOFILE_H */

#ifdef __cplusplus
}
#endif
