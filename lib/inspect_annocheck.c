/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <libgen.h>

#ifdef _WITH_LIBANNOCHECK
#include <libannocheck.h>
#endif

#include "rpminspect.h"

/* Global variables */
static bool reported = false;
#ifdef _WITH_ANNOCHECK
static const char *annocheck_profile = NULL;
#endif

#ifndef _WITH_LIBANNOCHECK
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
static char *build_annocheck_cmd(const char *cmd, const char *opts, const char *profile, const char *debugpath, const char *path)
{
    char *r = NULL;
    char *tmp = NULL;

    assert(cmd != NULL);
    assert(path != NULL);

    r = strappend(r, cmd, " --no-use-debuginfod", NULL);

    if (opts) {
        r = strappend(r, " ", opts, NULL);
    }

    if (profile) {
        r = strappend(r, " --profile=", profile, NULL);
    }

    if (debugpath) {
        tmp = joindelim(PATH_SEP, debugpath, DEBUG_PATH, NULL);
        r = strappend(r, " --debug-dir=", tmp, NULL);
        free(tmp);
    }

    r = strappend(r, " .", path, NULL);

    return r;
}
#endif

#ifdef _WITH_LIBANNOCHECK
/*
 * Try to map the product release string to an appropriate
 * libannocheck profile.  This is going to be something that will need
 * maintenance over time.  Since the profile list is embedded in
 * libannocheck, there's no easy way to discover changes.  The
 * assumption here is the profile naming scheme remains the same.
 */
static void set_libannocheck_profile(struct libannocheck_internals *anno, const char *annocheck_profile, const char *product_release)
{
    const char **profiles = NULL;
    unsigned int num_profiles = 0;
    unsigned int i = 0;
    libannocheck_error annoerr = 0;
    const char *pr = NULL;

    if (anno == NULL || (annocheck_profile == NULL && product_release == NULL)) {
        return;
    }

    /* if the config file specified a profile, use it */
    if (annocheck_profile) {
        annoerr = libannocheck_enable_profile(anno, annocheck_profile);

        if (annoerr != libannocheck_error_none) {
            warnx(_("*** libannocheck_enable_profile: %s"), libannocheck_get_error_message(anno, annoerr));
        }

        return;
    }

    /* try to match a profile against the product release */

    /* trim any leading periods */
    pr = product_release;
    assert(pr != NULL);

    while (*pr == '.' && *pr != '\0') {
        pr++;
    }

    /* get libannocheck profiles first */
    annoerr = libannocheck_get_known_profiles(anno, &profiles, &num_profiles);

    if (annoerr != libannocheck_error_none) {
         warnx(_("*** libannocheck_get_known_profiles: %s"), libannocheck_get_error_message(anno, annoerr));
         return;
    }

    /* iterate over the profiles to try and find a match */
    for (i = 0; i < num_profiles; i++) {
        /*
         * 'fc' is unique to rpminspect-data-fedora and 'rawhide' is
         * unique to libannocheck, but these should probably be in the
         * config file for rpminspect
         */
        if (strprefix(profiles[i], pr) || (strprefix(pr, "fc") && !strcmp(profiles[i], "rawhide"))) {
            annoerr = libannocheck_enable_profile(anno, profiles[i]);

            if (annoerr != libannocheck_error_none) {
                warnx(_("*** libannocheck_enable_profile: %s"), libannocheck_get_error_message(anno, annoerr));
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
 * it will initialize libannocheck.  Otherwise, it will reinit the
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
        annoerr = libannocheck_init(libannocheck_get_version(), file->fullpath, get_debuginfo_path(ri, file, arch, AFTER_BUILD), &anno);

        if (annoerr != libannocheck_error_none) {
             warnx(_("*** libannocheck_init: %s"), libannocheck_get_error_message(anno, annoerr));
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
                        warnx(_("*** libannocheck_%s_test: %s"), test, libannocheck_get_error_message(anno, annoerr));
                        libannocheck_finish(anno);
                        return NULL;
                    }
                }

                list_free(args, free);
            }
        } else {
            annoerr = libannocheck_enable_all_tests(anno);

            if (annoerr != libannocheck_error_none) {
                warnx(_("*** libannocheck_enable_all_tests: %s"), libannocheck_get_error_message(anno, annoerr));
                libannocheck_finish(anno);
                return NULL;
            }
        }

        /* enable libannocheck profile if there's a match */
        set_libannocheck_profile(anno, ri->annocheck_profile, ri->product_release);
    } else {
        /* reinitialize with a new file */
        annoerr = libannocheck_reinit(anno, file->fullpath, get_debuginfo_path(ri, file, arch, AFTER_BUILD));

        if (annoerr != libannocheck_error_none) {
             warnx(_("*** libannocheck_reinit: %s"), libannocheck_get_error_message(anno, annoerr));
             libannocheck_finish(anno);
             return NULL;
        }
    }

    return anno;
}
#endif

