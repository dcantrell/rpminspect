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
#include "internal/callbacks.h"
#include "parser.h"
#include "rpminspect.h"

/* Globals */
static const char *srpm = NULL;
static int nspdx = 0;
string_list_t *booleans = NULL;

/* Local helper functions */
static parser_plugin *read_licensedb(struct rpminspect *ri, const char *db, parser_context **context_out)
{
    char *actualdb = NULL;
    parser_plugin *p = &json_parser;
    parser_context *context = NULL;

    assert(ri != NULL);
    assert(db != NULL);

    /* build path to license db if necessary */
    if (db && db[0] == '/') {
        actualdb = strdup(db);
        assert(actualdb != NULL);
    } else {
        xasprintf(&actualdb, "%s/%s/%s", ri->vendor_data_dir, LICENSES_DIR, db);
    }

    if (p->parse_file(&context, actualdb)) {
        warnx(_("*** parse error in license database %s"), db);
        free(actualdb);
        return NULL;
    }

    free(actualdb);
    *context_out = context;
    return p;
}

/* lambda; checks against an entry in the license database. */
static bool lic_cb(const char *license_name, void *cb_data)
{
    lic_cb_data *data = cb_data;
    const char *lic = data->lic;
    parser_plugin *p = data->p;
    parser_context *db = data->db;
    parser_context *cont = NULL;
    json_object *block = NULL;
    string_list_t *slist = NULL;
    string_entry_t *entry = NULL;
    string_list_t *fedora_abbrev = NULL;
    string_list_t *fedora_name = NULL;
    string_list_t *exceptions = NULL;
    char *spdx_abbrev = NULL;
    bool approved = false;
    char *t = NULL;

    if (data->valid) {
        /* Already decided; no need to check further. */
        return false;
    }

    if (strlen(license_name) == 0) {
        return false;
    }

    /* get the license db block for this license name */
    if (json_object_object_get_ex((json_object *) db, license_name, &block) == 0) {
        return false;
    }

    /* try to read new license data format */
    if (json_object_is_type(block, json_type_object)) {
        cont = (parser_context *) block;

        array(p, cont, "fedora", "legacy-abbreviation", &fedora_abbrev);
        array(p, cont, "fedora", "legacy-name", &fedora_name);
        spdx_abbrev = p->getstr(cont, "license", "expression");

        approved = false;
        array(p, cont, "license", "status", &slist);
        array(p, cont, "license", "packages_with_exceptions", &exceptions);

        if (list_len(slist)) {
            TAILQ_FOREACH(entry, slist, items) {
                if (!strcmp(entry->data, "allowed") || strprefix(entry->data, "allowed-") || (!strcmp(entry->data, "not-allowed") && list_contains(exceptions, srpm))) {
                    approved = true;
                    break;
                }
           }
       }

       list_free(slist, free);
       list_free(exceptions, free);
    }

    /* new format failed, fall back on previous format */
    if (spdx_abbrev == NULL) {
        list_free(fedora_abbrev, free);
        list_free(fedora_name, free);
        approved = false;

        /* the new API format failed, fall back on the legacy format */
        t = p->getstr(db, license_name, "fedora_abbrev");
        fedora_abbrev = list_add(fedora_abbrev, t);
        free(t);

        t = p->getstr(db, license_name, "fedora_name");
        fedora_name = list_add(fedora_name, t);
        free(t);

        spdx_abbrev = p->getstr(db, license_name, "spdx_abbrev");

        t = p->getstr(db, license_name, "approved");

        if (t && (!strcasecmp(t, "yes") || !strcasecmp(t, "true"))) {
            approved = true;
        } else {
            approved = false;
        }

        free(t);
    }

    /* Handle "commented out" fields - not proper JSON, but I'm not a cop. */
    fedora_abbrev = list_trim(fedora_abbrev, "#");
    fedora_name = list_trim(fedora_name, NULL);

    if (spdx_abbrev && *spdx_abbrev == '#') {
        free(spdx_abbrev);
        spdx_abbrev = NULL;
    }

    /* No full tag match and no abbreviations; license entry invalid. */
    if (spdx_abbrev == NULL && fedora_abbrev == NULL && fedora_name == NULL) {
        goto done;
    }

    /*
     * If the entire license string is approved, that is valid.
     * If we hit 'fedora_abbrev', that is valid.
     * If we hit 'spdx_abbrev' and approved is true, that is valid.
     * NOTE: we only match the first hit in the license database
     */
    if (!approved) {
        /* invalid - do nothing */
        goto done;
    } else if (spdx_abbrev && !strcasecmp(lic, spdx_abbrev)) {
        /* SPDX identifier matched */
        data->valid = true;
        nspdx++;
    } else if (list_contains(fedora_abbrev, lic) || (list_len(fedora_abbrev) == 0 && spdx_abbrev == NULL && list_contains(fedora_name, lic))) {
        /* Old Fedora abbreviation matches -or- there are no Fedora abbreviations but a Fedora name matches */
        data->valid = true;
    }

done:
    list_free(fedora_abbrev, free);
    list_free(fedora_name, free);
    free(spdx_abbrev);

    return false;
}

