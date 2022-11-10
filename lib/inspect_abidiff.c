/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "queue.h"
#include "uthash.h"
#include "rpminspect.h"

/* Globals */
static char *cmdprefix = NULL;
static string_list_t *suppressions = NULL;
static abi_t *abi = NULL;

static severity_t check_abi(const severity_t sev, const long int threshold, const char *path, const char *pkg, long int *compat)
{
    abi_t *entry = NULL;
    string_entry_t *dsoentry = NULL;
    char pathcopy[PATH_MAX + 1];
    char *basefn = NULL;

    /* no ABI compat level data */
    if (abi == NULL) {
        return sev;
    }

    assert(path != NULL);
    assert(pkg != NULL);

    /* look for this package in the ABI table */
    HASH_FIND_STR(abi, pkg, entry);

    if (entry == NULL) {
        /* package not found, no ABI checking */
        return sev;
    }

    *compat = entry->level;

    /* is the ABI level above the threshold? */
    if (entry->level > threshold) {
        if (entry->all) {
            return RESULT_INFO;
        } else {
            /* do specific matching on the DSO name */
            TAILQ_FOREACH(dsoentry, entry->dsos, items) {
                if ((dsoentry->data[0] == '/') && strprefix(path, dsoentry->data)) {
                    return RESULT_INFO;
                } else {
                    /* do a soft match against the file basename */
                    basefn = strncpy(pathcopy, path, PATH_MAX);
                    assert(basefn != NULL);
                    basefn = basename(pathcopy);
                    assert(basefn != NULL);

                    if (strprefix(basefn, dsoentry->data)) {
                        return RESULT_INFO;
                    }
                }
            }
        }
    }

    return sev;
}

static bool abidiff_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    bool rebase = false;
    char **argv = NULL;
    string_entry_t *entry = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    int exitcode = 0;
    char *cmd = NULL;
    char *tmp = NULL;
    char *output = NULL;
    struct result_params params;
    bool report = false;
    long int compat_level = 0;

    assert(ri != NULL);
    assert(file != NULL);
    assert(cmdprefix != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* Ignore debuginfo and debugsource paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* skip anything that is not an ELF shared library file.  */
    if (!S_ISREG(file->st.st_mode) || !is_elf_shared_library(file->fullpath)) {
        return true;
    }

    /* get the package architecture */
    arch = get_rpm_header_arch(file->rpm_header);

    /* build the abidiff command */
    cmd = strdup(cmdprefix);
    assert(cmd != NULL);

    if (suppressions && !TAILQ_EMPTY(suppressions)) {
        TAILQ_FOREACH(entry, suppressions, items) {
            cmd = strappend(cmd, " ", entry->data, NULL);
        }
    }

    /* debug dir args */
    tmp = joinpath(ri->worksubdir, ROOT_SUBDIR, BEFORE_SUBDIR, arch, DEBUG_PATH, NULL);
    assert(tmp != NULL);
    cmd = strappend(cmd, " ", ABI_DEBUG_INFO_DIR1, " ", tmp, NULL);
    assert(cmd != NULL);
    free(tmp);

    tmp = joinpath(ri->worksubdir, ROOT_SUBDIR, AFTER_SUBDIR, arch, DEBUG_PATH, NULL);
    assert(tmp != NULL);
    cmd = strappend(cmd, " ", ABI_DEBUG_INFO_DIR2, " ", tmp, NULL);
    assert(cmd != NULL);
    free(tmp);

    /* the before and after builds */
    cmd = strappend(cmd, " ", file->peer_file->fullpath, " ", file->fullpath, NULL);

    /* run abidiff */
    argv = build_argv(cmd);
    output = run_cmd_vpe(&exitcode, NULL, argv);
    free_argv(argv);

    /* determine if this is a rebase build */
    rebase = is_rebase(ri);

    /* report the results */
    init_result_params(&params);
    params.header = NAME_ABIDIFF;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.remedy = REMEDY_ABIDIFF;
    params.arch = arch;
    params.file = file->localpath;

    if ((exitcode & ABIDIFF_ERROR) || (exitcode & ABIDIFF_USAGE_ERROR)) {
        params.severity = RESULT_VERIFY;
        params.verb = VERB_FAILED;
        params.noun = _("abidiff usage error");;
        report = true;
    } else if (!rebase && (exitcode & ABIDIFF_ABI_CHANGE)) {
        params.severity = RESULT_VERIFY;
        params.verb = VERB_CHANGED;
        params.noun = _("ABI change in ${FILE} on ${ARCH}");
        report = true;
    } else if (!rebase && (exitcode & ABIDIFF_ABI_INCOMPATIBLE_CHANGE)) {
        params.severity = RESULT_BAD;
        params.verb = VERB_CHANGED;
        params.noun = _("ABI incompatible change in ${FILE} on ${ARCH}");
        report = true;
    }

    /* check the ABI compat level list */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    params.severity = check_abi(params.severity, ri->abi_security_threshold, file->localpath, name, &compat_level);

    /* add additional details */
    if (report) {
        if (!strcmp(file->peer_file->localpath, file->localpath)) {
            if (compat_level) {
                xasprintf(&params.msg, _("Comparing old vs. new version of %s in package %s with ABI compatibility level %ld on %s revealed ABI differences."), file->localpath, name, compat_level, arch);
            } else {
                xasprintf(&params.msg, _("Comparing old vs. new version of %s in package %s on %s revealed ABI differences."), file->localpath, name, arch);
            }
        } else {
            if (compat_level) {
                xasprintf(&params.msg, _("Comparing from %s to %s in package %s with ABI compatibility level %ld on %s revealed ABI differences."), file->peer_file->localpath, file->localpath, name, compat_level, arch);
            } else {
                xasprintf(&params.msg, _("Comparing from %s to %s in package %s on %s revealed ABI differences."), file->peer_file->localpath, file->localpath, name, arch);
            }
        }
    }

    if (report || (exitcode && output)) {
        if (exitcode && output) {
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.msg = strdup(_("ABI comparison ended unexpectedly."));
            params.verb = VERB_FAILED;
            params.noun = _("abidff unexpected exit");
        }

        params.file = file->localpath;
        xasprintf(&params.details, _("Command: %s\n\n%s"), cmd, output);
        add_result(ri, &params);
        free(params.msg);
        free(params.details);
        result = false;
    }

    /* cleanup */
    free(cmd);
    free(output);

    return result;
}

/*
 * Main driver for the 'abidiff' inspection.
 */
bool inspect_abidiff(struct rpminspect *ri)
{
    bool result = false;
    struct result_params params;

    assert(ri != NULL);

    /* get the ABI compat level data if there is any */
    abi = read_abi(ri->vendor_data_dir, ri->product_release);

    /* if there's a .abignore file in the after SRPM, we need to use it */
    suppressions = get_abidiff_suppressions(ri, ri->abidiff_suppression_file);

    /* build the list of first part of the command */
    if (ri->abidiff_extra_args) {
        xasprintf(&cmdprefix, "%s %s", ri->commands.abidiff, ri->abidiff_extra_args);
    } else {
        cmdprefix = strdup(ri->commands.abidiff);
    }

    /* run the main inspection */
    result = foreach_peer_file(ri, NAME_ABIDIFF, abidiff_driver);

    /* clean up */
    free_abi(abi);
    free(cmdprefix);
    list_free(suppressions, free);

    /* report the inspection results */
    if (result) {
        init_result_params(&params);
        params.header = NAME_ABIDIFF;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
