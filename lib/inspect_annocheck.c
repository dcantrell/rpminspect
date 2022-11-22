/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <libannocheck.h>

#include "rpminspect.h"

/* Global variables */
static bool reported = false;

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

    if (anno == NULL || product_release == NULL) {
        return;
    }

    /* get libannocheck profiles first */
    annoerr = libannocheck_get_known_profiles(anno, &profiles, &num_profiles);

    if (annoerr != libannocheck_error_none) {
         warnx(_("libannocheck_get_known_profiles error: %s"), libannocheck_get_error_message(anno, annoerr));
         return;
    }

    /* iterate over the profiles to try and find a match */
    for (i = 0; i < num_profiles; i++) {
        /* XXX: this needs a map in the config file */
        if (strsuffix(product_release, profiles[i]) || (strprefix(product_release, ".fc") && !strcmp(profiles[i], "rawhide"))) {
            annoerr = libannocheck_enable_profile(anno, profiles[i]);

            if (annoerr != libannocheck_error_none) {
                warnx(_("libannocheck_enable_profile error: %s"), libannocheck_get_error_message(anno, annoerr));
            }

            return;
        }
    }

    return;
}

/*
 * Convert a libannocheck_test_state enum to a string suitable for
 * reporting.  Do not free this string.
 */
static const char *get_state(libannocheck_test_state s)
{
    if (s == libannocheck_test_state_not_run) {
        return _("NOT RUN");
    } else if (s == libannocheck_test_state_passed) {
        return _("PASSED");
    } else if (s == libannocheck_test_state_failed) {
        return _("FAILED");
    } else if (s == libannocheck_test_state_maybe) {
        return _("MAYBE");
    } else if (s == libannocheck_test_state_skipped) {
        return _("skipped");
    } else {
        return _("UNKNOWN");
    }
}

/*
 * Given the existing 'worst' value and a new libannocheck_test_state
 * value, compare them returning the worst one.  This function ignores
 * the 'not run' and 'skipped' states.
 */
static libannocheck_test_state get_worst(libannocheck_test_state worst, libannocheck_test_state s)
{
    /* ignore 'not run' and 'skipped' states */
    if (s == libannocheck_test_state_not_run || s == libannocheck_test_state_skipped) {
        return worst;
    }

    if (s > worst) {
        return s;
    }

    return worst;
}

/*
 * Do the libannocheck setup for a file.  If this is the first call,
 * it will initializae libannocheck.  Otherwise, it will reinit the
 * existing handle.  Returns libannocheck handle on success, NULL
 * otherwise.
 */
static struct libannocheck_internals *libannocheck_setup(struct rpminspect *ri, const rpmfile_entry_t *file, char *opts, struct libannocheck_internals *h)
{
    struct libannocheck_internals *anno = h;
    const char *arch = NULL;
    libannocheck_error annoerr = 0;
    string_list_t *args = NULL;
    string_entry_t *entry = NULL;
    char *test = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    arch = get_rpm_header_arch(file->rpm_header);

    if (anno == NULL) {
        /* initialize libannocheck for this test on this file */
        annoerr = libannocheck_init(LIBANNOCHECK_VERSION, file->fullpath, get_after_debuginfo_path(ri, file, arch), &anno);

        if (annoerr != libannocheck_error_none) {
             warnx(_("libannocheck_init error: %s"), libannocheck_get_error_message(anno, annoerr));
             libannocheck_finish(anno);
             return NULL;
        }

        /*
         * handle annocheck options if there are any, otherwise
         * enable all tests
         */
        if (opts) {
            args = strsplit(opts, " \t");

            if (args) {
                TAILQ_FOREACH(entry, args, items) {
                    if (strstr(entry->data, "=") || !strprefix(entry->data, "--test-") || !strprefix(entry->data, "--skip-")) {
                        continue;
                    }

                    /* the argument past the leading --test- or --skip- */
                    test = entry->data + 7;

                    if (strprefix(entry->data, "--test-")) {
                        annoerr = libannocheck_enable_test(anno, test);
                        test = "enable";
                    } else {
                        annoerr = libannocheck_disable_test(anno, test);
                        test = "disable";
                    }

                    if (annoerr != libannocheck_error_none) {
                        warnx(_("libannocheck_%s_test error: %s:"), test, libannocheck_get_error_message(anno, annoerr));
                        libannocheck_finish(anno);
                        return NULL;
                    }
                }

                list_free(args, free);
            }
        } else {
            annoerr = libannocheck_enable_all_tests(anno);

            if (annoerr != libannocheck_error_none) {
                warnx(_("libannocheck_enable_all_tests error: %s:"), libannocheck_get_error_message(anno, annoerr));
                libannocheck_finish(anno);
                return NULL;
            }
        }

        /* enable libannocheck profile if there's a match */
        set_libannocheck_profile(anno, ri->product_release);
    } else {
        /* reinitialize with a new file */
        annoerr = libannocheck_reinit(anno, file->fullpath, get_after_debuginfo_path(ri, file, arch));

        if (annoerr != libannocheck_error_none) {
             warnx(_("libannocheck_reinit error: %s"), libannocheck_get_error_message(anno, annoerr));
             libannocheck_finish(anno);
             return NULL;
        }
    }