/*
 * Called by is_valid_license() to check each short license token.  It
 * will also try to do a whole match on the license tag string.
 */
static bool check_license_abbrev(parser_plugin *p, parser_context *db, const char *lic)
{
    lic_cb_data data = { p, db, lic, false };

    if (p->keymap(db, NULL, NULL, lic_cb, &data)) {
        warnx(_("*** problem checking license database"));
        return false;
    }

    return data.valid;
}

static void token_add(string_map_t **tags, const char *token)
{
    string_map_t *tag_entry = NULL;

    assert(token != NULL);

    tag_entry = calloc(1, sizeof(*tag_entry));
    assert(tag_entry != NULL);
    tag_entry->key = strdup(token);
    assert(tag_entry->key != NULL);
    tag_entry->value = NULL;
    HASH_ADD_KEYPTR(hh, *tags, tag_entry->key, strlen(tag_entry->key), tag_entry);

    return;
}

/*
 * Split up the license expression in to tokens.  This can be an empty
 * list on return.
 */
static string_map_t *tokenize_license_tag(const char *license)
{
    char *tagtokens = NULL;
    char *tagcopy = NULL;
    char *token = NULL;
    char *lic = NULL;
    string_map_t *tags = NULL;

    assert(license != NULL);

    tagtokens = tagcopy = strdup(license);
    assert(tagcopy != NULL);

    while ((token = strsep(&tagtokens, " ()")) != NULL) {
        if (!strcmp(token, "")) {
            continue;
        }

        if (!strcasecmp(token, "and") || !strcasecmp(token, "or")) {
            booleans = list_add(booleans, token);

            if (lic == NULL) {
                continue;
            }

            /* add this tag to the hash table */
            token_add(&tags, lic);

            /* clear the local tag to start over */
            free(lic);
            lic = NULL;
        } else if (lic == NULL) {
            lic = strdup(token);
        } else {
            lic = strappend(lic, " ", token, NULL);
        }
    }

    if (lic) {
        /* add this last tag to the hash table */
        token_add(&tags, lic);
    }

    /* cleanup */
    free(lic);
    free(tagcopy);

    return tags;
}

/* Free the license tags hash */
static void free_tags(string_map_t *tags)
{
    string_map_t *tagtoken = NULL;
    string_map_t *tmp_tagtoken = NULL;

    if (tags == NULL) {
        return;
    }

    HASH_ITER(hh, tags, tagtoken, tmp_tagtoken) {
        tagtoken->value = NULL;
    }

    free_string_map(tags);
    return;
}

/*
 * This is very unique to how license tags have historically been used
 * in Fedora and Red Hat.  Sometimes a compound expression is allowed
 * but as individual tokens not all of them are allowed.  An example
 * is Perl packages using "GPL+ or Artistic" as their license tag for
 * a long time.  GPL+ was allowed but Artistic was not.  However the
 * compound expression "GPL+ or Artistic" was allowed per the license
 * database.  This is legacy stuff, but we still need to handle it
 * until all packages have moved over to SPDX expressions exclusively.
 *
 * The two in-use instances of these types of legacy compound
 * expression are either as the entire license tag or as an expression
 * within parens.  We do not handle nested parens.  This function
 * builds a list of compound expressions in parens and returns it to
 * the caller.  The caller is responsible for freeing this list.
 */
