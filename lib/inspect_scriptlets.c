/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <rpm/rpmtag.h>
#include "rpminspect.h"

/*
 * The types of scriptlets that can appear in an RPM header.  These
 * are defined in spec files and can be used to perform additional
 * actions at package installation or removal time.
 */
static scriptlet_type_t scriptlet_types[] = {
    { RPMTAG_PRETRANS,     RPMTAG_PRETRANSPROG,     "%pretrans" },
    { RPMTAG_PREIN,        RPMTAG_PREINPROG,        "%pre" },
    { RPMTAG_POSTIN,       RPMTAG_POSTINPROG,       "%post" },
    { RPMTAG_PREUN,        RPMTAG_PREUNPROG,        "%preun" },
    { RPMTAG_POSTUN,       RPMTAG_POSTUNPROG,       "%postun" },
    { RPMTAG_POSTTRANS,    RPMTAG_POSTTRANSPROG,    "%posttrans" },
    { RPMTAG_VERIFYSCRIPT, RPMTAG_VERIFYSCRIPTPROG, "%verifyscript" },
    { RPMTAG_NOT_FOUND,    RPMTAG_NOT_FOUND,        NULL }
};

static bool reported = false;
static bool result = true;
static struct result_params params;

/* Free memory associated with the scriptlet_t */
static void free_scriptlet(scriptlet_t *scriptlet)
{
    if (scriptlet == NULL) {
        return;
    }

    free(scriptlet->script);
    free(scriptlet->args);
    free(scriptlet);
    return;
}

/*
 * Given an RPM header and a scriptlet type array entry, return a
 * scriptlet_t containing the program interpreter and the scriptlet
 * contents.  The caller must free the returned result.  NULL is
 * returned if the specified scriptlet does not exist in the package.
 */
static scriptlet_t *get_scriptlet(Header h, const scriptlet_type_t scriptlet_type)
{
    scriptlet_t *scriptlet = NULL;

    if (h == NULL) {
        return NULL;
    }

    /* allocate space for this scriptlet */
    scriptlet = calloc(1, sizeof(*scriptlet));
    assert(scriptlet != NULL);

    /* get the scriptlet interpreter */
    scriptlet->script = get_rpmtag_str(h, scriptlet_type.type);

    if (scriptlet->script == NULL) {
        free(scriptlet);
        return NULL;
    }

    /* get the scriptlet contents */
    scriptlet->args = get_rpmtag_str(h, scriptlet_type.prog);

    return scriptlet;
}

/*
 * Given a scriptlet as a string_list_t, walk the lines and join
 * together continued lines.  That is, the lines that end with "\".
 * Return newly allocated string representing the script.
 */
static char *join_scriptlet_lines(char *script)
{
    string_list_t *lines = NULL;
    string_list_t *new = NULL;
    string_entry_t *entry = NULL;
    string_entry_t *next = NULL;
    string_entry_t *jentry = NULL;
    char *joined = NULL;

    assert(script != NULL);

    /* split in to individual lines */
    lines = strsplit(script, "\n");

    /* walk the lines and join those that need it */
    if (new == NULL) {
        new = calloc(1, sizeof(*new));
        assert(new != NULL);
        TAILQ_INIT(new);
    }

    while (!TAILQ_EMPTY(lines)) {
        entry = TAILQ_FIRST(lines);
        TAILQ_REMOVE(lines, entry, items);

        if (strsuffix(entry->data, "\\") && !TAILQ_EMPTY(lines)) {
            next = TAILQ_FIRST(lines);
            TAILQ_REMOVE(lines, next, items);

            entry->data[strcspn(entry->data, "\\")] = '\0';
            next->data = strtrim(next->data);

            if (jentry) {
                jentry->data = strappend(jentry->data, entry->data, next->data, NULL);
            } else {
                jentry = entry;
                jentry->data = strappend(jentry->data, next->data, NULL);
            }

            free(next->data);
            free(next);
        } else {
            if (jentry) {
                TAILQ_INSERT_TAIL(new, jentry, items);
                jentry = NULL;
            }

            TAILQ_INSERT_TAIL(new, entry, items);
        }
    }

    joined = list_to_string(new, "\n");
    list_free(new, free);

    return joined;
}