    return anno;
}

static bool annocheck_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    string_map_t *hentry = NULL;
    string_map_t *tmp_hentry = NULL;
    struct libannocheck_internals *ah = NULL;
    libannocheck_error annoerr = 0;
    struct libannocheck_test *annotests = NULL;
    unsigned int i = 0;
    unsigned int numtests = 0;
    unsigned int failed = 0;
    unsigned int maybe = 0;
    char *tmp = NULL;
    string_list_t *details = NULL;
    libannocheck_test_state after_worst = 0;
    libannocheck_test_state before_worst = 0;
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
    params.header = NAME_ANNOCHECK;
    params.arch = arch;
    params.file = file->localpath;

    /* Run each annocheck test and report the results */
    HASH_ITER(hh, ri->annocheck, hentry, tmp_hentry) {
        /* run libannocheck on the before build (if any) first */
        if (file->peer_file) {
            ah = libannocheck_setup(ri, file->peer_file, hentry->value, ah);

            if (ah == NULL) {
                continue;
            }

            /* run the tests on the before build file (if any) */
            annoerr = libannocheck_run_tests(ah, &failed, &maybe);

            if (annoerr != libannocheck_error_none) {
                warnx(_("blurp libannocheck_run_tests error: %s:"), libannocheck_get_error_message(ah, annoerr));
                libannocheck_finish(ah);
                continue;
            }

            /* collect the worst result from the build build now */
            annoerr = libannocheck_get_known_tests(ah, &annotests, &numtests);

            if (annoerr != libannocheck_error_none) {
                warnx(_("libannocheck_get_known_tests error: %s"), libannocheck_get_error_message(ah, annoerr));
                libannocheck_finish(ah);
                continue;
            }

            for (i = 0; i < numtests; i++) {
                if (!annotests[i].enabled || annotests[i].state == libannocheck_test_state_not_run) {
                    continue;
                }

                /* capture worst result */
                before_worst = get_worst(before_worst, annotests[i].state);
            }

            /* close the handle and prepare to start over */
            annoerr = libannocheck_finish(ah);

            if (annoerr != libannocheck_error_none) {
                warnx(_("libannocheck_finish error: %s"), libannocheck_get_error_message(ah, annoerr));
            }

            ah = NULL;
        }

        /* set up libannocheck for the after build file */
        ah = libannocheck_setup(ri, file, hentry->value, ah);

        if (ah == NULL) {
            continue;
        }

        /* run the tests on the after build file */
        annoerr = libannocheck_run_tests(ah, &failed, &maybe);

        if (annoerr != libannocheck_error_none) {
            warnx(_("flurfle libannocheck_run_tests error: %s:"), libannocheck_get_error_message(ah, annoerr));
            libannocheck_finish(ah);
            continue;
        }

        /* get the list of known tests */
        annoerr = libannocheck_get_known_tests(ah, &annotests, &numtests);

        if (annoerr != libannocheck_error_none) {
             warnx(_("libannocheck_get_known_tests error: %s"), libannocheck_get_error_message(ah, annoerr));
             libannocheck_finish(ah);
             continue;
        }

        /* build details */
        for (i = 0; i < numtests; i++) {
            if (!annotests[i].enabled || annotests[i].state == libannocheck_test_state_not_run) {
                continue;
            }

            /* build the detailed reporting similar to annocheck(1) */
            xasprintf(&tmp, "Hardened: %s: %.4s: '%s' test", file->localpath, get_state(annotests[i].state), annotests[i].name);
            assert(tmp != NULL);
            details = list_add(details, tmp);
            free(tmp);

            xasprintf(&tmp, "Hardened: %s: %.4s: %s", file->localpath, get_state(annotests[i].state), annotests[i].description);
            assert(tmp != NULL);
            details = list_add(details, tmp);
            free(tmp);

            if (annotests[i].state == libannocheck_test_state_failed || annotests[i].state == libannocheck_test_state_maybe) {
                xasprintf(&tmp, "Hardened: %s: %.4s: %s", file->localpath, get_state(annotests[i].state), annotests[i].doc_url);
                assert(tmp != NULL);
                details = list_add(details, tmp);
                free(tmp);
            }

            /* handle loss of -O2 -D_FORTIFY_SOURCE for reporting */
            if ((!strcmp(annotests[i].name, "fortify") || !strcmp(annotests[i].name, "optimization")) &&
                (annotests[i].state == libannocheck_test_state_maybe || annotests[i].state == libannocheck_test_state_failed)) {
                params.waiverauth = WAIVABLE_BY_SECURITY;
                params.remedy = REMEDY_ANNOCHECK_FORTIFY_SOURCE;
                params.verb = VERB_REMOVED;
                params.noun = _("lost -D_FORTIFY_SOURCE in ${FILE} on ${ARCH}");
                params.severity = get_secrule_result_severity(ri, file, SECRULE_FORTIFYSOURCE);

                xasprintf(&params.msg, _("%s may have lost -O2 -D_FORTIFY_SOURCE on %s"), file->localpath, arch);
                params.details = NULL;

                if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                    add_result(ri, &params);
                    reported = true;
                }

                free(params.msg);
                params.msg = NULL;
            }

            /* capture worst result */
            after_worst = get_worst(after_worst, annotests[i].state);
        }

        /* report the results */
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.remedy = REMEDY_ANNOCHECK;
        params.verb = VERB_OK;

        if (after_worst == libannocheck_test_state_maybe || after_worst == libannocheck_test_state_failed) {
            params.severity = ri->annocheck_failure_severity;
            params.waiverauth = WAIVABLE_BY_ANYONE;
        }

        if (file->peer_file) {
            if (before_worst == libannocheck_test_state_passed && after_worst == libannocheck_test_state_passed) {
                xasprintf(&params.msg, _("libannocheck '%s' continues passing for %s on %s"), hentry->key, file->localpath, arch);
            } else if ((before_worst == libannocheck_test_state_maybe || before_worst == libannocheck_test_state_failed) &&
                       after_worst == libannocheck_test_state_passed) {
                xasprintf(&params.msg, _("libannocheck '%s' test now passes for %s on %s"), hentry->key, file->localpath, arch);
            } else if (before_worst == libannocheck_test_state_passed &&
                       (after_worst == libannocheck_test_state_maybe || after_worst == libannocheck_test_state_failed)) {
                xasprintf(&params.msg, _("libannocheck '%s' test now fails for %s on %s"), hentry->key, file->localpath, arch);
                params.verb = VERB_CHANGED;
            } else if (after_worst == libannocheck_test_state_maybe || after_worst == libannocheck_test_state_failed) {
                xasprintf(&params.msg, _("libannocheck '%s' test fails for %s on %s"), hentry->key, file->localpath, arch);
                params.verb = VERB_CHANGED;
            }
        } else {
            if (after_worst == libannocheck_test_state_passed) {
                xasprintf(&params.msg, _("libannocheck '%s' test passes for %s on %s"), hentry->key, file->localpath, arch);
            } else {
                xasprintf(&params.msg, _("libannocheck '%s' test fails for %s on %s"), hentry->key, file->localpath, arch);
                params.verb = VERB_CHANGED;
            }
        }

        params.details = list_to_string(details, "\n");
        add_result(ri, &params);
        reported = true;
        free(params.details);
        free(params.msg);
        list_free(details, free);
    }

    /* set result based on worst result encountered */
    if (after_worst >= libannocheck_test_state_maybe) {
        result = !(ri->annocheck_failure_severity >= RESULT_VERIFY);
    }

    /* close out */
    annoerr = libannocheck_finish(ah);

    if (annoerr != libannocheck_error_none) {
        warnx(_("libannocheck_finish error: %s"), libannocheck_get_error_message(ah, annoerr));
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