static string_list_t *get_paren_expressions(const char *license)
{
    char *start = NULL;
    char *end = NULL;
    char *copy = NULL;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    assert(license != NULL);

    start = copy = strdup(license);
    assert(start != NULL);

    while ((start = strchr(start, '(')) != NULL) {
        while (*start == '(') {
            start++;
        }

        end = strchr(start, ')');

        if ((end - start) > 0) {
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);

            entry->data = calloc(1, end - start + 1);
            assert(entry->data != NULL);

            entry->data = strncpy(entry->data, start, end - start);

            if (list == NULL) {
                list = calloc(1, sizeof(*list));
                assert(list != NULL);
                TAILQ_INIT(list);
            }

            TAILQ_INSERT_TAIL(list, entry, items);
        }

        start = end;
    }

    free(copy);
    return list;
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
 * 1) Check to see that expressions in parentheses are balanced.  For
 *    example, "(GPLv2+ and MIT) or LGPLv2+" is valid, but we want to
 *    make sure that "GPLv2+ and MIT) or (LGPLv2+" is invalid.
 * 2) Tokenize the license tag.
 * 3) Iterate over each token, skipping the 'and' and 'or' keywords, to
 *    match against the license database.
 * 4) The function returns true if all license tags are approved in the
 *    database.  Any single tag that is unapproved results in false.
 */
