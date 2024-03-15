/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <libgen.h>
#include <ftw.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>

#include "rpminspect.h"

/*
 * Called by udevrules_driver() to determine if a found file is one
 * we want to look at.  Returns true if it is, false otherwise.
 */
static bool is_udev_rules_file(struct rpminspect *ri, const rpmfile_entry_t *file)
{
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return false;
    }

    /* Is this a regular file? */
    if (!file->fullpath || !S_ISREG(file->st.st_mode)) {
        return false;
    }

    /* Make sure we are looking at a udev rules file */
    if (!strsuffix(file->localpath, UDEV_RULES_FILENAME_EXTENSION)) {
        return false;
    }

    TAILQ_FOREACH(entry, ri->udev_rules_dirs, items) {
        if (strprefix(file->localpath, entry->data)) {
            return true;
        }
    }

    return false;
}

/*
 * A trivial wrapper called by udevrules_driver() and inspect_udevrules()
 * to invoke udevadm verify.
 */
static char *run_udevadm_verify(int *exitcode, const struct rpminspect *ri, const char *arg)
{
    assert(ri != NULL);
    assert(arg != NULL);

    return run_cmd(exitcode, ri->worksubdir, ri->commands.udevadm, "verify", "--no-summary", "--no-style", "--resolve-names=never", arg, NULL);
}

static bool udevrules_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    const char *arch = NULL;
    int before_rc = 0;
    int after_rc = 0;
    char *details = NULL;
    char *tmpbuf;
    struct result_params params;

    /*
     * Is this a file we should look at?
     * NOTE: Returning 'true' here is like 'continue' in the calling loop.
     */
    if (!is_udev_rules_file(ri, file)) {
        return true;
    }

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up the result parameters */
    init_result_params(&params);
    params.header = NAME_UDEVRULES;
    params.verb = VERB_OK;
    params.arch = arch;
    params.file = file->localpath;

    /* Validate the udev rules file */
    tmpbuf = run_udevadm_verify(&after_rc, ri, file->fullpath);
    details = strreplace(tmpbuf, file->fullpath, file->localpath);
    free(tmpbuf);

    /* If we have a before peer, validate the corresponding udev rules file */
    if (file->peer_file && is_udev_rules_file(ri, file->peer_file)) {
        tmpbuf = run_udevadm_verify(&before_rc, ri, file->peer_file->fullpath);
        free(tmpbuf);

        if (before_rc == 0 && after_rc == 0) {
            xasprintf(&params.msg, _("%s is a valid udev rules file on %s"), file->localpath, arch);
        } else if (before_rc && after_rc == 0) {
            xasprintf(&params.msg, _("%s is now a valid udev rules file on %s"), file->localpath, arch);
            params.verb = VERB_CHANGED;
        } else if (before_rc == 0 && after_rc) {
            xasprintf(&params.msg, _("%s is no longer a valid udev rules file on %s"), file->localpath, arch);
            params.verb = VERB_CHANGED;
        } else {
            xasprintf(&params.msg, _("%s is not a valid udev rules file on %s"), file->localpath, arch);
        }
    } else {
        if (after_rc == 0) {
            xasprintf(&params.msg, _("%s is a valid udev rules file on %s"), file->localpath, arch);
        } else {
            xasprintf(&params.msg, _("%s is not a valid udev rules file on %s"), file->localpath, arch);
        }
    }

    if (after_rc == 0) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
    } else {
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
        params.remedy = get_remedy(REMEDY_UDEVRULES);
        params.details = details;
        result = false;
    }

    if (params.msg) {
        add_result(ri, &params);
        free(params.msg);
    }

    free(details);

    return result;
}

/*
 * Main driver for the 'udevrules' inspection.
 */
bool inspect_udevrules(struct rpminspect *ri)
{
    char *details;
    int rc = 0;
    bool result;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /* Check whether udevadm verify is available. */
    details = run_cmd(&rc, ri->worksubdir, ri->commands.udevadm, "verify", "--help", NULL);

    /* Skip the inspection if udevadm verify is not available. */
    if (rc != 0) {
        init_result_params(&params);
        params.header = NAME_UDEVRULES;
        params.severity = RESULT_SKIP;
        params.verb = VERB_SKIP;
        params.msg = _("The 'udevadm verify' command does not operate as expected on this system.");
        params.details = details;
        add_result(ri, &params);
        free(details);
        return true;
    }

    free(details);

    /* Perform syntax check on udev rules files using udevadm verify. */
    result = foreach_peer_file(ri, NAME_UDEVRULES, udevrules_driver);

    if (result) {
        init_result_params(&params);
        params.header = NAME_UDEVRULES;
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
