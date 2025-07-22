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

/* Global variables */
static struct rpminspect *sri = NULL;
static char *file_to_find = NULL;
static filetype_t filetype = FILETYPE_NULL;

/*
 * From:
 * https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html#icon_lookup
 */
static const char *icon_extensions[] = { ".png", ".svg", ".xpm", NULL };

/*
 * Helper used by nftw() in validate_desktop_contents()
 */
static int find_file(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
    int i = 0;
    char *bn = NULL;
    char *tmp = NULL;
    char *last = NULL;
    char *tmpicon = NULL;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    /* Only looking at regular files */
    /*
     * XXX: should FTW_SLN have special handling here?  does packaging policy
     * allow packages to ship absolute symlinks to paths not provided by the
     * package itself?
     */
    if (tflag == FTW_D || tflag == FTW_DNR || tflag == FTW_DP || tflag == FTW_NS) {
        return 0;
    }

    /* Skip debug paths */
    if (is_debug_or_build_path(fpath)) {
        return 0;
    }

    /* Look for this name as the basename */
    if (filetype == FILETYPE_EXECUTABLE) {
        list = strsplit(file_to_find, " ");

        if (list != NULL && !TAILQ_EMPTY(list)) {
            TAILQ_FOREACH_REVERSE(entry, list, string_entry_s, items) {
                /* safety check */
                if (entry->data == NULL) {
                    continue;
                }

                /* skip desktop spec params and any variables */
                if (strchr(entry->data, '%') || strchr(entry->data, '=')) {
                    continue;
                }

                if (*entry->data == PATH_SEP) {
                    tmp = strdup(entry->data);
                } else {
                    /* everything else would be in /usr/bin */
                    xasprintf(&tmp, "/usr/bin/%s", entry->data);
                }

                /* actual check */
                if (strsuffix(fpath, tmp)) {
                    free(file_to_find);
                    file_to_find = strdup(fpath);
                    list_free(list, free);
                    free(tmp);
                    return 1;
                }

                free(tmp);
            }
        }

        list_free(list, free);
    }

    /* Might be a base name missing a graphics format ending */
    /*
     * This will take an fpath like:
     *     iconfile.png
     *     iconfile.jpg
     *     org.Organization.IconFile
     * And will trim the end of the string from the last period
     * to the end.  So if a desktop entry file specifies 'iconfile'
     * and the package provides iconfile.* somewhere as a file, this
     * will pass.
     */
    if (filetype == FILETYPE_ICON) {
        if (strsuffix(fpath, file_to_find) && strprefix(mime_type(sri, fpath), "image/")) {
            /* file is found and is an image type according to libmagic */
            free(file_to_find);
            file_to_find = strdup(fpath);
            return 1;
        } else {
            /* handle icon specs without an extension */
            bn = strdup(file_to_find);
            assert(bn != NULL);
            last = basename(bn);
            assert(last != NULL);

            for (i = 0; icon_extensions[i] != NULL; i++) {
                xasprintf(&tmpicon, "%s%s", last, icon_extensions[i]);
                assert(tmpicon != NULL);

                if (strsuffix(fpath, tmpicon)) {
                    free(file_to_find);
                    file_to_find = strdup(fpath);
                    free(bn);
                    free(tmpicon);
                    return 1;
                }

                free(tmpicon);
            }

            free(bn);
        }
    }

    return 0;
}

/*
 * Called by desktop_driver() to determine if a found file is one we want to
 * look at.  Returns true if it is, false otherwise.
 */
static bool is_desktop_entry_file(const char *desktop_entry_files_dir, const rpmfile_entry_t *file)
{
    assert(desktop_entry_files_dir != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return false;
    }

    /* Is this a regular file? */
    if (!file->fullpath || !S_ISREG(file->st_mode)) {
        return false;
    }

    /* Make sure we are looking at a desktop file */
    if (!strprefix(file->localpath, desktop_entry_files_dir)) {
        return false;
    }

    if (!strsuffix(file->localpath, DESKTOP_FILENAME_EXTENSION) && !strsuffix(file->localpath, DIRECTORY_FILENAME_EXTENSION)) {
        return false;
    }

    return true;
}

/*
 * Validate the Exec= and Icon= lines in a desktop entry file.  False means
 * something didn't validate.  Results are reported from this function.
 */
