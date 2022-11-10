/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect_license.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief 'license' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <assert.h>
#include <errno.h>
#include <err.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <json.h>
#include "rpminspect.h"

/* Local helper functions */
static struct json_object *read_licensedb(struct rpminspect *ri, const char *db)
{
    struct json_object *r = NULL;
    char *actualdb = NULL;
    struct json_tokener *tok = NULL;
    enum json_tokener_error jerr;
    int len = 0;
    char *licdata = NULL;
    off_t liclen = 0;

    assert(ri != NULL);
    assert(db != NULL);

    /* build path to license db if necessary */
    if (db && db[0] == '/') {
        actualdb = strdup(db);
    } else {
        xasprintf(&actualdb, "%s/%s/%s", ri->vendor_data_dir, LICENSES_DIR, db);
    }

    assert(actualdb != NULL);

    if (access(actualdb, F_OK|R_OK)) {
        warn(_("unable to open license db %s"), actualdb);
        free(actualdb);
        return NULL;
    }

    licdata = read_file_bytes(actualdb, &liclen);
    free(actualdb);
    tok = json_tokener_new();
    assert(tok != NULL);

    len = strlen(licdata);
    r = json_tokener_parse_ex(tok, licdata, len);
    jerr = json_tokener_get_error(tok);

    if (jerr != json_tokener_success) {
        warnx("json_tokener_parse_ex: %s", json_tokener_error_desc(jerr));
    }

    json_tokener_free(tok);
    free(licdata);

    return r;
}

/*
 * Called by is_valid_license() to check each short license token.  It
 * will also try to do a whole match on the license tag string.
 */
static bool check_license_abbrev(const struct json_object *db, const char *lic)
{
    struct json_object *ldb = (struct json_object *) db;
    const char *fedora_abbrev = "";
    const char *fedora_name = "";
    const char *spdx_abbrev = "";
    bool approved = false;

    assert(ldb != NULL);
    assert(lic != NULL);

    json_object_object_foreach(ldb, license_name, val) {
        /* first reset our variables */
        fedora_abbrev = "";
        fedora_name = "";
        spdx_abbrev = "";
        approved = false;

        if (strlen(license_name) == 0) {
            continue;
        }

        /* collect the properties */
        json_object_object_foreach(val, prop, propval) {
            if (!strcmp(prop, "fedora_abbrev")) {
                fedora_abbrev = json_object_get_string(propval);
            } else if (!strcmp(prop, "fedora_name")) {
                fedora_name = json_object_get_string(propval);
            } else if (!strcmp(prop, "spdx_abbrev")) {
                spdx_abbrev = json_object_get_string(propval);
            } else if (!strcmp(prop, "approved") && !strcasecmp(json_object_get_string(propval), "yes")) {
                approved = true;
            }
        }

        /* handle "commented out" fields */
        if (strlen(fedora_abbrev) > 0 && *fedora_abbrev == '#') {
            fedora_abbrev = "";
        }

        if (strlen(spdx_abbrev) > 0 && *spdx_abbrev == '#') {
            spdx_abbrev = "";
        }

        /*
         * no full tag match and no abbreviations, license entry invalid
         */
        if (strlen(fedora_abbrev) == 0 && strlen(spdx_abbrev) == 0 && strlen(fedora_name) == 0) {
            continue;
        }

        /*
         * if the entire license string is approved, that is valid
         * if we hit 'fedora_abbrev', that is valid
         * if we hit 'spdx_abbrev' and approved is true, that is valid
         * NOTE: we only match the first hit in the license database
         */
        if (approved && ((!strcmp(lic, fedora_abbrev) || !strcmp(lic, spdx_abbrev)) ||
                         (strlen(fedora_abbrev) == 0 && strlen(spdx_abbrev) == 0 && !strcmp(lic, fedora_name)))) {
            return true;
        }
    }

    return false;
}

/*
 * Dupe or append a token to the given string.
 */
static void token_append(char **dest, const char *token)
{
    char *newdest = NULL;

    assert(token != NULL);

    if (*dest == NULL) {
        *dest = strdup(token);
        assert(*dest != NULL);
    } else {
        xasprintf(&newdest, "%s %s", *dest, token);
        assert(newdest != NULL);
        free(*dest);
        *dest = newdest;
    }

    return;
}