static bool is_allowed_uid(const uid_list_t *allowed_uids, uid_t uid)
{
    uid_entry_t *entry = NULL;

    if (allowed_uids == NULL || TAILQ_EMPTY(allowed_uids)) {
        return true;
    }

    TAILQ_FOREACH(entry, allowed_uids, items) {
        if (entry->uid == uid) {
            return true;
        }
    }

    return false;
}

/*
 * Check for uses of the command 'useradd':
 *     a) Look for " -u" and " -s" to avoid matching
 *        privilege-seperated ssh; treat tabs as spaces
 *     b) Look for " -u" or " --uid" or " -r".  If none of these are
 *        present, report a BAD result indicating useradd must specify a
 *        UID
 *     c) If the UID is specified, extract it and report VERIFY if it
 *        is over 200 (where does this rule come from?)
 *     d) Look for " -s" or " --shell".  If none of these are present,
 *        report BAD indicating a login shell must be specified
 *     e) If the shell is specified, ensure it is
 *        "[/usr]/sbin/nologin".  If not, report the specified shell
 *        as INFO (??? not VERIFY?)
 */
static void check_useradd(struct rpminspect *ri, const Header h, const char *name, scriptlet_t *scriptlet)
{
    string_list_t *lines = NULL;
    string_list_t *tokens = NULL;
    string_entry_t *entry = NULL;
    string_entry_t *token = NULL;
    string_entry_t *arg = NULL;
    bool found_useradd = false;
    bool sflag = false;
    bool uflag = false;
    bool rflag = false;
    char *shell = NULL;
    uid_t uid = 0;
    const char *pkg = NULL;
    const char *arch = NULL;

    assert(ri != NULL);
    assert(h != NULL);
    assert(name != NULL);

    if (scriptlet == NULL) {
        /*
         * NULL input means the driver loop found a before/after
         * comparison where the after scriptlet is lost.  Return here
         * and let the main loop report the loss of the scriptlet.
         */
        return;
    }

    /* information about this RPM */
    pkg = headerGetString(h, RPMTAG_NAME);
    arch = get_rpm_header_arch(h);

    /* join any continued scriptlet lines */
    scriptlet->script = join_scriptlet_lines(scriptlet->script);

    /* break it up in to lines */
    lines = strsplit(scriptlet->script, "\n");

    /* look for any use of the account commands */
    TAILQ_FOREACH(entry, lines, items) {
        if (strstr(entry->data, USERADD_CMD)) {
            tokens = strsplit(entry->data, " ");
            found_useradd = false;

            TAILQ_FOREACH(token, tokens, items) {
                if (!found_useradd && strsuffix(token->data, USERADD_CMD)) {
                    found_useradd = true;
                } else if (found_useradd) {
                    if (!strcmp(token->data, "-r") || !strcmp(token->data, "--system")) {
                        rflag = true;
                    } else if (!strcmp(token->data, "-s") || !strcmp(token->data, "--shell")) {
                        sflag = true;
                        arg = TAILQ_NEXT(token, items);

                        if (arg == NULL) {
                            sflag = false;
                        } else {
                            shell = strdup(arg->data);
                        }
                    } else if (!strcmp(token->data, "-u") || !strcmp(token->data, "--uid")) {
                        uflag = true;
                        arg = TAILQ_NEXT(token, items);

                        if (arg == NULL) {
                            uflag = false;
                        } else {
                            errno = 0;
                            uid = strtol(arg->data, NULL, 10);

                            if (errno == ERANGE || errno == EINVAL) {
                                uid = -1;
                                uflag = false;
                            }
                        }
                    }
                }
            }

            list_free(tokens, free);

            /* check and report */
            if (found_useradd) {
                if (is_rebase(ri)) {
                    params.severity = RESULT_INFO;
                } else {
                    params.severity = RESULT_VERIFY;
                }

                params.waiverauth = WAIVABLE_BY_ANYONE;

                if (((!uflag && uid == 0) || !rflag) && (!sflag && shell == NULL)) {
                    /* useradd found but missing options */
                    xasprintf(&params.msg, _("The %s command is present in the %s scriptlet in the %s package on %s, but it is missing the required options (-r or -u/--uid as well as -s/--shell)."), USERADD_CMD, name, pkg, arch);
                    params.details = entry->data;
                    params.remedy = REMEDY_REQUIRED_USERADD_OPTIONS;
                    add_result(ri, &params);
                    free(params.msg);

                    reported = true;
                    result = false;
                }

                if (!uflag && !rflag && sflag) {
                    /* useradd missing -u or -r option */
                    xasprintf(&params.msg, _("The %s command is present in the %s scriptlet in the %s package on %s, but it is missing either the -r/--system option or the -u/--uid option."), USERADD_CMD, name, pkg, arch);
                    params.details = entry->data;
                    params.remedy = REMEDY_USERADD_OPTION_UID;
                    add_result(ri, &params);
                    free(params.msg);

                    reported = true;
                    result = false;
                }

                if (uflag && uid > ri->uid_boundary && !is_allowed_uid(ri->allowed_uids, uid)) {
                    /* useradd specified with a UID greater than the upper bound and not an allowed UID */
                    xasprintf(&params.msg, _("The %s command is present in the %s scriptlet in the %s package on %s, but it specifies a UID value greater than the defined boundary in the rpminspect configuration file: %d > %d.  And it is not an allowed UID as defined in the rpminspect configuration file."), USERADD_CMD, name, pkg, arch, uid, ri->uid_boundary);
                    params.details = entry->data;
                    params.remedy = REMEDY_USERADD_UID_BOUNDARY;
                    add_result(ri, &params);
                    free(params.msg);

                    reported = true;
                    result = false;
                }

                if (sflag && !strsuffix(shell, "/nologin")) {
                    /* useradd with a shell other than nologin */
                    xasprintf(&params.msg, _("The %s command is present in the %s scriptlet in the %s package on %s, but it specifies a login shell other than `nologin': %s"), USERADD_CMD, name, pkg, arch, shell);
                    params.details = entry->data;
                    params.remedy = REMEDY_USERADD_OPTION_SHELL;
                    add_result(ri, &params);
                    free(params.msg);

                    reported = true;
                    result = false;
                }
            }
        }
    }

    free(shell);
    list_free(lines, free);
    return;
}

