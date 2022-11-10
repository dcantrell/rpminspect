/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <dirent.h>
#include <sys/types.h>
#include "rpminspect.h"

/* Flags used by the inspection */
#define NEEDS_SYMTAB        (((uint64_t) 1) << 2)
#define NEEDS_GDB_INDEX     (((uint64_t) 1) << 3)
#define NEEDS_GNU_DEBUGDATA (((uint64_t) 1) << 4)
#define NEEDS_DEBUG_INFO    (((uint64_t) 1) << 5)

static uint64_t get_flags(const char *s)
{
    uint64_t r = 0;
    string_list_t *sections = NULL;
    string_entry_t *entry = NULL;

    assert(s != NULL);

    sections = strsplit(s, " \t");
    assert(sections != NULL);

    TAILQ_FOREACH(entry, sections, items) {
        if (!strcasecmp(entry->data, ELF_SYMTAB)) {
            r |= NEEDS_SYMTAB;
        } else if (!strcasecmp(entry->data, ELF_GDB_INDEX)) {
            r |= NEEDS_GDB_INDEX;
        } else if (!strcasecmp(entry->data, ELF_GNU_DEBUGDATA)) {
            r |= NEEDS_GNU_DEBUGDATA;
        } else if (!strcasecmp(entry->data, ELF_DEBUG_INFO)) {
            r |= NEEDS_DEBUG_INFO;
        }
    }

    list_free(sections, free);
    return r;
}

static uint64_t missing_sections(const char *fullpath, const uint64_t flags)
{
    uint64_t missing = 0;
    int fd = 0;
    Elf *elf = NULL;

    assert(fullpath != NULL);

    elf = get_elf(fullpath, &fd);
    assert(elf != NULL);

    if ((flags & NEEDS_SYMTAB) && !have_elf_section(elf, -1, ELF_SYMTAB)) {
        missing |= NEEDS_SYMTAB;
    }

    if ((flags & NEEDS_GDB_INDEX) && !have_elf_section(elf, -1, ELF_GDB_INDEX)) {
        missing |= NEEDS_GDB_INDEX;
    }

    if ((flags & NEEDS_GNU_DEBUGDATA) && !have_elf_section(elf, -1, ELF_GNU_DEBUGDATA)) {
        missing |= NEEDS_GNU_DEBUGDATA;
    }

    if ((flags & NEEDS_DEBUG_INFO) && !have_elf_section(elf, -1, ELF_DEBUG_INFO)) {
        missing |= NEEDS_DEBUG_INFO;
    }

    close(fd);
    elf_end(elf);
    return missing;
}

static char *strflags(const uint64_t flags)
{
    string_list_t *list = NULL;
    char *r = NULL;

    if (flags & NEEDS_SYMTAB) {
        list = list_add(list, ELF_SYMTAB);
    } else if (flags & NEEDS_GDB_INDEX) {
        list = list_add(list, ELF_GDB_INDEX);
    } else if (flags & NEEDS_GNU_DEBUGDATA) {
        list = list_add(list, ELF_GNU_DEBUGDATA);
    } else if (flags & NEEDS_DEBUG_INFO) {
        list = list_add(list, ELF_DEBUG_INFO);
    }

    r = list_to_string(list, " ");
    list_free(list, free);
    return r;
}

static bool debuginfo_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    const char *name = NULL;
    char *nvr = NULL;
    char *tmp = NULL;
    bool debugpkg = false;
    uint64_t flags = 0;
    uint64_t missing = 0;
    int fd = 0;
    Elf *elf = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Only deal with ELF shared libraries or executables */
    if (!is_elf_shared_library(file->fullpath) && !is_elf_executable(file->fullpath)) {
        return true;
    }

    if (file->peer_file) {
        if (!is_elf_shared_library(file->peer_file->fullpath) && !is_elf_executable(file->peer_file->fullpath)) {
            return true;
        }
    }

    /* the package nvr and arch is used for reporting */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    nvr = get_nevr(file->rpm_header);
    arch = get_rpm_header_arch(file->rpm_header);

    /* debuginfo and debugsource packages have special handling */
    if (strsuffix(name, DEBUGINFO_SUFFIX) || strsuffix(name, DEBUGSOURCE_SUFFIX)) {
        debugpkg = true;
    }

    /* set the sections to check for */
    if (debugpkg) {
        /* debugging packages need debugging symbols */
        flags |= NEEDS_SYMTAB;
        flags |= NEEDS_DEBUG_INFO;
    } else {
        /* default section (set in config file usually) */
        flags = get_flags(ri->debuginfo_sections);
    }

    /* Golang packages should not have .gnu_debugdata */
    elf = get_elf(file->fullpath, &fd);
    assert(elf != NULL);

    if (have_elf_section(elf, -1, ELF_GOSYMTAB)) {
        flags &= ~NEEDS_GNU_DEBUGDATA;
    }

    elf_end(elf);
    close(fd);

    /* Initialize the result parameters */
    init_result_params(&params);
    params.header = NAME_DEBUGINFO;

    /* Check for and report missing sections for single builds */
    missing = missing_sections(file->fullpath, flags);

    if (missing) {
        xasprintf(&params.msg, _("%s in %s on %s is missing ELF sections "), file->localpath, nvr, arch);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("missing debugging symbols");
        params.file = file->localpath;
        params.arch = arch;

        tmp = strflags(missing);
        xasprintf(&params.details, _("Missing: %s"), tmp);
        free(tmp);

        add_result(ri, &params);

        free(params.msg);
        free(params.details);
    }

    /* When comparing builds, report changes */
    if (file->peer_file) {
        /* stripped in the before file but not the after file */
        missing = missing_sections(file->peer_file->fullpath, flags);

        if (missing && !missing_sections(file->fullpath, flags)) {
            xasprintf(&params.msg, _("%s in %s on %s is no longer stripped"), file->localpath, nvr, arch);
            params.verb = VERB_FAILED;
            params.noun = _("need debugging symbols");
            params.file = file->localpath;
            params.arch = arch;

            tmp = strflags(missing);
            xasprintf(&params.details, _("Need: %s"), tmp);
            free(tmp);

            if (debugpkg) {
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
            } else {
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
            }

            add_result(ri, &params);

            free(params.msg);
            free(params.details);
        }

        /* not stripped in the before file, stripped in the after file */
        missing = missing_sections(file->fullpath, flags);

        if (!missing_sections(file->peer_file->fullpath, flags) && missing) {
            xasprintf(&params.msg, _("%s in %s on %s became stripped"), file->localpath, nvr, arch);
            params.verb = VERB_FAILED;
            params.noun = _("lost debugging symbols");
            params.file = file->localpath;
            params.arch = arch;

            tmp = strflags(missing);
            xasprintf(&params.details, _("Lost: %s"), tmp);
            free(tmp);

            if (debugpkg) {
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
            } else {
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
            }

            add_result(ri, &params);

            free(params.msg);
            free(params.details);
        }
    }

    free(nvr);
    return result;
}

bool inspect_debuginfo(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    result = foreach_peer_file(ri, NAME_DEBUGINFO, debuginfo_driver);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_DEBUGINFO;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