static bool validate_desktop_contents(struct rpminspect *ri, const rpmfile_entry_t *file)
{
    bool result = true;
    string_list_t *contents = NULL;
    string_entry_t *entry = NULL;
    char *buf = NULL;
    char *tmp = NULL;
    const char *arch = NULL;
    struct stat sb;
    bool found = false;
    rpmpeer_entry_t *peer = NULL;
    struct result_params params;
    desktop_skips_t *ds = NULL;
    unsigned int flags = 0;
    char *key_exec = NULL;
    char *key_icon = NULL;
    char *key_tryexec = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Not for source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Ignore debug and build paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* Get the package architecture and the extraction subtree */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up result parameters */
    init_result_params(&params);
    params.header = NAME_DESKTOP;
    params.remedy = REMEDY_DESKTOP;
    params.arch = arch;
    params.file = file->localpath;

    /* Read the desktop entry file */
    contents = read_file(file->fullpath);

    if (contents == NULL) {
        return false;
    }

    /* Determine if we need to skip the Exec= check for this file. */
    HASH_FIND_STR(ri->desktop_skips, file->localpath, ds);

    if (ds != NULL) {
        flags = ds->flags;
    }

    /*
     * Iterate over the entire file line by line looking for Exec= and Icon=
     * lines.  When found, validate the value after the '='.
     */
    TAILQ_FOREACH(entry, contents, items) {
        buf = entry->data;
        filetype = FILETYPE_NULL;

        if (!(flags & SKIP_EXEC) && strprefix(buf, "Exec=")) {
            key_exec = buf + 5;
        } else if (!(flags & SKIP_ICON) && strprefix(buf, "Icon=")) {
            tmp = buf + 5;
            tmp[strcspn(tmp, "\n")] = 0;

            key_icon = tmp;
        } else if (strprefix(buf, "TryExec=")) {
            key_tryexec = buf + 8;
        }
    }

    if (key_exec != NULL) {
        filetype = FILETYPE_EXECUTABLE;
        file_to_find = strdup(key_exec);

        TAILQ_FOREACH(peer, ri->peers, items) {
            /*
             * Skip the SRPM and any package that lacks a tree (like a
             * meta package only providing headers)
             */
            if (headerIsSource(peer->after_hdr) || peer->after_root == NULL) {
                continue;
            }

            /*
             * If we get 1 back from nftw(), it means the executable was
             * found and is valid.  If found, the nftw() helper replaces
             * file_to_find with the full path to where it was found.
             */
            found = false;

            if (nftw(peer->after_root, find_file, FOPEN_MAX, FTW_MOUNT|FTW_PHYS) == 1) {
                found = true;

                if (lstat(file_to_find, &sb) == -1) {
                    warn("*** lstat");
                    list_free(contents, free);
                    free(file_to_find);
                    return false;
                }

                if (!(sb.st_mode & S_IXOTH)) {
                    xasprintf(&params.msg, _("Desktop file %s on %s references executable %s but %s is not executable by all"), file->localpath, arch, tmp, tmp);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    params.verb = VERB_FAILED;
                    params.noun = _("${FILE} references non-executable file on ${ARCH}");
                    add_result(ri, &params);
                    free(params.msg);
                    result = false;
                }
            }

            if (found) {
                break;
            }
        }

        if (!found) {
            if (key_tryexec != NULL) {
                /*
                 * At this point, nftw() did not find the executable.
                 * However, since there is TryExec in the desktop file,
                 * then the desktop file may be ignored by menu
                 * implementations. Hence, report it only as "INFO"
                 * result, as this is acceptable.
                 */
               xasprintf(&params.msg, _("Desktop file %s on %s references executable %s; no subpackages contain an executable of that name, however it has a TryExec key so it may be ignored in case %s does not exist"), file->localpath, arch, tmp, key_tryexec);
                params.severity = RESULT_INFO;
                params.waiverauth = NOT_WAIVABLE;
                params.verb = VERB_OK;
                add_result(ri, &params);
                free(params.msg);
                result = false;
            } else {
                xasprintf(&params.msg, _("Desktop file %s on %s references executable %s but no subpackages contain an executable of that name"), file->localpath, arch, tmp);
                params.severity = RESULT_VERIFY;
                params.waiverauth = WAIVABLE_BY_ANYONE;
                params.verb = VERB_FAILED;
                params.noun = _("${FILE} references missing executable on ${ARCH}");
                add_result(ri, &params);
                free(params.msg);
                result = false;
            }
        }

        free(file_to_find);
    }

    if (key_icon != NULL) {
        filetype = FILETYPE_ICON;
        file_to_find = strdup(key_icon);
        found = false;

        TAILQ_FOREACH(peer, ri->peers, items) {
            /*
             * Skip the SRPM and any package that lacks a tree (like a
             * meta package only providing headers)
             */
            if (headerIsSource(peer->after_hdr) || peer->after_root == NULL) {
                continue;
            }

            /*
             * If we get 1 back from nftw(), it means the icon was
             * found and is valid.  If found, the nftw() helper replaces
             * file_to_find with the full path to where it was found.
             */
            found = false;

            if (nftw(peer->after_root, find_file, FOPEN_MAX, FTW_MOUNT|FTW_PHYS) == 1) {
                found = true;

                if (lstat(file_to_find, &sb) == -1) {
                    warn("*** lstat");
                    list_free(contents, free);
                    free(file_to_find);
                    return false;
                }

                if (!(sb.st_mode & S_IROTH)) {
                    xasprintf(&params.msg, _("Desktop file %s on %s references icon %s but %s is not readable by all"), file->localpath, arch, key_icon, key_icon);
                    params.severity = RESULT_VERIFY;
                    params.waiverauth = WAIVABLE_BY_ANYONE;
                    params.verb = VERB_FAILED;
                    params.noun = _("${FILE} references unreadable icon on ${ARCH}");
                    add_result(ri, &params);
                    free(params.msg);
                    result = false;
                }
            }

            if (found) {
                break;
            }
        }

        /* check standard system icons as a failsafe */
        if (!found && init_icons(ri) && list_contains(ri->icons, key_icon)) {
            found = true;
        }

        if (!found) {
            xasprintf(&params.msg, _("Desktop file %s on %s references icon %s but no subpackages contain %s"), file->localpath, arch, key_icon, key_icon);
            params.severity = RESULT_VERIFY;
            params.waiverauth = WAIVABLE_BY_ANYONE;
            params.verb = VERB_FAILED;
            params.noun = _("${FILE} references missing icon on ${ARCH}");
            add_result(ri, &params);
            free(params.msg);
            result = false;
        }

        free(file_to_find);
    }

    list_free(contents, free);

    return result;
}