/*
 * Given a license expression without parens, check it.
 */
static bool is_valid_expression(const struct json_object *db, const char *s, const char *nevra, struct result_params *params, results_t **rq)
{
    bool result = true;
    char *tagtokens = NULL;
    char *tagcopy = NULL;
    char *token = NULL;
    char *lic = NULL;

    assert(db != NULL);
    assert(s != NULL);
    assert(params != NULL);

    /* see if the entire substring matches */
    if (check_license_abbrev(db, s)) {
        return true;
    }

    /* fall back on checking each individual token */
    tagtokens = tagcopy = strdup(s);

    while ((token = strsep(&tagtokens, " ")) != NULL) {
        if (!strcmp(token, "")) {
            continue;
        }

        if (!strcasecmp(token, "and") || !strcasecmp(token, "or")) {
            if (lic == NULL) {
                continue;
            }

            if (!check_license_abbrev(db, lic)) {
                result = false;

                xasprintf(&params->msg, _("Unapproved license in %s: %s"), nevra, lic);
                add_result_entry(rq, params);
                free(params->msg);
            } else {
                DEBUG_PRINT("APPROVED: |%s|\n", lic);
            }

            free(lic);
            lic = NULL;
        } else if (lic == NULL) {
            lic = strdup(token);
        } else {
            token_append(&lic, token);
        }
    }

    /* check final token */
    if (lic) {
        if (!check_license_abbrev(db, lic)) {
            result = false;

            xasprintf(&params->msg, _("Unapproved license in %s: %s"), nevra, lic);
            add_result_entry(rq, params);
            free(params->msg);
        } else {
            DEBUG_PRINT("APPROVED: |%s|\n", lic);
        }
    }

    /* cleanup */
    free(lic);
    free(tagcopy);

    return result;
}

/*
 * RPM License tags in the spec file permit parentheses to group licenses
 * together that need to be used together.  The License tag also permits
 * the use of boolean 'and' and 'or' keywords.  The only thing of note for
 * these expressions is that they do not permit negation since that does
 * not really make sense for the License tag.  If a license doesn't apply,
 * the RPM cannot ship that.
 *
 * The check involved here consists of multiple parts:
 * 1) Check to see that expressions in parentheses are balance.  For
 *    example, "(GPLv2+ and MIT) or LGPLv2+" is valid, but we want to
 *    make sure that "GPLv2+ and MIT) or (LGPLv2+" is invalid.
 * 2) Tokenize the license tag.
 * 3) Iterate over each token, skipping the 'and' and 'or' keywords, to
 *    match against the license database.
 * 4) The function returns true if all license tags are approved in the
 *    database.  Any single tag that is unapproved results in false.
 */
static bool is_valid_license(struct rpminspect *ri, struct result_params *params, const char *nevra, const char *tag)
{
    bool result = true;
    int balance = 0;
    size_t i = 0;
    char *tagtokens = NULL;
    char *tagcopy = NULL;
    char *token = NULL;
    results_t *rq = NULL;
    results_entry_t *entry = NULL;
    string_entry_t *sentry = NULL;
    struct json_object *db = NULL;

    assert(ri != NULL);
    assert(params != NULL);
    assert(nevra != NULL);
    assert(tag != NULL);

    /* Set up the result parameters */
    params->severity = RESULT_BAD;
    params->remedy = REMEDY_UNAPPROVED_LICENSE;

    /* check for matching parens */
    for (i = 0; i < strlen(tag); i++) {
        if (balance < 0) {
            return false;
        }

        if (tag[i] == '(') {
            balance++;
            continue;
        }

        if (tag[i] == ')') {
            balance--;
        }
    }

    if (balance != 0) {
        return false;
    }

    /* read in each approved license database */
    TAILQ_FOREACH(sentry, ri->licensedb, items) {
        db = read_licensedb(ri, sentry->data);

        if (db == NULL) {
            continue;
        }

        /* First try to match the entire string */
        if (check_license_abbrev(db, tag)) {
            return true;
        }

        /* tokenize the license tag and validate each license */
        tagtokens = tagcopy = strdup(tag);

        while ((token = strsep(&tagtokens, "()")) != NULL) {
            if (!strcmp(token, "")) {
                continue;
            }

            if (!is_valid_expression(db, token, nevra, params, &rq)) {
                result = false;
            }
        }

        free(tagcopy);
        json_object_put(db);
    }

    /*
     * license tag is approved if number of seen tags equals
     * number of valid tags
     */
    if (result) {
        free_results(rq);
        return true;
    } else {
        if (ri->results == NULL) {
            ri->results = init_results();
        }

        if (rq && !TAILQ_EMPTY(rq)) {
            /*
             * make sure to set the worst result based on queued
             * license inspection failures.
             */
            TAILQ_FOREACH(entry, rq, items) {
                if (entry->severity > ri->worst_result) {
                    ri->worst_result = entry->severity;
                }
            }

            /* append the queued license inspection failures */
            TAILQ_CONCAT(ri->results, rq, items);
        }

        return false;
    }

    return false;
}

