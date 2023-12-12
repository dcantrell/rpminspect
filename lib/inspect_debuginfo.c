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
#define NEEDS_GNU_DEBUGLINK (((uint64_t) 1) << 5)
#define NEEDS_DEBUG_INFO    (((uint64_t) 1) << 6)

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
        } else if (!strcasecmp(entry->data, ELF_GNU_DEBUGLINK)) {
            r |= NEEDS_GNU_DEBUGLINK;
        } else if (!strcasecmp(entry->data, ELF_DEBUG_INFO)) {
            r |= NEEDS_DEBUG_INFO;
        }
    }

    list_free(sections, free);
    return r;
}

static uint64_t _section_helper(const char *fullpath, const uint64_t flags, const bool check)
{
    uint64_t gathered = 0;
    int fd = 0;
    Elf *elf = NULL;

    assert(fullpath != NULL);

    elf = get_elf(fullpath, &fd);
    assert(elf != NULL);

    if ((flags & NEEDS_SYMTAB) && have_elf_section(elf, -1, ELF_SYMTAB) == check) {
        gathered |= NEEDS_SYMTAB;
    }

    if ((flags & NEEDS_GDB_INDEX) && have_elf_section(elf, -1, ELF_GDB_INDEX) == check) {
        gathered |= NEEDS_GDB_INDEX;
    }

    if ((flags & NEEDS_GNU_DEBUGDATA) && have_elf_section(elf, -1, ELF_GNU_DEBUGDATA) == check) {
        gathered |= NEEDS_GNU_DEBUGDATA;
    }

    if ((flags & NEEDS_GNU_DEBUGLINK) && have_elf_section(elf, -1, ELF_GNU_DEBUGLINK) == check) {
        gathered |= NEEDS_GNU_DEBUGLINK;
    }

    if ((flags & NEEDS_DEBUG_INFO) && have_elf_section(elf, -1, ELF_DEBUG_INFO) == check) {
        gathered |= NEEDS_DEBUG_INFO;
    }

    close(fd);
    elf_end(elf);
    return gathered;
}

static uint64_t have_sections(const char *fullpath, const uint64_t flags)
{
    return _section_helper(fullpath, flags, true);
}

static uint64_t missing_sections(const char *fullpath, const uint64_t flags)
{
    return _section_helper(fullpath, flags, false);
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
    } else if (flags & NEEDS_GNU_DEBUGLINK) {
        list = list_add(list, ELF_GNU_DEBUGLINK);
    } else if (flags & NEEDS_DEBUG_INFO) {
        list = list_add(list, ELF_DEBUG_INFO);
    }

    r = list_to_string(list, " ");
    list_free(list, free);
    return r;
}

/*
 * If we see any section headers that begin with ".guile." then assume
 * this is a Guile object file.
 */
static bool is_guile(const char *path)
{
    bool r = false;
    Elf *elf = NULL;
    int fd = -1;
    string_list_t *sections = NULL;
    string_entry_t *entry = NULL;

    assert(path != NULL);

    elf = get_elf(path, &fd);

    if (elf == NULL) {
        return false;
    }

    sections = get_elf_section_names(elf, SHT_PROGBITS);

    if (sections == NULL || TAILQ_EMPTY(sections)) {
        return false;
    }

    close(fd);
    elf_end(elf);

    TAILQ_FOREACH(entry, sections, items) {
        if (strprefix(entry->data, ".guile.")) {
            r = true;
            break;
        }
    }

    list_free(sections, free);
    return r;
}

