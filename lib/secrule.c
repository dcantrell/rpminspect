/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include <fnmatch.h>
#include "rpminspect.h"

/*
 * Returns NULL if the path is not matched, which means the result
 * should be reported per default rules.  If it is found, the
 * security_entry_t for the match is returned and the caller can take
 * appropriate reporting action.
 */
security_entry_t *get_secrule_by_path(struct rpminspect *ri, const rpmfile_entry_t *file)
{
    const char *name = NULL;
    const char *version = NULL;
    const char *release = NULL;
    security_entry_t *sentry = NULL;
    int flags = FNM_NOESCAPE | FNM_PATHNAME;
    int w = 0;
    int x = 0;
    int y = 0;
    int z = 0;

    assert(ri != NULL);
    assert(file != NULL);
    assert(file->rpm_header != NULL);
    assert(file->localpath != NULL);

    /* initialize the security table */
    if (!ri->security_initialized) {
        if (!init_security(ri)) {
            return NULL;
        }
    }

    /* get NVR which will be used in the matching loop */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);
    version = headerGetString(file->rpm_header, RPMTAG_VERSION);
    release = headerGetString(file->rpm_header, RPMTAG_RELEASE);

    /* try to find a secrule */
    TAILQ_FOREACH(sentry, ri->security, items) {
        w = fnmatch(sentry->path, file->localpath, flags);
        x = fnmatch(sentry->pkg, name, flags);
        y = fnmatch(sentry->ver, version, flags);
        z = fnmatch(sentry->rel, release, flags);

        if (w == 0 && x == 0 && y == 0 && z == 0) {
            /* match found */
            return sentry;
        }
    }

    /* no match found */
    return NULL;
}

/*
 * Given a secrule, return its result reporting severity.  These are
 * distinct from the severity_t types in order to give the secrule
 * workflow more flexibility.
 */
severity_t get_secrule_result_severity(struct rpminspect *ri, const rpmfile_entry_t *file, const int type)
{
    security_entry_t *sentry = NULL;
    secrule_t *srule = NULL;
    secrule_t *tmp_srule = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* get the security rule entry for this path */
    sentry = get_secrule_by_path(ri, file);

    /* no rules defined, default result */
    if (sentry == NULL || sentry->rules == NULL) {
        return RESULT_BAD;
    }

    /* find the rule for this type */
    HASH_ITER(hh, sentry->rules, srule, tmp_srule) {
        if (srule->type == type) {
            return srule->severity;
        }
    }

    /* nothing found for this type, default result */
    return RESULT_BAD;
}
