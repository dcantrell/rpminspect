/*
 * Copyright 2019 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <libannocheck.h>

#include "rpminspect.h"

const unsigned int libannocheck_version = 1088;

/* Global variables */
static bool reported = false;

/* Trim workdir substrings from a generated string. */
static char *trim_workdir(const rpmfile_entry_t *file, char *s)
{
    size_t fl = 0;
    size_t ll = 0;
    char *workdir = NULL;
    char *tmp = NULL;

    assert(file != NULL);

    if (s == NULL) {
        return s;
    }

    fl = strlen(file->fullpath);
    ll = strlen(file->localpath);

    if (fl > ll) {
        workdir = strndup(file->fullpath, fl - ll);
    }

    if (workdir) {
        tmp = strreplace(s, workdir, NULL);
        free(s);
        free(workdir);
        s = tmp;
    }

    return s;
}

/*
 * Build the annocheck command to run and report in the output.  This
 * is a single string the caller must free.
 */
static char *build_annocheck_cmd(const char *cmd, const char *opts, const char *debugpath, const char *path)
{
    char *r = NULL;

    assert(cmd != NULL);
    assert(path != NULL);

    if (opts == NULL && debugpath == NULL) {
        xasprintf(&r, "%s %s", cmd, path);
    } else if (opts && debugpath == NULL) {
        xasprintf(&r, "%s %s %s", cmd, opts, path);
    } else if (opts && debugpath) {
        xasprintf(&r, "%s %s --debug-dir=%s %s", cmd, opts, debugpath, path);
    }

    return r;
}

/*
 * Try to map the product release string to an appropriate
 * libannocheck profile.  This is going to be something that will need
 * maintenance over time.  Since the profile list is embedded in
 * libannocheck, there's no easy way to discover changes.  The
 * assumption here is the profile naming scheme remains the same.
 */
static void set_libannocheck_profile(struct libannocheck_internals *anno, const char *product_release)
{
    const char **profiles = NULL;
    unsigned int num_profiles = 0;
    unsigned int i = 0;
    libannocheck_error annoerr = 0;

    assert(anno != NULL);
    assert(product_release != NULL);

    /* get libannocheck profiles first */
    annoerr = libannocheck_get_known_profiles(anno, &profiles, &num_profiles);

    if (annoerr != 0) {
         warnx(_("libannocheck_get_known_profiles error: %s"), libannocheck_get_error_message(anno, annoerr));
         return;
    }

    /* iterate over the profiles to try and find a match */
    for (i = 0; i < num_profiles; i++) {
DEBUG_PRINT("profiles[%u]=|%s|\n", i, profiles[i]);
    }

    return;
}

