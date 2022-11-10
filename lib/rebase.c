/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file rebase.c
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2020
 * @brief Debugging utility functions.
 * @copyright LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include "rpminspect.h"

/**
 * @brief Determine if the program is inspecting a rebase build.  That
 * is, the package name of the before and after build match but the
 * versions are different.  If there is no before package, then this
 * function returns true.
 *
 * The value is cached in ri.  By default the value is 0, which
 * indicates it is not set.  This function will set it and a value < 0
 * means false and a value > 0 means true.
 *
 * @param ri The struct rpminspect structure for the program.
 * @return True if the packages are a rebase, false otherwise
 */
bool is_rebase(struct rpminspect *ri)
{
    rpmpeer_entry_t *peer = NULL;
    const char *bn = NULL;
    const char *an = NULL;
    const char *bv = NULL;
    const char *av = NULL;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /* disabled with the -n command line option */
    if (!ri->rebase_detection) {
        ri->rebase_build = -1;
        return false;
    }

    /* run the rebase detection */
    if (ri->rebase_build == 0) {
        ri->rebase_build = -1;

        TAILQ_FOREACH(peer, ri->peers, items) {
            /* take the first package where we have each peer */
            if (peer->before_hdr && peer->after_hdr) {
                break;
            }
        }

        if (peer) {
            if (peer->before_hdr == NULL) {
                /* no previous build cannot be a rebased build */
                ri->rebase_build = -1;
            } else {
                bn = headerGetString(peer->before_hdr, RPMTAG_NAME);
                an = headerGetString(peer->after_hdr, RPMTAG_NAME);
                bv = headerGetString(peer->before_hdr, RPMTAG_VERSION);
                av = headerGetString(peer->after_hdr, RPMTAG_VERSION);

                if (!strcmp(bn, an) && strcmp(bv, av)) {
                    ri->rebase_build = 1;
                }
            }
        }
    }

    /* if the package name is on the rebaseable list, it's valid */
    if (ri->rebase_build == -1 && init_rebaseable(ri) && list_contains(ri->rebaseable, an) && ((bn && !strcmp(bn, an)) || bn == NULL)) {
        return true;
    }

    /* is it a rebase or not or did an error occur? */
    if (ri->rebase_build == -1) {
        return false;
    } else if (ri->rebase_build == 1) {
        return true;
    } else {
        /* should not reach this */
        err(RI_PROGRAM_ERROR, "is_rebase");
    }
}