static void scriptlets_driver(struct rpminspect *ri, rpmpeer_entry_t *peer)
{
    int i = 0;
    scriptlet_t *before_scriptlet = NULL;
    scriptlet_t *after_scriptlet = NULL;

    assert(ri != NULL);
    assert(peer != NULL);

    /* skip source packages */
    if (headerIsSource(peer->after_hdr)) {
        return;
    }

    /* skip debuginfo and debugsource packages */
    if (is_debuginfo_rpm(peer->after_hdr) || is_debugsource_rpm(peer->after_hdr)) {
        return;
    }

    /* look for each possible scriptlet in the header; if found perform checks */
    for (i = 0; scriptlet_types[i].type != RPMTAG_NOT_FOUND; i++ ) {
        /* get the before and after package scriptlets */
        after_scriptlet = get_scriptlet(peer->after_hdr, scriptlet_types[i]);

        if (peer->before_hdr != NULL) {
            before_scriptlet = get_scriptlet(peer->before_hdr, scriptlet_types[i]);
        }

        if (after_scriptlet == NULL && before_scriptlet == NULL) {
            continue;
        }

        /* check for correct useradd usage */
        check_useradd(ri, peer->after_hdr, scriptlet_types[i].name, after_scriptlet);

        /* cleanup */
        free_scriptlet(after_scriptlet);
        free_scriptlet(before_scriptlet);
    }

/*
 * what the old thing did:
 *
 * 4) If there is a peer package, get the same scriptlet from the
 *    before package and diff the contents.  Before comparing the
 *    contents, replace the package version and release in the form of
 *    VR and NVR in the before and after scriptlets appropriately.
 *    Report this way:
 *    a) If the scripts differ:
 *       i) INFO and not waivable if the after script contains no
 *          security keywords.
 *       ii) VERIFY and waivable by security if the after script
 *           contains any security keywords.  Security keywords found
 *           should be highlighting in the details reporting of the
 *           results message (like line number and the line itself).
 *    b) If the after package loses a previously defined scriptlet,
 *       report that as a loss with a VERIFY result waivable by
 *       anyone.  If this is a rebase, it should be an INFO result.
 *    c) If the after package gains a previously undefined scriptlet,
 *       report that as INFO and not waivable.
 */



    return;
}

/*
 * Main driver for the 'scriptlets' inspection.
 */
bool inspect_scriptlets(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    init_result_params(&params);
    params.header = NAME_SCRIPTLETS;

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are reported via INSPECT_EMPTYRPM */
        if (peer->after_rpm == NULL) {
            continue;
        }

        scriptlets_driver(ri, peer);
    }

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_SCRIPTLETS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