static bool annocheck_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    char **argv = NULL;
    char *before_cmd = NULL;
    char *after_cmd = NULL;
    string_map_t *hentry = NULL;
    string_map_t *tmp_hentry = NULL;
    char *after_out = NULL;
    int after_exit = 0;
    char *before_out = NULL;
    int before_exit = 0;
    char *details = NULL;
    string_list_t *slist = NULL;
    string_entry_t *sentry = NULL;
    struct libannocheck_internals *anno = NULL;
    libannocheck_error annoerr = 0;
    struct libannocheck_test *annotests = NULL;
    unsigned int numtests = 0;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debug and build paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Only run this check on ELF files */
    if (!is_elf_file(file->fullpath)
        || (!is_elf_file(file->fullpath) && file->peer_file && !is_elf_file(file->peer_file->fullpath))) {
        return true;
    }

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_ANNOCHECK;
    params.remedy = REMEDY_ANNOCHECK;
    params.arch = arch;
    params.file = file->localpath;
    params.verb = VERB_OK;

    /* Run each annocheck test and report the results */
    HASH_ITER(hh, ri->annocheck, hentry, tmp_hentry) {
        /* initialize libannocheck for this test on this file */
        annoerr = libannocheck_init(libannocheck_get_version(), file->fullpath, get_after_debuginfo_path(ri, file, arch), &anno);

        if (annoerr != 0) {
             warnx(_("libannocheck_init error: %s"), libannocheck_get_error_message(anno, annoerr));
             libannocheck_finish(anno);
             continue;
        }

        /* get the list of known tests */
        annoerr = libannocheck_get_known_tests(anno, &annotests, &numtests);

        if (annoerr != 0) {
             warnx(_("libannocheck_get_known_tests error: %s"), libannocheck_get_error_message(anno, annoerr));
             libannocheck_finish(anno);
             continue;
        }

        /* enable libannocheck profile if there's a match */
        set_libannocheck_profile(anno, ri->product_release);




/* XXX
 *
 * map the product release to an annocheck profile and enable that
 *
 * read the options and anything that is enable or disable, call libannocheck_enable or libannocheck_disable as appropriate
 *     for all of the options from the config file, only honor enable/disable ones for tests
 *     note which ones are skipped as part of -v messages
 *
 * call libannocheck_get_known_tests to get the array of known tests
 *
 * call libannocheck_run_tests and report results
 *
 * libannocheck_finish
 */
        annoerr = libannocheck_finish(anno);

        if (annoerr != 0) {
            warnx(_("libannocheck_finish error: %s"), libannocheck_get_error_message(anno, annoerr));
            continue;
        }


continue;







        /* Run the test on the file */
        after_cmd = build_annocheck_cmd("annocheck", hentry->value, get_after_debuginfo_path(ri, file, arch), file->fullpath);
        argv = build_argv(after_cmd);
        after_out = run_cmd_vpe(&after_exit, ri->worksubdir, argv);
        free_argv(argv);

        /* If we have a before build, run the command on that */
        if (file->peer_file) {
            before_cmd = build_annocheck_cmd("annocheck", hentry->value, get_before_debuginfo_path(ri, file, arch), file->peer_file->fullpath);
            argv = build_argv(before_cmd);
            before_out = run_cmd_vpe(&before_exit, ri->workdir, argv);
            free_argv(argv);

            /* Build a reporting message if we need to */
            if (before_exit == 0 && after_exit == 0) {
                xasprintf(&params.msg, _("annocheck '%s' test passes for %s on %s"), hentry->key, file->localpath, arch);
            } else if (before_exit && after_exit == 0) {
                xasprintf(&params.msg, _("annocheck '%s' test now passes for %s on %s"), hentry->key, file->localpath, arch);
            } else if (before_exit == 0 && after_exit) {
                xasprintf(&params.msg, _("annocheck '%s' test now fails for %s on %s"), hentry->key, file->localpath, arch);
                params.severity = ri->annocheck_failure_severity;
                params.verb = VERB_CHANGED;
                result = !(ri->annocheck_failure_severity >= RESULT_VERIFY);
            } else if (after_exit) {
                xasprintf(&params.msg, _("annocheck '%s' test fails for %s on %s"), hentry->key, file->localpath, arch);
                params.severity = ri->annocheck_failure_severity;
                params.verb = VERB_CHANGED;
                result = !(ri->annocheck_failure_severity >= RESULT_VERIFY);
            }
        } else {
            if (after_exit == 0) {
                xasprintf(&params.msg, _("annocheck '%s' test passes for %s on %s"), hentry->key, file->localpath, arch);
            } else if (after_exit) {
                xasprintf(&params.msg, _("annocheck '%s' test fails for %s on %s"), hentry->key, file->localpath, arch);
                params.severity = ri->annocheck_failure_severity;
                params.verb = VERB_CHANGED;
                result = !(ri->annocheck_failure_severity >= RESULT_VERIFY);
            }
        }

        /* Report the results */
        if (params.msg) {
            /* trim the before build working directory and generate details */
            if (before_cmd) {
                before_cmd = trim_workdir(file->peer_file, before_cmd);
                xasprintf(&details, "Command: %s\nExit Code: %d\n    compared with the output of:\nCommand: %s\nExit Code: %d\n\n%s", before_cmd, before_exit, after_cmd, after_exit, after_out);
            } else {
                xasprintf(&details, "Command: %s\nExit Code: %d\n\n%s", after_cmd, after_exit, after_out);
            }

            /* trim the after build working directory */
            details = trim_workdir(file, details);

            params.details = details;
            add_result(ri, &params);
            reported = true;
            free(params.msg);
        }

        /* Check for loss of -O2 -D_FORTIFY_SOURCE=2 */
        /* XXX: this will change with a move to libannocheck */
        if (after_out) {
            slist = strsplit(after_out, "\n");
            assert(slist != NULL);

            TAILQ_FOREACH(sentry, slist, items) {
                if (strprefix(sentry->data, "FAIL:") && (strstr(sentry->data, "fortify") || strstr(sentry->data, "optimization"))) {
                    init_result_params(&params);
                    params.header = NAME_ANNOCHECK;
                    params.waiverauth = WAIVABLE_BY_SECURITY;
                    params.remedy = REMEDY_ANNOCHECK_FORTIFY_SOURCE;
                    params.arch = arch;
                    params.file = file->localpath;
                    params.verb = VERB_REMOVED;
                    params.noun = _("lost -D_FORTIFY_SOURCE in ${FILE} on ${ARCH}");
                    params.severity = get_secrule_result_severity(ri, file, SECRULE_FORTIFYSOURCE);

                    xasprintf(&params.msg, _("%s may have lost -D_FORTIFY_SOURCE on %s"), file->localpath, arch);
                    params.details = details;

                    if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                        add_result(ri, &params);
                        reported = true;
                        result = !(params.severity >= RESULT_VERIFY);
                    }

                    break;
                }
            }

            list_free(slist, free);
        }

        /* Cleanup */
        free(details);

        free(after_out);
        free(before_out);

        free(after_cmd);
        free(before_cmd);

        after_exit = 0;
        before_exit = 0;
    }

    return result;
}

/*
 * Main driver for the 'annocheck' inspection.
 */
bool inspect_annocheck(struct rpminspect *ri)
{
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    /* skip if we have no annocheck tests defined */
    if (ri->annocheck == NULL) {
        return true;
    }

    /* run the annocheck tests across all ELF files */
    result = foreach_peer_file(ri, NAME_ANNOCHECK, annocheck_driver);

    /* if everything was fine, just say so */
    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_ANNOCHECK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
