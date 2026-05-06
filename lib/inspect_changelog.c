/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <rpm/header.h>

#include "rpminspect.h"

static struct result_params params;
static regex_list_t *forbidden = NULL;

/*
 * Initialize a list of compiled regular expressions that the
 * changelog inspection will use to look for forbidden entries.
 */
static void init_forbidden_regex(const string_list_t *list)
{
    regex_entry_t *rentry = NULL;
    string_entry_t *sentry = NULL;

    /* do nothing if no forbidden regexps are in the config file */
    if (list == NULL || TAILQ_EMPTY(list)) {
        return;
    }

    /* initialize the list */
    forbidden = xalloc(sizeof(*forbidden));
    assert(forbidden != NULL);
    TAILQ_INIT(forbidden);

    /* turn each regular expression in a regex_t ready to use */
    TAILQ_FOREACH(sentry, list, items) {
        rentry = xalloc(sizeof(*rentry));
        assert(rentry != NULL);
        rentry->src = sentry->data;

        if (add_regex(rentry->src, &rentry->re) != 0) {
            warnx(_("*** error processing forbidden changelog regular expression: %s"), rentry->src);
        } else {
            TAILQ_INSERT_TAIL(forbidden, rentry, items);
        }
    }

    return;
}

/*
 * Cleanup for the compiled forbidden regular expressions.
 */
static void free_forbidden_regex(void)
{
    regex_entry_t *entry = NULL;

    if (forbidden == NULL || TAILQ_EMPTY(forbidden)) {
        return;
    }

    while (!TAILQ_EMPTY(forbidden)) {
        entry = TAILQ_FIRST(forbidden);
        TAILQ_REMOVE(forbidden, entry, items);
        free_regex(entry->re);
        free(entry);
    }

    free(forbidden);
    return;
}

/*
 * Check the given string for any forbidden regexp matches.
 */
static const char *has_forbidden_match(const char *s)
{
    int r = -1;
    regmatch_t match[1];
    regex_entry_t *entry = NULL;

    if (s == NULL || (forbidden == NULL || TAILQ_EMPTY(forbidden))) {
        return NULL;
    }

    TAILQ_FOREACH(entry, forbidden, items) {
        r = regexec(entry->re, s, 1, match, REG_EXTENDED);

        if (r == 0 && match[0].rm_so > -1) {
            return entry->src;
        }
    }

    return NULL;
}

/*
 * Given an RPM header, read the %changelog and reconstruct it as
 * a string_list_t where each entry is a changelog entry.
 */
static string_list_t *get_changelog(const Header hdr)
{
    string_list_t *changelog = NULL;
    string_entry_t *entry = NULL;
    rpmtd times = NULL;
    rpmtd names = NULL;
    rpmtd lines = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    time_t timestamp;
    struct tm *logtime = NULL;
    const char *name = NULL;
    const char *line = NULL;
    int i = 0;
    char tbuf[16];

    /* empty header? */
    if (hdr == NULL) {
        return NULL;
    }

    /* start the changelog */
    changelog = xalloc(sizeof(*changelog));
    TAILQ_INIT(changelog);

    /* Read this RPM header and construct a new changelog */
    times = rpmtdNew();
    names = rpmtdNew();
    lines = rpmtdNew();

    if (headerGet(hdr, RPMTAG_CHANGELOGTIME, times, flags) &&
        headerGet(hdr, RPMTAG_CHANGELOGNAME, names, flags) &&
        headerGet(hdr, RPMTAG_CHANGELOGTEXT, lines, flags)) {
        while ((rpmtdNext(times) != -1) &&
               (rpmtdNext(names) != -1) &&
               (rpmtdNext(lines) != -1)) {
            /* Get all the parts of the changelog entry */
            timestamp = rpmtdGetNumber(times);
            logtime = gmtime(&timestamp);
            name = rpmtdGetString(names);
            line = rpmtdGetString(lines);

            /* Convert the time in to an RPM-like string */
            if (logtime) {
                strftime(tbuf, sizeof(tbuf), "%a %b %d %Y", logtime);
            } else {
                /* gmtime() failed, but just move on */
                for (i = 0; i < 15; i++) {
                    tbuf[i] = '?';
                }

                tbuf[15] = '\0';
            }

            /* Create a new changelog entry */
            /*
             * The entry reconstruction comes from rpmpopt.in in the rpm
             * source:
             *     rpm alias --changes --qf '[* %{CHANGELOGTIME:date} %{CHANGELOGNAME}\n%{CHANGELOGTEXT}\n\n]' \
             *         --POPTdesc=$"list changes for this package with full time stamps"
             * Which is worth noting here because when you query the changelog
             * from an RPM (rpm -qp --changelog), it is reproducing the
             * %changelog section from the spec file entry by entry and the
             * actual number of blank lines may not be the same.
             */
            entry = xalloc(sizeof(*entry));
            xasprintf(&entry->data, "* %s %s\n%s\n\n", tbuf, name, line);

            DEBUG_PRINT("\n%s\n", entry->data);

            /* Add the changelog entry to the list */
            TAILQ_INSERT_TAIL(changelog, entry, items);
        }
    }

    /* Cleanup */
    rpmtdFreeData(times);
    rpmtdFreeData(names);
    rpmtdFreeData(lines);
    rpmtdFree(times);
    rpmtdFree(names);
    rpmtdFree(lines);

    return changelog;
}

