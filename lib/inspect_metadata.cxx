/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "queue.h"
#include "rpminspect.h"

/*
 * Validate the metadata tags in the RPM headers.
 */
static bool valid_peers(struct rpminspect *ri, const Header before_hdr, const Header after_hdr)
{
    bool ret = true;
    bool valid_subdomain = false;
    bool rebase = false;
    string_entry_t *subdomain = NULL;
    const char *after_vendor = NULL;
    const char *after_buildhost = NULL;
    const char *after_summary = NULL;
    const char *after_description = NULL;
    const char *after_name = NULL;
    char *after_nevra = NULL;
    struct result_params params;

    assert(ri != NULL);

    after_nevra = get_nevra(after_hdr);
    rebase = is_rebase(ri);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_METADATA;

    after_vendor = headerGetString(after_hdr, RPMTAG_VENDOR);
    if (ri->vendor == NULL) {
        xasprintf(&params.msg, _("Vendor not set in the rpminspect configuration, ignoring Package Vendor \"%s\" in %s"), after_vendor, after_nevra);
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_OK;
        params.noun = NULL;
        params.file = NULL;
        params.arch = NULL;
        params.remedy = get_remedy(REMEDY_VENDOR);
        add_result(ri, &params);
        free(params.msg);
    } else if (after_vendor && strcmp(after_vendor, ri->vendor)) {
        xasprintf(&params.msg, _("Package Vendor \"%s\" is not \"%s\" in %s"), after_vendor, ri->vendor, after_nevra);
        params.severity = RESULT_BAD;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_FAILED;
        params.noun = _("invalid vendor ${FILE} on ${ARCH}");
        params.file = after_vendor;
        params.arch = get_rpm_header_arch(after_hdr);
        params.remedy = get_remedy(REMEDY_VENDOR);
        add_result(ri, &params);
        free(params.msg);
        ret = false;
    }

    after_buildhost = headerGetString(after_hdr, RPMTAG_BUILDHOST);
    if (after_buildhost && ri->buildhost_subdomain != NULL) {
        valid_subdomain = false;

        TAILQ_FOREACH(subdomain, ri->buildhost_subdomain, items) {
            if (strsuffix(after_buildhost, subdomain->data)) {
                valid_subdomain = true;
                break;
            }
        }

        if (!valid_subdomain) {
            xasprintf(&params.msg, _("Package Build Host \"%s\" is not within an expected build host subdomain in %s"), after_buildhost, after_nevra);
            params.severity = RESULT_BAD;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_FAILED;
            params.noun = _("invalid build host ${FILE} on ${ARCH}");
            params.file = after_buildhost;
            params.arch = get_rpm_header_arch(after_hdr);
            params.remedy = get_remedy(REMEDY_BUILDHOST);
            add_result(ri, &params);
            free(params.msg);
            ret = false;
        }
    }

    after_summary = headerGetString(after_hdr, RPMTAG_SUMMARY);
    if (after_summary && has_bad_word(after_summary, ri->badwords)) {
        xasprintf(&params.msg, _("Package Summary contains unprofessional language in %s"), after_nevra);
        xasprintf(&params.details, _("Summary: %s"), after_summary);
        params.severity = RESULT_BAD;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_FAILED;
        params.noun = _("Summary contains unprofessional words on ${ARCH}");
        params.file = NULL;
        params.arch = get_rpm_header_arch(after_hdr);
        params.remedy = get_remedy(REMEDY_BADWORDS);
        add_result(ri, &params);
        free(params.msg);
        free(params.details);
        ret = false;
    }

    after_description = headerGetString(after_hdr, RPMTAG_DESCRIPTION);
    if (after_description && has_bad_word(after_description, ri->badwords)) {
        xasprintf(&params.msg, _("Package Description contains unprofessional language in %s:"), after_nevra);
        xasprintf(&params.details, "%s", after_description);
        params.severity = RESULT_BAD;
        params.waiverauth = NOT_WAIVABLE;
        params.verb = VERB_FAILED;
        params.noun = _("Description contains unprofessional words on ${ARCH}");
        params.file = NULL;
        params.arch = get_rpm_header_arch(after_hdr);
        params.remedy = get_remedy(REMEDY_BADWORDS);
        add_result(ri, &params);
        free(params.msg);
        free(params.details);
        ret = false;
    }

    free(after_nevra);

    if (before_hdr != NULL) {
        const char *before_vendor = headerGetString(before_hdr, RPMTAG_VENDOR);
        const char *before_summary = headerGetString(before_hdr, RPMTAG_SUMMARY);
        const char *before_description = headerGetString(before_hdr, RPMTAG_DESCRIPTION);
        after_name = headerGetString(after_hdr, RPMTAG_NAME);
        params.msg = NULL;

        if (before_vendor == NULL && after_vendor) {
            xasprintf(&params.msg, _("Gained Package Vendor \"%s\" in %s"), after_vendor, after_name);
        } else if (before_vendor && after_vendor == NULL) {
            xasprintf(&params.msg, _("Lost Package Vendor \"%s\" in %s"), before_vendor, after_name);
        } else if (before_vendor && after_vendor && strcmp(before_vendor, after_vendor)) {
            xasprintf(&params.msg, _("Package Vendor changed from \"%s\" to \"%s\" in %s"), before_vendor, after_vendor, after_name);
        }

        if (params.msg) {
            if (rebase) {
                params.severity = RESULT_INFO;
            } else {
                params.severity = RESULT_VERIFY;
                ret = false;
            }

            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.remedy = NULL;
            add_result(ri, &params);
            free(params.msg);
        }

        if (before_summary && after_summary && strcmp(before_summary, after_summary)) {
            xasprintf(&params.msg, _("Package Summary change from \"%s\" to \"%s\" in %s"), before_summary, after_summary, after_name);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
            params.noun = NULL;
            params.file = NULL;
            params.arch = NULL;
            params.remedy = NULL;
            add_result(ri, &params);
            free(params.msg);
        }

        if (before_description && after_description && strcmp(before_description, after_description)) {
            xasprintf(&params.msg, _("Package Description changed in %s"), after_name);
            xasprintf(&params.details, _("from:\n\n%s\n\nto:\n\n%s"), before_description, after_description);
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;
            params.verb = VERB_OK;
            params.noun = NULL;
            params.file = NULL;
            params.arch = NULL;
            params.remedy = NULL;
            add_result(ri, &params);
            free(params.msg);
            free(params.details);
       }
    }

    return ret;
}

/*
 * Main driver for the 'metadata' inspection.
 */
bool inspect_metadata(struct rpminspect *ri)
{
    bool good = true;
    rpmpeer_entry_t *peer = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * The metadata inspection looks at the RPM header information and
     * performs a few checks:
     *     - Make sure the Vendor is correct
     *     - Make sure the Buildhost is in the expected subdomain
     *     - Check for bad words in the Summary and Description
     *     - In cases of before and after builds, check for differences
     *       between the above RPM header values and report them.
     */

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_rpm == NULL) {
            continue;
        }

        if (!valid_peers(ri, peer->before_hdr, peer->after_hdr)) {
            good = false;
        }
    }

    if (good) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_METADATA;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return good;
}