static bool annocheck_driver(struct rpminspect *ri, rpmfile_entry_t *file, rpmpeer_entry_t *peer)
{
    bool result = true;
    bool ignore = false;
    const char *arch = NULL;
    string_map_t *hentry = NULL;
    string_map_t *tmp_hentry = NULL;
    struct result_params params;
#ifdef _WITH_LIBANNOCHECK
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
#else
    char **argv = NULL;
    char *before_cmd = NULL;
    char *after_cmd = NULL;
    char *after_out = NULL;
    int after_exit = 0;
    char *before_out = NULL;
    int before_exit = 0;
    char *details = NULL;
    string_list_t *slist = NULL;
    string_entry_t *sentry = NULL;
#endif

    assert(ri != NULL);
    assert(file != NULL);
    assert(peer != NULL);

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debug and build paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* Only run this check on ELF files */
    if (!is_elf_file(file) || (!is_elf_file(file) && file->peer_file && !is_elf_file(file->peer_file))) {
        return true;
    }

    /* We will skip checks for ignored files */
    ignore = ignore_rpmfile_entry(ri, NAME_ANNOCHECK, file);

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up the result parameters */
    init_result_params(&params);
    params.header = NAME_ANNOCHECK;
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.remedy = REMEDY_ANNOCHECK;
    params.verb = VERB_OK;
    params.arch = arch;
    params.file = file->localpath;

    /* Run each annocheck test and report the results */
    HASH_ITER(hh, ri->annocheck, hentry, tmp_hentry) {
#ifdef _WITH_LIBANNOCHECK
        /* run libannocheck on the before build (if any) first */
        if (file->peer_file) {
            ah = libannocheck_setup(ri, file->peer_file, hentry->value, ah);

            if (ah == NULL) {
                /* failed to initialize libannocheck so call that a failure */
                return false;
            }

            /* run the tests on the before build file (if any) */
            annoerr = libannocheck_run_tests(ah, &failed, &maybe);

            if (annoerr != libannocheck_error_none) {
                warnx(_("*** before libannocheck_run_tests: %s (%d)"), libannocheck_get_error_message(ah, annoerr), annoerr);
                libannocheck_finish(ah);
                continue;
            }

            /* collect the worst result from the build build now */
            annoerr = libannocheck_get_known_tests(ah, &annotests, &numtests);

            if (annoerr != libannocheck_error_none) {
                warnx(_("*** libannocheck_get_known_tests: %s"), libannocheck_get_error_message(ah, annoerr));
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
                warnx(_("*** libannocheck_finish: %s"), libannocheck_get_error_message(ah, annoerr));
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
            warnx(_("*** after libannocheck_run_tests: %s (%d)"), libannocheck_get_error_message(ah, annoerr), annoerr);
            libannocheck_finish(ah);
            continue;
        }

        /* get the list of known tests */
        annoerr = libannocheck_get_known_tests(ah, &annotests, &numtests);

        if (annoerr != libannocheck_error_none) {
             warnx(_("*** libannocheck_get_known_tests: %s"), libannocheck_get_error_message(ah, annoerr));
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
        if (!ignore) {
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
    }

    /* set result based on worst result encountered */
    if (after_worst >= libannocheck_test_state_maybe) {
        result = !(ri->annocheck_failure_severity >= RESULT_VERIFY);
    }

    /* close out */
    if (ah != NULL) {
        annoerr = libannocheck_finish(ah);
    }

    if (annoerr != libannocheck_error_none) {
        warnx(_("*** libannocheck_finish: %s"), libannocheck_get_error_message(ah, annoerr));
    }

    return result;
#else
        /* Run the test on the file */
        after_cmd = build_annocheck_cmd(ri->commands.annocheck, hentry->value, annocheck_profile, get_debuginfo_path(ri, file, arch, AFTER_BUILD), file->localpath);
        argv = build_argv(after_cmd);
        after_out = run_cmd_vp(&after_exit, peer->after_root, argv);
        free_argv(argv);

        /* If we have a before build, run the command on that */
        if (!ignore) {
            if (file->peer_file) {
                before_cmd = build_annocheck_cmd(ri->commands.annocheck, hentry->value, annocheck_profile, get_debuginfo_path(ri, file, arch, BEFORE_BUILD), file->peer_file->localpath);
                argv = build_argv(before_cmd);
                before_out = run_cmd_vp(&before_exit, peer->before_root, argv);
                free_argv(argv);

                /* Build a reporting message if we need to */
                if (before_exit == 0 && after_exit == 0) {
                    xasprintf(&params.msg, _("annocheck '%s' test passes for %s on %s"), hentry->key, file->localpath, arch);
                } else if (before_exit && after_exit == 0) {
                    xasprintf(&params.msg, _("annocheck '%s' test now passes for %s on %s"), hentry->key, file->localpath, arch);
                } else if (before_exit == 0 && after_exit) {
                    xasprintf(&params.msg, _("annocheck '%s' test now fails for %s on %s"), hentry->key, file->localpath, arch);
                    params.severity = ri->annocheck_failure_severity;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    params.verb = VERB_CHANGED;
                    result = !(ri->annocheck_failure_severity >= RESULT_VERIFY);
                } else if (after_exit) {
                    xasprintf(&params.msg, _("annocheck '%s' test fails for %s on %s"), hentry->key, file->localpath, arch);
                    params.severity = ri->annocheck_failure_severity;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    params.verb = VERB_CHANGED;
                    result = !(ri->annocheck_failure_severity >= RESULT_VERIFY);
                }
            } else {
                if (after_exit == 0) {
                    xasprintf(&params.msg, _("annocheck '%s' test passes for %s on %s"), hentry->key, file->localpath, arch);
                } else if (after_exit) {
                    xasprintf(&params.msg, _("annocheck '%s' test fails for %s on %s"), hentry->key, file->localpath, arch);
                    params.severity = ri->annocheck_failure_severity;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
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
        }

        /* Check for loss of -O2 -D_FORTIFY_SOURCE=2 */
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
#endif
}

/*
 * Main driver for the 'annocheck' inspection.
 */
bool inspect_annocheck(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct result_params params;

    assert(ri != NULL);

#ifdef _WITH_ANNOCHECK
    /* skip if we have no annocheck tests defined */
    if (ri->annocheck == NULL) {
        return true;
    }

    /* XXX: determine a annocheck_profile string */
    /* this is a workaround until we can drop annocheck(1) support */
    if (strprefix(ri->product_release, "el7")) {
        annocheck_profile = "el7";
    } else if (strprefix(ri->product_release, "el8")) {
        annocheck_profile = "el8";
    } else if (strprefix(ri->product_release, "el9")) {
        annocheck_profile = "el9";
    } else if (strprefix(ri->product_release, "el10")) {
        annocheck_profile = "el10";
    } else if (strprefix(ri->product_release, "rhivos")) {
        annocheck_profile = "rhivos";
    } else if (strprefix(ri->product_release, "fc35")) {
        annocheck_profile = "f35";
    } else if (strprefix(ri->product_release, "fc")) {
        annocheck_profile = "fedora";
    } else if (!strcmp(ri->product_release, "rawhide")) {
        annocheck_profile = "rawhide";
    }
#endif

    /* Prevent debuginfod from fetching debuginfo packages. */
    if (unsetenv("DEBUGINFOD_URLS") == -1) {
        warn("*** unsetenv");
    }

    /* run the annocheck tests across all ELF files */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* Ignore files we should be ignoring */
            if (ignore_path(ri, NAME_ANNOCHECK, file->localpath, peer->after_root) && !has_security_checks(NAME_ANNOCHECK)) {
                continue;
            }

            if (!annocheck_driver(ri, file, peer)) {
                result = false;
            }
        }
    }

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