/*
 * Generate temporary changelog files for use with diff(1).
 */
static char *create_changelog(const string_list_t *changelog, const char *where)
{
    int fd;
    char *output = NULL;
    string_entry_t *entry = NULL;
    FILE *logfp = NULL;

    assert(where != NULL);

    /* no changelog data means no changelog file */
    if (changelog == NULL) {
        return NULL;
    }

    xasprintf(&output, "%s/changelog.XXXXXX", where)
    fd = mkstemp(output);

    if (fd == -1) {
        warn("*** mkstemp");
        free(output);
        return NULL;
    }

    logfp = fdopen(fd, "w");

    if (logfp == NULL) {
        warn("*** fdopen");
        close(fd);
        unlink(output);
        free(output);
        return NULL;
    }

    TAILQ_FOREACH(entry, changelog, items) {
        fprintf(logfp, "%s", entry->data);
    }

    if (fclose(logfp) != 0) {
        warn("*** fclose");
        close(fd);
        unlink(output);
        free(output);
        return NULL;
    }

    return output;
}

/*
 * Given 'diff -u' output, advance the string past the headers.
 */
static char *skip_diff_headers(char *diff_output)
{
    char *diff_walk = diff_output;

    if (diff_output == NULL) {
        return diff_output;
    }

    if (!strncmp(diff_walk, "--- ", 4)) {
        diff_walk = xstrchr(diff_walk, '\n') + 1;

        while (*diff_walk == '\n') {
            diff_walk++;
        }
    }

    if (!strncmp(diff_walk, "+++ ", 4)) {
        diff_walk = xstrchr(diff_walk, '\n') + 1;

        while (*diff_walk == '\n') {
            diff_walk++;
        }
    }

    return diff_walk;
}

/*
 * Perform %changelog checks on the SRPM packages between builds. Do
 * the following:
 *     - Report if the %changelog was removed in the after build but
 *       was preset in the before build. (VERIFY)
 *     - Report if the %changelog was missing in the before build but
 *       added in the after build. (INFO)
 *     - Report if the first entries in the before and after changelogs
 *       are identical (report as "no new changelog entry"). (BAD)
 */