/*
 * Called by inspect_license()
 */
static int check_peer_license(struct rpminspect *ri, struct result_params *params, const Header hdr)
{
    int ret = 0;
    bool valid = false;
    char *nevra = NULL;
    const char *license = NULL;

    assert(ri != NULL);
    assert(params != NULL);

    nevra = get_nevra(hdr);
    assert(nevra != NULL);
    license = headerGetString(hdr, RPMTAG_LICENSE);
    params->file = nevra;
    params->arch = get_rpm_header_arch(hdr);

    if (license == NULL) {
        xasprintf(&params->msg, _("Empty License Tag in %s"), nevra);
        params->severity = RESULT_BAD;
        params->remedy = REMEDY_LICENSE;
        params->verb = VERB_FAILED;
        params->noun = _("missing License tag in ${FILE}");
        add_result(ri, params);
        free(params->msg);
        ret = 1;
    } else {
        /* is the license tag valid or not */
        valid = is_valid_license(ri, params, nevra, license);

        if (valid) {
            xasprintf(&params->msg, _("Valid License Tag in %s: %s"), nevra, license);
            params->severity = RESULT_INFO;
            params->remedy = NULL;
            params->verb = VERB_OK;
            params->noun = NULL;
            params->file = NULL;
            params->arch = NULL;
            add_result(ri, params);
            free(params->msg);
            ret = 1;
        }

        /* does the license tag contain bad words? */
        if (has_bad_word(license, ri->badwords)) {
            xasprintf(&params->msg, _("License Tag contains unprofessional language in %s: %s"), nevra, license);
            params->severity = RESULT_BAD;
            params->remedy = REMEDY_LICENSE;
            params->verb = VERB_FAILED;
            params->noun = _("unprofessional language in License tag in ${FILE}");
            add_result(ri, params);
            free(params->msg);
            ret = 1;
        }
    }

    free(nevra);
    return ret;
}

/**
 * @brief Perform the 'license' inspection.
 *
 * Verify the string specified in the License tag of the RPM metadata
 * describes permissible software licenses as defined by the license
 * database. Also checks to see if the License tag contains any
 * unprofessional words as defined in the configuration file.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_license(struct rpminspect *ri)
{
    int good = 0;
    int seen = 0;
    bool result = false;
    rpmpeer_entry_t *peer = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    init_result_params(&params);
    params.header = NAME_LICENSE;
    params.waiverauth = NOT_WAIVABLE;

    if (ri->licensedb == NULL || TAILQ_EMPTY(ri->licensedb)) {
        xasprintf(&params.msg, _("Missing license database(s)."));
        params.severity = RESULT_BAD;
        params.remedy = REMEDY_LICENSEDB;
        params.verb = VERB_FAILED;
        params.noun = _("missing license database");
        params.file = NULL;
        params.arch = NULL;
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    /*
     * The license test just looks at the licenses on the after build
     * packages.  The before build is not used here.
     */

    /* Second check the binary peers */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are reported via INSPECT_EMPTYRPM */
        if (peer->after_rpm == NULL) {
            continue;
        }

        good += check_peer_license(ri, &params, peer->after_hdr);
        seen++;
    }

    if (good == seen) {
        init_result_params(&params);
        params.header = NAME_LICENSE;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
        result = true;
    }

    return result;
}