static bool is_valid_license(struct rpminspect *ri, struct result_params *params, const char *nevra, const char *license)
{
    bool r = true;
    int balance = 0;
    size_t i = 0;
    char *tmp = NULL;
    char *wlicense = NULL;
    char *nlicense = NULL;
    string_map_t *tags = NULL;
    string_map_t *tagtoken = NULL;
    string_map_t *tmp_tagtoken = NULL;
    string_entry_t *entry = NULL;
    string_list_t *parenexps = NULL;
    string_entry_t *pentry = NULL;
    parser_plugin *p = NULL;
    parser_context *db = NULL;

    assert(ri != NULL);
    assert(params != NULL);
    assert(nevra != NULL);
    assert(license != NULL);

    /* Set up the result parameters */
    params->severity = RESULT_BAD;
    params->remedy = get_remedy(REMEDY_UNAPPROVED_LICENSE);

    /* check for matching parens */
    for (i = 0; i < strlen(license); i++) {
        if (balance < 0) {
            return false;
        }

        if (license[i] == '(') {
            balance++;
            continue;
        }

        if (license[i] == ')') {
            balance--;
        }
    }

    if (balance != 0) {
        return false;
    }

    wlicense = strdup(license);
    assert(wlicense != NULL);

    /*
     * loop over each license database and try to validate all license
     * tags, but there are 3 distinct phases because everything has to
     * be complicated
     */

    /*
     * first, loop over each db trying to match the entire tag.  this
     * is the common case.
     */
    TAILQ_FOREACH(entry, ri->licensedb, items) {
        /* read in this license database */
        p = read_licensedb(ri, entry->data, &db);

        if (p == NULL) {
            continue;
        }

        /* first, try to match the entire string */
        if (check_license_abbrev(p, db, wlicense)) {
            p->fini(db);
            free(wlicense);
            return true;
        }

        /* close this db */
        p->fini(db);
    }

    /*
     * second, try to match license substring tokens in parens (this
     * is because of past bad policy decisions).  if any match, remove
     * them from the whole license tag.  the reason for this is we
     * only want the third phase to deal with whatever is leftoveer
     * from this phase.
     */
    TAILQ_FOREACH(entry, ri->licensedb, items) {
        /* read in this license database */
        p = read_licensedb(ri, entry->data, &db);

        if (p == NULL) {
            continue;
        }

        parenexps = get_paren_expressions(wlicense);

        if (parenexps && !TAILQ_EMPTY(parenexps)) {
            TAILQ_FOREACH(pentry, parenexps, items) {
                if (check_license_abbrev(p, db, pentry->data)) {
                    xasprintf(&tmp, "(%s)", pentry->data);
                    assert(tmp != NULL);
                    nlicense = strreplace(wlicense, tmp, NULL);
                    free(tmp);
                    free(wlicense);
                    wlicense = nlicense;
                }
            }

            list_free(parenexps, free);
        }

        /* close this db */
        p->fini(db);
    }

    /*
     * third, check each remaining token not caught in the second
     * step.  this is individual tag checking for whole compound
     * expressions.
     */
    tags = tokenize_license_tag(wlicense);

    if (tags) {
        TAILQ_FOREACH(entry, ri->licensedb, items) {
            /* read in this license database */
            p = read_licensedb(ri, entry->data, &db);

            if (p == NULL) {
                continue;
            }

            HASH_ITER(hh, tags, tagtoken, tmp_tagtoken) {
                if (tagtoken->value == tagtoken->key) {
                    /* already validated */
                    continue;
                }

                if (check_license_abbrev(p, db, tagtoken->key)) {
                    /* set the value to non-NULL so we know it passed (DO NOT FREE) */
                    tagtoken->value = tagtoken->key;
                }
            }

            /* close this db */
            p->fini(db);
        }
    }

    /* report unapproved license tag tokens */
    if (tags) {
        HASH_ITER(hh, tags, tagtoken, tmp_tagtoken) {
            if (tagtoken->value == NULL) {
                r = false;

                if (ri->results == NULL) {
                    ri->results = init_results();
                }

                xasprintf(&params->msg, _("Unapproved license in %s: %s"), nevra, tagtoken->key);
                add_result(ri, params);
                free(params->msg);

                /*
                 * make sure to set the worst result based on queued
                 * license inspection failures.
                 */
                if (params->severity > ri->worst_result) {
                    ri->worst_result = params->severity;
                }
            }
        }
    }

    free_tags(tags);
    free(wlicense);

    /* for SPDX tags found, ensure booleans are all uppercase */
    if (nspdx > 0 && (booleans && !TAILQ_EMPTY(booleans))) {
        TAILQ_FOREACH(entry, booleans, items) {
            if ((!strcasecmp(entry->data, "AND") && strcmp(entry->data, "AND"))
                || (!strcasecmp(entry->data, "OR") && strcmp(entry->data, "OR"))) {
                r = false;

                params->severity = RESULT_BAD;
                params->remedy = get_remedy(REMEDY_INVALID_BOOLEAN);
                xasprintf(&params->msg, _("SPDX license expressions in use, but an invalid boolean was found: %s; when using SPDX expression the booleans must be in all caps."), entry->data);
                add_result(ri, params);
                free(params->msg);
            }
        }
    }

    return r;
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
        params->remedy = get_remedy(REMEDY_LICENSE);
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
            params->remedy = get_remedy(REMEDY_LICENSE);
            params->verb = VERB_FAILED;
            params->noun = _("unprofessional language in License tag in ${FILE}");
            add_result(ri, params);
            free(params->msg);
            ret = 1;
        }
    }

    free(nevra);
    list_free(booleans, free);
    booleans = NULL;
    nspdx = 0;
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
        params.remedy = get_remedy(REMEDY_LICENSEDB);
        params.verb = VERB_FAILED;
        params.noun = _("missing license database");
        params.file = NULL;
        params.arch = NULL;
        add_result(ri, &params);
        free(params.msg);
        return false;
    }

    /* Find the SRPM and get the package name */
    TAILQ_FOREACH(peer, ri->peers, items) {
        if (peer->after_rpm == NULL) {
            continue;
        }

        if (headerIsSource(peer->after_hdr)) {
            srpm = headerGetString(peer->after_hdr, RPMTAG_NAME);
            assert(srpm != NULL);
            break;
        } else {
        }
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
