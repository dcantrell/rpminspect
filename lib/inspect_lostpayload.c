/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect_lostpayload.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief 'lostpayload' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "rpminspect.h"

/**
 * @brief Perform the 'lostpayload' inspection.
 *
 * Check all binary RPMs in the before and after builds for any
 * packages that lose their payloads from the before build to the
 * after build.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_lostpayload(struct rpminspect *ri)
{
    bool good = true;
    bool messaged = false;
    rpmpeer_entry_t *peer = NULL;
    char *an = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    init_result_params(&params);
    params.header = NAME_LOSTPAYLOAD;

    /*
     * The lostpayload inspection looks for any packages missing payloads.
     * These could be packages that lost their payloads from the before
     * build to the after build.
     */

    /* Check the binary peers */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /*
         * Subpackages may disappear in subsequent builds.  Sometimes this
         * is intentional, sometimes not.
         */
        if (peer->before_rpm != NULL && peer->after_rpm == NULL) {
            xasprintf(&params.msg, _("Existing subpackage %s is now missing"), headerGetString(peer->before_hdr, RPMTAG_NAME));
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.verb = VERB_FAILED;
            params.noun = _("missing subpackage ${FILE} on ${ARCH}");
            params.file = headerGetString(peer->before_hdr, RPMTAG_NAME);
            params.arch = get_rpm_header_arch(peer->before_hdr);
            params.remedy = REMEDY_LOSTPAYLOAD;
            add_result(ri, &params);
            free(params.msg);
            good = false;
            messaged = true;
            continue;
        }

        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            if (peer->before_files == NULL || TAILQ_EMPTY(peer->before_files)) {
                xasprintf(&params.msg, _("Package %s continues to be empty (no payloads)"), basename(peer->after_rpm));
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.verb = VERB_OK;
                params.noun = NULL;
                params.file = NULL;
                params.arch = NULL;
                params.remedy = REMEDY_LOSTPAYLOAD;
                add_result(ri, &params);
                free(params.msg);
            } else {
                an = basename(peer->after_rpm);
                xasprintf(&params.msg, _("Package %s became empty (no payloads)"), an);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.verb = VERB_FAILED;
                params.noun = _("subpackage ${FILE} on ${ARCH} now has empty payload");
                params.file = an;
                params.arch = get_rpm_header_arch(peer->after_hdr);
                params.remedy = REMEDY_LOSTPAYLOAD;
                add_result(ri, &params);
                free(params.msg);
                good = false;
            }

            messaged = true;
        }
    }

    if (!messaged) {
        init_result_params(&params);
        params.header = NAME_LOSTPAYLOAD;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return good;
}