static bool desktop_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    int after_code;
    char *before_out = NULL;
    const char *arch = NULL;
    char *tmpbuf = NULL;
    struct result_params params;

    /* allow static callback functions to see ri */
    sri = ri;

    /*
     * Is this a file we should look at?
     * NOTE: Returning 'true' here is like 'continue' in the calling loop.
     */
    if (!is_desktop_entry_file(ri->desktop_entry_files_dir, file)) {
        return true;
    }

    /* Get result parameters ready */
    init_result_params(&params);

    /* Validate the desktop file */
    params.details = run_cmd(&after_code, ri->worksubdir, ri->commands.desktop_file_validate, "--no-hints", file->fullpath, NULL);
    tmpbuf = strreplace(params.details, file->fullpath, file->localpath);
    free(params.details);
    params.details = tmpbuf;

    if (file->peer_file && is_desktop_entry_file(ri->desktop_entry_files_dir, file->peer_file)) {
        /* if we have a before peer, validate the corresponding desktop file */
        before_out = run_cmd(NULL, ri->worksubdir, ri->commands.desktop_file_validate, "--no-hints", file->peer_file->fullpath, NULL);
        tmpbuf = strreplace(before_out, file->peer_file->fullpath, file->peer_file->localpath);
        free(before_out);
        before_out = tmpbuf;
    }

    if (after_code) {
        /* non-zero on exit is a failed desktop file */
        result = false;
    }

    /* Report validation results */
    arch = get_rpm_header_arch(file->rpm_header);

    if (after_code == 0) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
    } else {
        params.severity = RESULT_BAD;
        params.waiverauth = WAIVABLE_BY_ANYONE;
    }

    params.header = NAME_DESKTOP;
    params.remedy = REMEDY_DESKTOP;
    params.arch = arch;
    params.file = file->localpath;
    params.verb = VERB_CHANGED;
    params.noun = _("${FILE} is not valid on ${ARCH}");

    if (file->peer_file && before_out == NULL && params.details != NULL) {
        xasprintf(&params.msg, _("File %s is no longer a valid desktop entry file on %s; desktop-file-validate reports:"), file->localpath, arch);
    } else if (file->peer_file == NULL && params.details != NULL) {
        xasprintf(&params.msg, _("New file %s is not a valid desktop file on %s; desktop-file-validate reports:"), file->localpath, arch);
    } else if (params.details != NULL) {
        xasprintf(&params.msg, _("File %s is not a valid desktop file on %s; desktop-file-validate reports:"), file->localpath, arch);
    }

    if (params.msg) {
        add_result(ri, &params);
        free(params.msg);
    }

    free(params.details);
    free(before_out);

    /* Validate the contents of the desktop entry file */
    if (!validate_desktop_contents(ri, file) && result) {
        result = false;
    }

    return result;
}

/*
 * Main driver for the 'desktop' inspection.
 */
bool inspect_desktop(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    assert(ri != NULL);
    assert(ri->peers != NULL);

    /*
     * The desktop inspection looks at *.desktop and *.directory files
     * under /usr/share/applications and runs desktop-file-validate on
     * them.  The before and after peers are compared for these files.
     * For the after files, the Exec and Icon references are checked.
     */
    result = foreach_peer_file(ri, NAME_DESKTOP, desktop_driver);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_DESKTOP;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
