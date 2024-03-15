/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect_emptyrpm.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief 'emptyrpm' inspection
 * @copyright LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "rpminspect.h"

static bool payload_only_ghosts(Header h)
{
    bool onlyghosts = true;
    rpmtd td = NULL;
    rpmFlags tdflags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    rpmfileAttrs fileflags = 0;

    assert(h != NULL);

    /* check to see if the package only contains %ghost entries */
    td = rpmtdNew();

    if (headerGet(h, RPMTAG_FILEFLAGS, td, tdflags)) {
        while ((rpmtdNext(td) != -1)) {
            fileflags = *(rpmtdGetUint32(td));

            if (!(fileflags & RPMFILE_GHOST)) {
                /* we found a non-ghost, something is wrong */
                onlyghosts = false;
                break;
            }
        }
    }

    rpmtdFreeData(td);
    rpmtdFree(td);

    return onlyghosts;
}

/**
 * @brief Perform the 'emptyrpm' inspection.
 *
 * Report any packages that appear in the build that contain new
 * payload.  When comparing two builds, only report new empty payload
 * packages.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_emptyrpm(struct rpminspect *ri)
{
    bool good = true;
    bool reported = false;
    rpmpeer_entry_t *peer = NULL;
    const char *name = NULL;
    char *bn = NULL;
    char *an = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    init_result_params(&params);
    params.header = NAME_EMPTYRPM;

    /*
     * The emptyrpm inspection looks for any packages missing
     * payloads.  These are packages (or new packages when comparing)
     * that lack any payloads.
     */

    /* Check the binary peers */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* only check built RPMs, not the source RPM */
        if (headerIsSource(peer->after_hdr)) {
            continue;
        }

        if ((peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) && peer->before_rpm == NULL) {
            name = headerGetString(peer->after_hdr, RPMTAG_NAME);
            assert(name != NULL);
            bn = basename(peer->after_rpm);
            assert(bn != NULL);

            if (list_contains(ri->expected_empty_rpms, name)) {
                xasprintf(&params.msg, _("New package %s is empty (no payloads); this is expected per the rpminspect configuration"), bn);
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.verb = VERB_OK;
                params.noun = NULL;
                params.file = NULL;
                params.arch = NULL;
                params.remedy = NULL;
                reported = true;
            } else if (payload_only_ghosts(peer->after_hdr)) {
                xasprintf(&params.msg, _("New package %s is empty (no payloads); this is expected because the package only contains %%ghost entries"), bn);
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.verb = VERB_OK;
                params.noun = NULL;
                params.file = NULL;
                params.arch = NULL;
                params.remedy = NULL;
                reported = true;
            } else {
                an = basename(peer->after_rpm);
                xasprintf(&params.msg, _("New package %s is empty (no payloads)"), an);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.verb = VERB_FAILED;
                params.noun = _("${FILE} has empty payload");
                params.file = an;
                params.arch = get_rpm_header_arch(peer->after_hdr);
                params.remedy = get_remedy(REMEDY_EMPTYRPM);
                good = false;
                reported = true;
            }

            add_result(ri, &params);
            free(params.msg);
        }
    }

    if (good & !reported) {
        init_result_params(&params);
        params.header = NAME_EMPTYRPM;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return good;
}
