/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <err.h>

#include "rpminspect.h"

/*
 * Determine if a build specification is local or not.
 */
bool is_local_build(const char *workdir, const char *build, const bool fetch_only)
{
    char cwd[PATH_MAX + 1];
    char *check = NULL;
    char *r = NULL;
    struct stat sb;

    /* ignore unused variable warnings if assert is disabled */
    (void) r;

    if (build == NULL) {
        return false;
    }

    /* get current dir */
    memset(cwd, '\0', sizeof(cwd));

    if (getcwd(cwd, PATH_MAX) == NULL) {
        err(RI_PROGRAM_ERROR, "getcwd");
    }

    /* Figure out where to look */
    if (fetch_only && workdir) {
        xasprintf(&check, "%s/%s", workdir, build);
    } else {
        check = strdup(build);
    }

    /* Can we read it as a directory off the filesystem? */
    if (access(check, R_OK)) {
        free(check);
        return false;
    }

    if (stat(check, &sb) == -1) {
        warn("stat");
        free(check);
        return false;
    }

    if (!S_ISDIR(sb.st_mode)) {
        return false;
    }

    if (chdir(check) == -1) {
        warn("chdir");
        free(check);
        return false;
    }

    free(check);

    if (chdir(cwd) == -1) {
        warn("chdir");
        return false;
    }

    return true;
}

/*
 * Returns true if the specified filename is a local RPM.
 */
bool is_local_rpm(struct rpminspect *ri, const char *rpm)
{
    Header h;
    char *rpmpath = NULL;
    bool ret = true;

    if (rpm == NULL) {
        return false;
    }

    assert(ri != NULL);

    if (access(rpm, R_OK)) {
        return false;
    }

    rpmpath = realpath(rpm, NULL);
    h = get_rpm_header(ri, rpm);

    if ((rpmpath == NULL) || (h == NULL)) {
        ret = false;
    }

    free(rpmpath);
    return ret;
}
