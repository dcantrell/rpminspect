/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <assert.h>
#include <err.h>

#include "rpminspect.h"

/*
 * Initialize a new rpmpeer_t list.
 */
rpmpeer_t *init_peers(void)
{
    rpmpeer_t *peers = NULL;

    peers = calloc(1, sizeof(*(peers)));
    assert(peers != NULL);
    TAILQ_INIT(peers);
    return peers;
}

/*
 * Free memory associated with an rpmpeer_t list.
 */
void free_peers(rpmpeer_t *peers)
{
    rpmpeer_entry_t *entry = NULL;

    if (peers == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(peers)) {
        entry = TAILQ_FIRST(peers);
        TAILQ_REMOVE(peers, entry, items);
        free(entry->before_rpm);
        free(entry->after_rpm);
        free(entry->before_root);
        free(entry->after_root);
        free_files(entry->before_files);
        free_files(entry->after_files);
        free_deprules(entry->before_deprules);
        free_deprules(entry->after_deprules);
        free(entry);
    }

    free(peers);

    return;
}

/*
 * Add the specified package as a peer in the list of packages.
 */
void add_peer(rpmpeer_t **peers, int whichbuild, bool fetch_only, const char *pkg, Header hdr)
{
    rpmpeer_entry_t *peer = NULL;
    bool found = false;
    const char *newname = NULL;
    const char *newarch = NULL;
    const char *existingname = NULL;
    const char *existingarch = NULL;
    bool existingsrc = false;
    bool newsrc = false;

    assert(peers != NULL);
    assert(pkg != NULL);
    assert(hdr != NULL);

    if (*peers == NULL) {
        *peers = init_peers();
    }

    /* Get the package or subpackage name and arch */
    newname = headerGetString(hdr, RPMTAG_NAME);
    newarch = get_rpm_header_arch(hdr);
    newsrc = headerIsSource(hdr);

    /* If we don't have this peer, try to add it */
    TAILQ_FOREACH(peer, *peers, items) {
        if (whichbuild == BEFORE_BUILD && peer->after_rpm != NULL) {
            existingname = headerGetString(peer->after_hdr, RPMTAG_NAME);
            existingarch = get_rpm_header_arch(peer->after_hdr);
            existingsrc = headerIsSource(peer->after_hdr);
        } else if (whichbuild == AFTER_BUILD && peer->before_rpm != NULL) {
            existingname = headerGetString(peer->before_hdr, RPMTAG_NAME);
            existingarch = get_rpm_header_arch(peer->before_hdr);
            existingsrc = headerIsSource(peer->before_hdr);
        }

        if (existingname == NULL) {
            continue;
        }

        if ((existingsrc && newsrc && !strcmp(existingname, newname)) ||
            (!existingsrc && !newsrc && !strcmp(existingname, newname) && !strcmp(existingarch, newarch))) {
            /* found the existing peer */
            found = true;
            break;
        }
    }

    /* Add the peer if it doesn't already exist, otherwise add it */
    if (!found) {
        peer = calloc(1, sizeof(*peer));

        if (peer == NULL) {
            warn("calloc");
            return;
        }
    }

    if (whichbuild == BEFORE_BUILD) {
        peer->before_hdr = hdr;
        peer->before_rpm = strdup(pkg);
        peer->before_files = NULL;
        peer->before_root = NULL;
        peer->before_unpacked_size = headerGetNumber(peer->before_hdr, RPMTAG_SIZE);

        if (fetch_only) {
            peer->before_deprules = NULL;
        } else {
            peer->before_deprules = gather_deprules(hdr);
        }
    } else if (whichbuild == AFTER_BUILD) {
        peer->after_hdr = hdr;
        peer->after_rpm = strdup(pkg);
        peer->after_files = NULL;
        peer->after_root = NULL;
        peer->after_unpacked_size = headerGetNumber(peer->after_hdr, RPMTAG_SIZE);

        if (fetch_only) {
            peer->after_deprules = NULL;
        } else {
            peer->after_deprules = gather_deprules(hdr);
        }
    }

    if (!found) {
        TAILQ_INSERT_TAIL(*peers, peer, items);
    }

    return;
}

int extract_peers(struct rpminspect *ri, bool fetchonly)
{
    unsigned long avail = 0;
    char *availh = NULL;
    char *needh = NULL;
    rpmpeer_entry_t *peer = NULL;

    if (fetchonly) {
        return RI_SUCCESS;
    }

    assert(ri != NULL);
    assert(ri->peers != NULL);
    assert(ri->workdir != NULL);

    /* compute total unpacked size required and see if there's space */
    TAILQ_FOREACH(peer, ri->peers, items) {
        ri->unpacked_size += peer->before_unpacked_size;
        ri->unpacked_size += peer->after_unpacked_size;
    }

    avail = get_available_space(ri->workdir);

    if (avail < ri->unpacked_size) {
        availh = human_size(avail);
        needh = human_size(ri->unpacked_size);

        fprintf(stderr, _("There is not enough available space to unpack all of the RPMs.\n"));
        fprintf(stderr, _("    Need %s in %s, have %s.\n"), needh, ri->workdir, availh);
        fprintf(stderr, _("See the `-w' option for specifying an alternate working directory.\n"));
        fflush(stderr);
        free(needh);
        free(availh);
        return RI_INSUFFICIENT_SPACE;
    }

    /* unpack all RPMs */
    TAILQ_FOREACH(peer, ri->peers, items) {
        /* extract the before peer */
        if (peer->before_hdr && peer->before_rpm) {
            peer->before_files = extract_rpm(ri, peer->before_rpm, peer->before_hdr, BEFORE_SUBDIR, &peer->before_root);
        }

        /* extract the after peer */
        if (peer->after_hdr && peer->after_rpm) {
            peer->after_files = extract_rpm(ri, peer->after_rpm, peer->after_hdr, AFTER_SUBDIR, &peer->after_root);
        }

        /* match up file peers between builds */
        if (peer->before_files && peer->after_files) {
            find_file_peers(peer->before_files, peer->after_files);
        }
    }

    return RI_SUCCESS;
}
