/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>
#include <libgen.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#include "queue.h"
#include "rpminspect.h"

/* Globals */
static char *cmdprefix = NULL;
static string_list_t *suppressions = NULL;
static bool found_kernel_image = false;
static char *kabi_dir = NULL;

/* Pointers to root extracted package paths (do not free these) */
static const char *before_root = NULL;
static const char *after_root = NULL;

/**
 * Given a build, search for the dir path in all extracted packages.
 * If we don't find one, the path will remain NULL and no kabi will be
 * used during kmidiff runs.
 */
static void get_kabi_dir(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct stat sbuf;

    assert(ri != NULL);

    /* nothing to do if kabi_path is not defined in the configuration */
    if (ri->kabi_dir == NULL) {
        return;
    }

    /* look for first matching path */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* use stat(2) here because we want to read what symlinks point to */
            if (stat(file->fullpath, &sbuf) != 0) {
                continue;
            }

            if (!S_ISDIR(sbuf.st_mode)) {
                continue;
            }

            /* found the kabi directory in this package */
            if (!strcmp(file->localpath, ri->kabi_dir)) {
                kabi_dir = strdup(file->fullpath);
                break;
            }
        }

        if (kabi_dir) {
            break;
        }
    }

    return;
}

/*
 * Return a full path to the appropriate kabi file or NULL if it doesn't exist.
 * Caller must free the returned string.
 */
static char *get_kabi_file(const struct rpminspect *ri, const char *arch)
{
    char *tmp = NULL;
    char *template = NULL;
    char *kabi = NULL;

    assert(ri != NULL);
    assert(arch != NULL);

    if (kabi_dir == NULL || ri->kabi_filename == NULL) {
        return NULL;
    }

    /* build the template */
    xasprintf(&template, "%s/%s", kabi_dir, ri->kabi_filename);
    assert(template != NULL);

    /* replace variables */
    tmp = strreplace(template, "${ARCH}", arch);

    if (strstr(tmp, "$ARCH")) {
        free(tmp);
        tmp = strreplace(template, "$ARCH", arch);
    }

    /* normalize the path */
    kabi = abspath(tmp);
    assert(kabi != NULL);
    free(tmp);

    /* let's see if this is actually a file */
    if (access(kabi, R_OK)) {
        free(kabi);
        kabi = NULL;
    }

    /* we're done */
    free(template);
    return kabi;
}

