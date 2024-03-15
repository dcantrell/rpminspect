/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <err.h>

#ifdef _WITH_LIBCAP
#include <sys/capability.h>
#endif

#include "rpminspect.h"

bool check_permissions(struct rpminspect *ri, const rpmfile_entry_t *file, const char *header, bool *reported, bool force_non_security_checks)
{
    bool result = true;
    bool ignore = false;
    bool id_bit = false;
    bool relaxed = false;
    const char *arch = NULL;
    char *change = NULL;
    mode_t before_mode;
    mode_t after_mode;
    mode_t mode_diff;
    bool allowed = false;
    char *noun = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* Ignore files in the SRPM */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debuginfo and debugsource packages */
    if (strprefix(file->localpath, DEBUG_PATH) || strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* We will skip checks for ignored files */
    ignore = ignore_rpmfile_entry(ri, header, file);

    /* We need the architecture for reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = header;
    params.arch = arch;
    params.file = file->localpath;

    /* Local working copies for display */
    after_mode = file->st.st_mode & 0777;

    /* Compare the modes */
    if (file->peer_file) {
        before_mode = file->peer_file->st.st_mode & 0777;
    } else {
        memset(&before_mode, 0, sizeof(before_mode));
    }

    mode_diff = before_mode ^ after_mode;
    allowed = match_fileinfo_mode(ri, file, header, &result, reported);

    /* in cases of setuid or setgid bits on the mode, this is a non-ignorable check */
    id_bit = (!(before_mode & (S_ISUID|S_ISGID)) && (after_mode & (S_ISUID|S_ISGID)));

    /* have permissions been relaxed in cases we want to verify? */
    relaxed = (S_ISDIR(file->st.st_mode) && !S_ISLNK(file->st.st_mode)) && (((mode_diff & S_ISVTX) && !(after_mode & S_ISVTX)) || ((after_mode & mode_diff) != 0));

    if (id_bit) {
        ignore = false;
    }

    /* if setuid/setgid or new mode is more open */
    if (!ignore && mode_diff && file->peer_file && !allowed && ((ri->tests & INSPECT_PERMISSIONS) || force_non_security_checks)) {
        /* initial check of this happens outside this block */
        if (id_bit || relaxed) {
            params.severity = RESULT_VERIFY;
        }

        if (S_ISDIR(file->st.st_mode) && !S_ISDIR(file->peer_file->st.st_mode)) {
            if (id_bit) {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a directory; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a directory; permissions changed"), file->localpath);
                }
            } else {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s became a directory; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s became a directory; permissions changed"), file->localpath);
                }
            }
        } else if (S_ISCHR(file->st.st_mode) && !S_ISCHR(file->peer_file->st.st_mode)) {
            if (id_bit) {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a character device; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a character device; permissions changed"), file->localpath);
                }
            } else {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s became a character device; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s became a character device; permissions changed"), file->localpath);
                }
            }
        } else if (S_ISBLK(file->st.st_mode) && !S_ISBLK(file->peer_file->st.st_mode)) {
            if (id_bit) {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a block device; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a block device; permissions changed"), file->localpath);
                }
            } else {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s became a block device; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s became a block device; permissions changed"), file->localpath);
                }
            }
        } else if (S_ISREG(file->st.st_mode) && !S_ISREG(file->peer_file->st.st_mode)) {
            if (id_bit) {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a regular file; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a regular file; permissions changed"), file->localpath);
                }
            } else {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s became a regular file; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s became a regular file; permissions changed"), file->localpath);
                }
            }
        } else if (S_ISFIFO(file->st.st_mode) && !S_ISFIFO(file->peer_file->st.st_mode)) {
            if (id_bit) {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a FIFO; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a FIFO; permissions changed"), file->localpath);
                }
            } else {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s became a FIFO; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s became a FIFO; permissions changed"), file->localpath);
                }
            }
        } else if (S_ISLNK(file->st.st_mode) && !S_ISLNK(file->peer_file->st.st_mode)) {
            if (id_bit) {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a symbolic link; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a symbolic link; permissions changed"), file->localpath);
                }
            } else {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s became a symbolic link; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s became a symbolic link; permissions changed"), file->localpath);
                }
            }
        } else if (S_ISSOCK(file->st.st_mode) && !S_ISSOCK(file->peer_file->st.st_mode)) {
            if (id_bit) {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a socket; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s changed setuid/setgid; became a socket; permissions changed"), file->localpath);
                }
            } else {
                if (relaxed) {
                    xasprintf(&params.msg, _("%s became a socket; permissions relaxed"), file->localpath);
                } else {
                    xasprintf(&params.msg, _("%s became a socket; permissions changed"), file->localpath);
                }
            }
        }

        assert(params.msg != NULL);

        if (params.severity >= RESULT_VERIFY) {
            if (id_bit) {
                params.waiverauth = WAIVABLE_BY_SECURITY;
            } else {
                params.waiverauth = WAIVABLE_BY_ANYONE;
            }

            result = false;
        }

        xasprintf(&change, _("%s from %04o to %04o on %s"), params.msg, before_mode, after_mode, arch);
        assert(change != NULL);
        free(params.msg);
        params.msg = change;

        params.verb = VERB_CHANGED;
        xasprintf(&noun, _("${FILE} permissions from %04o to %04o on ${ARCH}"), before_mode, after_mode);
        params.noun = noun;
        add_result(ri, &params);
        free(params.msg);
        free(noun);

        *reported = true;
    }

    /* check for world-writability */
    if (!allowed && !S_ISLNK(file->st.st_mode) && ((after_mode & (S_IWOTH|S_ISVTX)) || (after_mode & S_IWOTH))) {
        params.severity = get_secrule_result_severity(ri, file, SECRULE_WORLDWRITABLE);

        if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
            xasprintf(&params.msg, _("%s (%s) is world-writable on %s"), file->localpath, get_mime_type(ri, file), arch);
            params.waiverauth = WAIVABLE_BY_SECURITY;
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} is world-writable on ${ARCH}");
            add_result(ri, &params);
            free(params.msg);

            result = false;
            *reported = true;
        }
    }

    return result;
}