static bool debuginfo_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    char *nvr = NULL;
    char *tmp = NULL;
    bool debugpkg = false;
    uint64_t flags = 0;
    uint64_t have = 0;
    uint64_t before_missing = 0;
    uint64_t after_missing = 0;
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
    if (!is_elf_shared_library(file) && !is_elf_executable(file)) {
        return true;
    }

    if (file->peer_file) {
        if (!is_elf_shared_library(file->peer_file) && !is_elf_executable(file->peer_file)) {
            return true;
        }
    }

    /* the package nvr and arch is used for reporting */
    nvr = get_nevr(file->rpm_header);
    arch = get_rpm_header_arch(file->rpm_header);

    /* debuginfo and debugsource packages have special handling */
    if (is_debuginfo_rpm(file->rpm_header) || is_debugsource_rpm(file->rpm_header)) {
        debugpkg = true;
    }

    /* set the sections to check for */
    flags |= get_flags(ri->debuginfo_sections);

    /* Initialize the result parameters */
    init_result_params(&params);
    params.header = NAME_DEBUGINFO;

    /* Check for and report missing or misplaced debuginfo symbols */
    after_missing = missing_sections(file->fullpath, flags);
    have = have_sections(file->fullpath, flags);

    if (debugpkg && after_missing) {
        /* debuginfo packages should not be missing debugging symbols */
        params.file = file->localpath;
        params.arch = arch;

        xasprintf(&params.msg, _("%s in %s on %s is missing debugging symbols"), file->localpath, nvr, arch);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("missing debugging symbols");

        tmp = strflags(after_missing);
        xasprintf(&params.details, _("Missing: %s"), tmp);
        free(tmp);

        add_result(ri, &params);

        free(params.msg);
        free(params.details);

        result = false;
    } else if (!debugpkg && !is_guile(file->fullpath) && have) {
        /* non-debuginfo packages should not contain debugging symbols */
        xasprintf(&params.msg, _("%s in %s on %s contains debugging symbols"), file->localpath, nvr, arch);
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.verb = VERB_FAILED;
        params.noun = _("contains debugging symbols");
        tmp = strflags(have);
        xasprintf(&params.details, _("Contains: %s"), tmp);
        free(tmp);

        add_result(ri, &params);

        free(params.msg);
        free(params.details);

        result = false;
    }

    /* handle build comparisons */
    if (file->peer_file) {
        after_missing = missing_sections(file->fullpath, flags);
        before_missing = missing_sections(file->peer_file->fullpath, flags);
        have = have_sections(file->fullpath, flags);

        if (before_missing && !after_missing && have) {
            /* stripped in the before file but not the after file */
            xasprintf(&params.msg, _("%s in %s on %s gained debugging symbols"), file->localpath, nvr, arch);
            params.noun = _("gained debugging symbols");
            params.file = file->localpath;
            params.arch = arch;

            tmp = strflags(have);
            xasprintf(&params.details, _("Gained: %s"), tmp);
            free(tmp);

            if (debugpkg) {
                params.verb = VERB_OK;
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
            } else {
                params.verb = VERB_FAILED;
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                result = false;
            }

            add_result(ri, &params);

            free(params.msg);
            free(params.details);
        } else if (!before_missing && after_missing) {
            /* not stripped in the before file, stripped in the after file */
            xasprintf(&params.msg, _("%s in %s on %s lost debugging symbols"), file->localpath, nvr, arch);
            params.noun = _("lost debugging symbols");
            params.file = file->localpath;
            params.arch = arch;

            tmp = strflags(after_missing);
            xasprintf(&params.details, _("Lost: %s"), tmp);
            free(tmp);

            if (debugpkg) {
                params.verb = VERB_FAILED;
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                result = false;
            } else {
                params.verb = VERB_OK;
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
            }

            add_result(ri, &params);

            free(params.msg);
            free(params.details);
        }
    }

    /* Final non-debuginfo package checks */
    if (!debugpkg) {
        elf = get_elf(file->fullpath, &fd);

        if (elf && have_elf_section(elf, -1, ELF_GOSYMTAB) && have_elf_section(elf, -1, ELF_GNU_DEBUGDATA)) {
            xasprintf(&params.msg, _("%s in %s on %s carries .gosymtab but should not have the .gnu_debugdata symbol"), file->localpath, nvr, arch);
            params.verb = VERB_FAILED;
            params.noun = _(".gnu_debugdata with .gosymtab");
            params.file = file->localpath;
            params.arch = arch;
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.details = NULL;
            result = false;

            add_result(ri, &params);
            free(params.msg);
        }

        close(fd);
        elf_end(elf);
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
