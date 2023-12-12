/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "rpminspect.h"

static bool reported = false;

static void _check_helper(struct rpminspect *ri, rpmpeer_entry_t *peer, const char *name, const char *arch, rpmTagVal t, rpmTagVal p, rpmTagVal f)
{
    struct rpmtd_s scripts;
    struct rpmtd_s progs;
    struct rpmtd_s flags;
    headerGetFlags hgflags = HEADERGET_MINMEM;

    assert(ri != NULL);
    assert(peer != NULL);
    assert(name != NULL);
    assert(arch != NULL);

    /* before package */
    headerGet(peer->before_hdr, t, &scripts, hgflags);
    headerGet(peer->before_hdr, p, &progs, hgflags);
    headerGet(peer->before_hdr, f, &flags, hgflags);

    return;
}

static void _check_normaltrigger(struct rpminspect *ri, rpmpeer_entry_t *peer, const char *name, const char *arch)
{
    assert(ri != NULL);
    assert(peer != NULL);
    assert(name != NULL);
    assert(arch != NULL);

    _check_helper(ri, peer, name, arch, RPMTAG_TRIGGERSCRIPTS, RPMTAG_TRIGGERSCRIPTPROG, RPMTAG_TRIGGERSCRIPTFLAGS);

    return;
}

static void triggers_driver(struct rpminspect *ri, rpmpeer_entry_t *peer)
{
    const char *name = NULL;
    const char *arch = NULL;

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

    name = headerGetString(peer->after_hdr, RPMTAG_NAME);
    arch = get_rpm_header_arch(peer->after_hdr);

    _check_normaltrigger(ri, peer, name, arch);

/*
 * get the package name and arch
 * check normal trigger scripts
 * check file trigger scripts
 * check transfile trigger scripts
 */

    return;
}

/*
 * Main driver for the 'triggers' inspection.
 */
bool inspect_triggers(struct rpminspect *ri)
{
    bool result = true;
    rpmpeer_entry_t *peer = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are reported via INSPECT_EMPTYRPM */
        if (peer->after_rpm == NULL) {
            continue;
        }

        triggers_driver(ri, peer);
    }

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_TRIGGERS;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