static bool kmidiff_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    bool rebase = false;
    char **argv = NULL;
    string_entry_t *entry = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    char *kabi = NULL;
    int exitcode = 0;
    int i = 0;
    char *fname[] = KERNEL_FILENAMES;
    char *compare = NULL;
    char *cmd = NULL;
    char *tmp = NULL;
    char *output = NULL;
    struct result_params params;
    bool report = false;

    assert(ri != NULL);
    assert(file != NULL);
    assert(cmdprefix != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip anything except debuginfo packages */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    assert(name != NULL);

    if (!is_debuginfo_rpm(file->rpm_header)) {
        return true;
    }

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip anything that is not an ELF file */
    if (!S_ISREG(file->st.st_mode) || !is_elf(file)) {
        return true;
    }

    /* skip anything that is not a kernel image */
    for (i = 0; fname[i] != NULL; i++) {
        xasprintf(&compare, "/%s", basename(fname[i]));
        assert(compare != NULL);

        if (strsuffix(file->localpath, compare)) {
            found_kernel_image = true;
        }

        free(compare);

        if (found_kernel_image) {
            break;
        }
    }

    if (!found_kernel_image) {
        return true;
    }

    /* get the package architecture */
    arch = get_rpm_header_arch(file->rpm_header);
    kabi = get_kabi_file(ri, arch);

    /* build the kmidiff command */
    cmd = strdup(cmdprefix);
    assert(cmd != NULL);

    if (kabi) {
        cmd = strappend(cmd, " ", KMIDIFF_KMI_WHITELIST, " ", kabi, NULL);
        free(kabi);
    }

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

    /* the before and after kernel images and root directories */
    cmd = strappend(cmd, " ", KMIDIFF_VMLINUX1, " ", file->peer_file->fullpath, " ", KMIDIFF_VMLINUX2, " ", file->fullpath, " ", before_root, " ", after_root, NULL);

    /* run kmidiff */
    argv = build_argv(cmd);
    output = run_cmd_vp(&exitcode, ri->worksubdir, argv);
    free_argv(argv);

    /* determine if this is a rebase build */
    rebase = is_rebase(ri);

    /* report the results */
    init_result_params(&params);
    params.header = NAME_KMIDIFF;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.remedy = REMEDY_KMIDIFF;
    params.arch = arch;
    params.file = file->localpath;

    /*
     * An exit code of 0 means the compared binaries are ABI-equal.
     * Non-zero means something, which is documented here:
     *
     * https://sourceware.org/libabigail/manual/abidiff.html#return-values
     */
    if (exitcode & ABIDIFF_ERROR) {
        report = true;
        params.severity = RESULT_VERIFY;

        if (exitcode & ABIDIFF_USAGE_ERROR) {
            xasprintf(&params.msg, _("Comparing %s to %s in package %s on %s generated a kmidiff(1) usage error."), file->peer_file->localpath, file->localpath, name, arch);
            params.verb = VERB_FAILED;
            params.noun = _("kmidiff usage error");
        } else if (!rebase && (exitcode & ABIDIFF_ABI_CHANGE)) {
            xasprintf(&params.msg, _("Comparing %s to %s in package %s on %s revealed Kernel Module Interface (KMI) differences."), file->peer_file->localpath, file->localpath, name, arch);
            params.verb = VERB_CHANGED;
            params.noun = _("KMI change in ${FILE} on ${ARCH}");
        } else if (!rebase && (exitcode & ABIDIFF_ABI_CHANGE) && (exitcode & ABIDIFF_ABI_INCOMPATIBLE_CHANGE)) {
            xasprintf(&params.msg, _("Comparing %s to %s in package %s on %s revealed incompatible Kernel Module Interface (KMI) differences."), file->peer_file->localpath, file->localpath, name, arch);
            params.severity = RESULT_BAD;
            params.verb = VERB_CHANGED;
            params.noun = _("KMI incompatible change in ${FILE} on ${ARCH}");
        } else {
            xasprintf(&params.msg, _("kmidiff(1) comparison of %s to %s in package %s on %s ended unexpectedly."), file->peer_file->localpath, file->localpath, name, arch);
            params.verb = VERB_FAILED;
            params.noun = _("kmidiff unexpected exit");
        }
    }

    /* add additional details */
    if (report) {
        params.file = file->localpath;
        xasprintf(&params.details, _("Command: %s\nExit code: %d%s%s"), cmd, exitcode, output ? "\n\n" : "", output ? output : "");
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
 * Main driver for the 'kmidiff' inspection.
 */
bool inspect_kmidiff(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct result_params params;

    assert(ri != NULL);

    /* get the kabi path if that exists in this build */
    get_kabi_dir(ri);

    /* if there's a .abignore file in the after SRPM, we need to use it */
    suppressions = get_abidiff_suppressions(ri, ri->kmidiff_suppression_file);

    /* build the list of first command line arguments */
    if (ri->kmidiff_extra_args) {
        xasprintf(&cmdprefix, "%s %s", ri->commands.kmidiff, ri->kmidiff_extra_args);
    } else {
        cmdprefix = strdup(ri->commands.kmidiff);
    }

    /* run the main inspection */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* Ignore files we should be ignoring */
            if (ignore_path(ri, NAME_KMIDIFF, file->localpath, peer->after_root)) {
                continue;
            }

            before_root = peer->before_root;
            after_root = peer->after_root;

            if (!kmidiff_driver(ri, file)) {
                result = false;
            }

            if (found_kernel_image) {
                break;
            }
        }

        if (found_kernel_image) {
            break;
        }
    }

    /* clean up */
    free(cmdprefix);
    list_free(suppressions, free);
    free(kabi_dir);

    /* report the inspection results */
    if (result) {
        init_result_params(&params);
        params.header = NAME_KMIDIFF;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
