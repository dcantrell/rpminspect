/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "i18n.h"

#ifndef _LIBRPMINSPECT_REMEDY_H
#define _LIBRPMINSPECT_REMEDY_H

#define REMEDY_NULL                      0
#define REMEDY_ABIDIFF                   1
#define REMEDY_ADDEDFILES                2
#define REMEDY_ANNOCHECK                 3
#define REMEDY_ANNOCHECK_FORTIFY_SOURCE  4
#define REMEDY_ARCH_GAIN                 5
#define REMEDY_ARCH_LOST                 6
#define REMEDY_BADFUNCS                  7
#define REMEDY_BADWORDS                  8
#define REMEDY_BUILDHOST                 9
#define REMEDY_CAPABILITIES              10
#define REMEDY_CHANGEDFILES              11
#define REMEDY_CHANGELOG                 12
#define REMEDY_CONFIG                    13
#define REMEDY_DESKTOP                   14
#define REMEDY_DISTTAG                   15
#define REMEDY_DOC                       16
#define REMEDY_DSODEPS                   17
#define REMEDY_ELF_EXECSTACK_EXECUTABLE  18
#define REMEDY_ELF_EXECSTACK_INVALID     19
#define REMEDY_ELF_EXECSTACK_MISSING     20
#define REMEDY_ELF_FPIC                  21
#define REMEDY_ELF_GNU_RELRO             22
#define REMEDY_ELF_TEXTREL               23
#define REMEDY_EMPTYRPM                  24
#define REMEDY_FILEINFO_RULE             25
#define REMEDY_FILE_PATHS                26
#define REMEDY_FILESIZE_BECAME_EMPTY     27
#define REMEDY_FILESIZE_BECAME_NOT_EMPTY 28
#define REMEDY_FILESIZE_GREW             29
#define REMEDY_FILESIZE_SHRANK           30
#define REMEDY_INVALID_BOOLEAN           31
#define REMEDY_JAVABYTECODE              32
#define REMEDY_KMIDIFF                   33
#define REMEDY_KMOD_ALIAS                34
#define REMEDY_KMOD_DEPS                 35
#define REMEDY_KMOD_PARM                 36
#define REMEDY_LICENSE                   37
#define REMEDY_LICENSEDB                 38
#define REMEDY_LOSTPAYLOAD               39
#define REMEDY_LTO                       40
#define REMEDY_MAN_ERRORS                41
#define REMEDY_MAN_PATH                  42
#define REMEDY_MODULARITY_LABEL          43
#define REMEDY_MODULARITY_RELEASE        44
#define REMEDY_MODULARITY_STATIC_CONTEXT 45
#define REMEDY_MOVEDFILES                46
#define REMEDY_OWNERSHIP_BIN_GROUP       47
#define REMEDY_OWNERSHIP_BIN_OWNER       48
#define REMEDY_OWNERSHIP_CHANGED         49
#define REMEDY_OWNERSHIP_DEFATTR         50
#define REMEDY_OWNERSHIP_IWGRP           51
#define REMEDY_OWNERSHIP_IXOTH           52
#define REMEDY_PATCHES_CORRUPT           53
#define REMEDY_PATCHES_MISMATCHED_MACRO  54
#define REMEDY_PATCHES_MISSING_MACRO     55
#define REMEDY_PATCHES_UNHANDLED_PATCH   56
#define REMEDY_PATHMIGRATION             57
#define REMEDY_POLITICS                  58
#define REMEDY_REMOVEDFILES              59
#define REMEDY_RPMDEPS_CHANGED           60
#define REMEDY_RPMDEPS_EPOCH             61
#define REMEDY_RPMDEPS_EXPLICIT          62
#define REMEDY_RPMDEPS_EXPLICIT_EPOCH    63
#define REMEDY_RPMDEPS_GAINED            64
#define REMEDY_RPMDEPS_LOST              65
#define REMEDY_RPMDEPS_MACROS            66
#define REMEDY_RPMDEPS_MULTIPLE          67
#define REMEDY_RUNPATH                   68
#define REMEDY_RUNPATH_BOTH              69
#define REMEDY_SHELLSYNTAX               70
#define REMEDY_SHELLSYNTAX_BAD           71
#define REMEDY_SHELLSYNTAX_GAINED_SHELL  72
#define REMEDY_SPECNAME                  73
#define REMEDY_SUBPACKAGES_GAIN          74
#define REMEDY_SUBPACKAGES_LOST          75
#define REMEDY_SYMLINKS                  76
#define REMEDY_SYMLINKS_DIRECTORY        77
#define REMEDY_TYPES                     78
#define REMEDY_UDEVRULES                 79
#define REMEDY_UNAPPROVED_LICENSE        80
#define REMEDY_UNICODE                   81
#define REMEDY_UNICODE_PREP_FAILED       82
#define REMEDY_UPSTREAM                  83
#define REMEDY_VENDOR                    84
#define REMEDY_VIRUS                     85
#define REMEDY_XML                       86
#define REMEDY_MIXED_LICENSE_TAGS        87

/* Initialize default remedy strings */
void init_remedy_strings(void);

/* Get the remedy string for the specified remedy ID */
const char *get_remedy(const unsigned int id);

/* Set remedy override string */
bool set_remedy(const char *name, const char *remedy);

#endif /* _LIBRPMINSPECT_REMEDY_H */

#ifdef __cplusplus
}
#endif