static bool check_src_rpm_changelog(struct rpminspect *ri, const rpmpeer_entry_t *peer)
{
    bool result = true;
    char *before_nevr = NULL;
    char *after_nevr = NULL;
    string_list_t *before_changelog = NULL;
    string_list_t *after_changelog = NULL;
    string_entry_t *before = NULL;
    string_entry_t *after = NULL;
    char *before_output = NULL;
    char *after_output = NULL;
    char *diff_output = NULL;
    const char *regexp = NULL;

    assert(ri != NULL);
    assert(peer != NULL);

    /* get reporting information */
    before_nevr = get_nevr(peer->before_hdr);
    after_nevr = get_nevr(peer->after_hdr);

    /* get the before and after changelogs */
    before_changelog = get_changelog(peer->before_hdr);
    after_changelog = get_changelog(peer->after_hdr);

    /* compare changelog data */
    if (before_changelog) {
        before = TAILQ_FIRST(before_changelog);
        before_output = create_changelog(before_changelog, ri->workdir);
    }

    if (after_changelog) {
        after = TAILQ_FIRST(after_changelog);
        after_output = create_changelog(after_changelog, ri->workdir);
    }

    /* Compare the changelogs */
    if (before_output && after_output) {
        diff_output = get_file_delta(before_output, after_output);
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_CHANGELOG;
    params.severity = RESULT_OK;
    params.waiverauth = NOT_WAIVABLE;
    params.noun = _("%%changelog");

    if (diff_output) {
        /* Skip past the diff(1) header lines */
        params.details = skip_diff_headers(diff_output);

        /* Perform checks */
        if (before_changelog && (after_changelog == NULL || TAILQ_EMPTY(after_changelog))) {
            xasprintf(&params.msg, "%%changelog lost between the %s and %s builds", before_nevr, after_nevr);
            params.severity = RESULT_INFO;
            params.verb = VERB_REMOVED;
        } else if ((before_changelog == NULL || TAILQ_EMPTY(before_changelog)) && after_changelog) {
            xasprintf(&params.msg, "Gained %%changelog between the %s and %s builds", before_nevr, after_nevr);
            params.severity = RESULT_INFO;
            params.verb = VERB_ADDED;
        } else if ((before_changelog == NULL || TAILQ_EMPTY(before_changelog)) && (after_changelog == NULL || TAILQ_EMPTY(after_changelog))) {
            xasprintf(&params.msg, "No %%changelog present in the %s build", after_nevr);
            params.severity = RESULT_INFO;
            params.verb = VERB_MISSING;
        }
    } else if (before_changelog == NULL || after_changelog == NULL) {
        if (before_changelog == NULL && after_changelog != NULL) {
            xasprintf(&params.msg, "Gained %%changelog between the %s and %s builds", before_nevr, after_nevr);
            params.severity = RESULT_INFO;
            params.verb = VERB_ADDED;
        } else if (before_changelog != NULL && after_changelog == NULL) {
            xasprintf(&params.msg, "%%changelog lost between the %s and %s builds", before_nevr, after_nevr);
            params.severity = RESULT_INFO;
            params.verb = VERB_REMOVED;
        } else if (before_changelog == NULL && after_changelog == NULL) {
            xasprintf(&params.msg, "No %%changelog present in the %s build", after_nevr);
            params.severity = RESULT_INFO;
            params.verb = VERB_MISSING;
        }
    } else if (before && after && !strcmp(before->data, after->data)) {
        /*
         * Only report that a new entry is missing if the builds have
         * different NVRs.  But compare NVRs with the dist tag
         * trimmed.
         */
        if (strcmp(headerGetString(peer->before_hdr, RPMTAG_NAME), headerGetString(peer->after_hdr, RPMTAG_NAME)) ||
            strcmp(headerGetString(peer->before_hdr, RPMTAG_VERSION), headerGetString(peer->after_hdr, RPMTAG_VERSION)) ||
            (get_before_rel(ri) && get_after_rel(ri) && strcmp(get_before_rel(ri), get_after_rel(ri)))) {
            xasprintf(&params.msg, "No new %%changelog entry in the %s build", after_nevr);
            params.severity = RESULT_INFO;
            params.verb = VERB_MISSING;
        }
    }

    if (params.msg) {
        add_result(ri, &params);
        free(params.msg);
    }

    /* INFO messages are not failures */
    if (params.severity == RESULT_VERIFY || params.severity == RESULT_BAD) {
        result = false;
    }

    /* Check for bad words and forbidden regexp match */
    TAILQ_FOREACH(after, after_changelog, items) {
        if (has_bad_word(after->data, ri->badwords)) {
            xasprintf(&params.msg, "%%changelog entry has unprofessional language in the %s spec file", after_nevr);
            params.severity = RESULT_BAD;
            params.waiverauth = NOT_WAIVABLE;
            params.remedy = REMEDY_CHANGELOG;
            params.details = after->data;
            params.verb = VERB_FAILED;
            params.noun = after->data;
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }

        regexp = has_forbidden_match(after->data);

        if (regexp != NULL) {
            xasprintf(&params.msg, "%%changelog entry matches forbidden regular expression '%s' in the %s spec", regexp, after_nevr);
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_CHANGELOG_FORBIDDEN;
            params.details = after->data;
            params.verb = VERB_FAILED;
            params.noun = after->data;
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }
    }

    /* cleanup */
    if (before_output) {
        if (unlink(before_output) != 0) {
            warn("*** unlink");
        }
    }

    if (after_output) {
        if (unlink(after_output) != 0) {
            warn("*** unlink");
        }
    }

    list_free(before_changelog, free);
    list_free(after_changelog, free);
    free(before_nevr);
    free(after_nevr);
    free(before_output);
    free(after_output);
    free(diff_output);

    return result;
}

/*
 * Perform %changelog checks on a single RPM packages between builds.
 * Do the following:
 *     - Report changed/removed lines or added lines as INFO
 *     - Report unprofessional language and report as BAD
 *     - Report matching forbidden regexps as VERIFY
 */
static bool check_bin_rpm_changelog(struct rpminspect *ri, const rpmpeer_entry_t *peer)
{
    bool result = true;
    char *before_nevr = NULL;
    char *after_nevr = NULL;
    string_list_t *before_changelog = NULL;
    string_list_t *after_changelog = NULL;
    char *before_output = NULL;
    char *after_output = NULL;
    char *diff_output = NULL;
    const char *regexp = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(peer != NULL);

    /* get reporting information */
    before_nevr = get_nevr(peer->before_hdr);
    after_nevr = get_nevr(peer->after_hdr);

    /* get the before and after changelogs */
    before_changelog = get_changelog(peer->before_hdr);
    after_changelog = get_changelog(peer->after_hdr);

    /* Generate temporary changelog files */
    before_output = create_changelog(before_changelog, ri->workdir);
    after_output = create_changelog(after_changelog, ri->workdir);

    /* Compare the changelogs */
    if (before_output && after_output) {
        diff_output = get_file_delta(before_output, after_output);
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_CHANGELOG;
    params.verb = VERB_CHANGED;
    params.noun = _("%%changelog");

    if (diff_output) {
        /* Skip past the diff(1) header lines */
        params.details = skip_diff_headers(diff_output);
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        xasprintf(&params.msg, "%%changelog modified between the %s and %s builds", before_nevr, after_nevr);
        add_result(ri, &params);
        free(params.msg);
    }

    free(diff_output);

    /* Check for bad words and forbidden regexp match */
    TAILQ_FOREACH(entry, after_changelog, items) {
        if (has_bad_word(entry->data, ri->badwords)) {
            xasprintf(&params.msg, "%%changelog entry has unprofessional language in the %s build", after_nevr);
            params.severity = RESULT_BAD;
            params.waiverauth = NOT_WAIVABLE;
            params.remedy = REMEDY_CHANGELOG;
            params.details = entry->data;
            params.verb = VERB_FAILED;
            params.noun = entry->data;
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }

        regexp = has_forbidden_match(entry->data);

        if (regexp != NULL) {
            xasprintf(&params.msg, "%%changelog entry matches forbidden regular expression '%s' in the %s build", regexp, after_nevr);
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = REMEDY_CHANGELOG_FORBIDDEN;
            params.details = entry->data;
            params.verb = VERB_FAILED;
            params.noun = entry->data;
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }
    }

    /* cleanup */
    if (before_output) {
        if (unlink(before_output) != 0) {
            warn("*** unlink");
        }
    }

    if (after_output) {
        if (unlink(after_output) != 0) {
            warn("*** unlink");
        }
    }

    free(before_output);
    free(after_output);
    list_free(before_changelog, free);
    list_free(after_changelog, free);
    free(before_nevr);
    free(after_nevr);

    return result;
}

/*
 * Main driver for the 'changelog' inspection.
 */
bool inspect_changelog(struct rpminspect *ri)
{
    bool src_result = true;
    bool bin_result = true;
    rpmpeer_entry_t *peer = NULL;
    rpmpeer_entry_t *src = NULL;
    rpmpeer_entry_t *bin = NULL;

    assert(ri != NULL);

    /* skip this inspection on modules */
    if (ri->buildtype != KOJI_BUILD_RPM) {
        init_result_params(&params);
        xasprintf(&params.msg, _("Inspection skipped because this build's type is not `rpm'."));
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.header = NAME_CHANGELOG;
        add_result(ri, &params);
        free(params.msg);
        return true;
    }

    /* Initialize regular expressions */
    init_forbidden_regex(ri->changelog_forbidden);

    /* Get the source and one binary package */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (src && bin) {
            break;
        }

        /* we need both a before and an after package */
        if (peer->before_hdr && peer->after_hdr) {
            if (headerIsSource(peer->after_hdr)) {
                src = peer;
            } else {
                bin = peer;
            }
        }
    }

    /* Check the packages */
    if (src) {
        src_result = check_src_rpm_changelog(ri, src);
    }

    if (bin) {
        bin_result = check_bin_rpm_changelog(ri, bin);
    }

    /* Clean up forbidden regular expressions */
    free_forbidden_regex();

    if (src_result && bin_result) {
        if (params.severity == RESULT_OK) {
            init_result_params(&params);
            params.severity = RESULT_OK;
            params.header = NAME_CHANGELOG;
            add_result(ri, &params);
        }

        return true;
    } else {
        return false;
    }
}
