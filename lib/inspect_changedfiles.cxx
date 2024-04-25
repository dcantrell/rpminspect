/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <assert.h>

#include "rpminspect.h"

static bool reported = false;

/*
 * Runs a command and redirects standard out to a temporary file created
 * in this function.  Caller is responsible for removing the temporary
 * file.
 */
static char *run_and_capture(const char *where, char **output, char *cmd, const char *fullpath, int *exitcode)
{
    int fd;

    assert(where != NULL);
    assert(output != NULL);
    assert(cmd != NULL);
    assert(fullpath != NULL);
    assert(exitcode != NULL);

    /* Build a temporary file */
    xasprintf(output, "%s/output.XXXXXX", where);

    /* Generate them and then close them */
    fd = mkstemp(*output);

    if (fd == -1) {
        warn("*** mkstemp");
        return NULL;
    }

    if (close(fd) == -1) {
        warn("*** close");
        return NULL;
    }

    /* Run command and return output */
    return run_cmd(exitcode, NULL, cmd, "-o", *output, fullpath, NULL);
}

/*
 * Performs all of the tests associated with the changedfiles inspection.
 */
static bool changedfiles_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool ignore = false;
    int ct = 0;
    int flags = O_RDONLY | O_CLOEXEC;
    const char *arch = NULL;
    char *nvr = NULL;
    const char *type = NULL;
    char *before_sum = NULL;
    char *after_sum = NULL;
    char *errors = NULL;
    char *short_errors = NULL;
    char *skip_line = NULL;
    int exitcode = 0;
    bool possible_header = false;
    string_entry_t *entry = NULL;
    char *before_tmp = NULL;
    char *after_tmp = NULL;
    char *before_uncompressed_file = NULL;
    char *after_uncompressed_file = NULL;
    rpmfile_entry_t *bun = NULL;
    rpmfile_entry_t *aun = NULL;
    const char *comptype = NULL;
    char *s = NULL;
    int fd;
    char magic[4];
    bool rebase = false;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

#ifdef O_LARGEFILE
    flags |= O_LARGEFILE;
#endif

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip files without a peer, other inspections handle new/missing files */
    if (!file->peer_file) {
        return true;
    }

    /* Ignore debuginfo and debugsource paths */
    if (is_debug_or_build_path(file->localpath)) {
        return true;
    }

    /* Only perform checks on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* Skip files in the debug path and debug source path */
    if (strprefix(file->localpath, DEBUG_PATH) || strprefix(file->localpath, DEBUG_SRC_PATH)) {
        return true;
    }

    /* ELF content changing is handled by other inspections */
    if (is_elf(file)) {
        return true;
    }

    /*
     * Determine if we are running on a rebased package or just a
     * package update.
     */
    rebase = is_rebase(ri);

    /* The architecture is used in reporting messages */
    arch = get_rpm_header_arch(file->rpm_header);

    /* Set up the result parameters */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_CHANGEDFILES;
    params.arch = arch;
    params.file = file->localpath;

    /* We will skip checks for ignored files */
    ignore = ignore_rpmfile_entry(ri, NAME_CHANGEDFILES, file);

    /* Set the waiver type if this is a file of security concern */
    if (ri->security_path_prefix) {
        TAILQ_FOREACH(entry, ri->security_path_prefix, items) {
            while (*entry->data != '/') {
                entry->data++;
            }

            if (strprefix(file->localpath, entry->data)) {
                params.severity = RESULT_BAD;
                params.waiverauth = WAIVABLE_BY_SECURITY;

                /* security-related results cannot be ignored */
                ignore = false;

                break;
            }
        }
    }

    /* Get the MIME type of the file, will need that */
    type = get_mime_type(ri, file);

    /* Skip Java class files and JAR files (handled elsewhere) */
    if (type
        && ((!strcmp(type, "application/zip") && strsuffix(file->fullpath, JAR_FILENAME_EXTENSION))
             || (!strcmp(type, "application/x-java-applet") && strsuffix(file->fullpath, CLASS_FILENAME_EXTENSION)))) {
        return true;
    }

    /* Skip Python bytecode files (these always change) */
    if (type
        && !strcmp(type, "application/octet-stream")
        && (strsuffix(file->fullpath, PYTHON_PYC_FILE_EXTENSION)
            || strsuffix(file->fullpath, PYTHON_PYO_FILE_EXTENSION))) {
        /* Double check that this is a Python bytecode file */
        fd = open(file->fullpath, flags);

        if (fd == -1) {
            warn("*** open");
            return true;
        }

        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            warn("*** read");

            if (close(fd) == -1) {
                warn("*** close");
            }

            return true;
        }

        if (close(fd) == -1) {
            warn("*** close");
            return true;
        }

        /*
         * Python bytecode files begin with 0x__0D0D0A
         * The __ is a version identifier which changes from time to time
         */
        if (magic[1] == '\x0D' && magic[2] == '\x0D' && magic[3] == '\x0A') {
            return true;
        }
    }

    /*
     * Compare compressed files
     *
     * Don't assume compressed files are text, so just perform a byte
     * comparison and report if the uncompressed content has changed
     * between builds.  The idea here is that the before and after
     * build could change the compression ratios or other properties
     * but the uncompressed content would be the same.
     */

    /* the octet-stream check is a workaround for bad/old versions of libmagic */
    if (type) {
        ct = (!strcmp(type, "application/x-gzip") || !strcmp(type, "application/gzip") || !strcmp(type, "application/x-bzip2") ||
              !strcmp(type, "application/bzip2") || !strcmp(type, "application/x-xz") || !strcmp(type, "application/xz")) ||
             (!strcmp(type, "application/octet-stream") &&
              (strsuffix(file->localpath, ".gz") || strsuffix(file->localpath, ".bz2") || strsuffix(file->localpath, ".xz")));
    }

    if (ct && ((!ignore && (ri->tests & INSPECT_CHANGEDFILES)) || params.waiverauth == WAIVABLE_BY_SECURITY)) {
        /* uncompress the files to temporary files for comparison */
        before_uncompressed_file = uncompress_file(ri, file->peer_file->fullpath, NAME_CHANGEDFILES);
        after_uncompressed_file = uncompress_file(ri, file->fullpath, NAME_CHANGEDFILES);

        /* we may not have been able to uncompress */
        if (before_uncompressed_file == NULL || after_uncompressed_file == NULL) {
            /* perform a byte comparison of the compressed files */
            exitcode = filecmp(file->peer_file->fullpath, file->fullpath);
        } else {
            /* we can use diff on text files, so try that first */
            bun = calloc(1, sizeof(*bun));
            assert(bun != NULL);
            aun = calloc(1, sizeof(*aun));
            assert(aun != NULL);

            bun->fullpath = before_uncompressed_file;
            aun->fullpath = after_uncompressed_file;

            if (is_text_file(ri, bun) && is_text_file(ri, aun)) {
                /* uncompressed files are text, use diff */
                params.details = get_file_delta(bun->fullpath, aun->fullpath);

                /* clean up the diff headers */
                if (params.details) {
                    s = strreplace(params.details, bun->fullpath, file->peer_file->localpath);
                    free(params.details);
                    params.details = s;

                    s = strreplace(params.details, aun->fullpath, file->localpath);
                    free(params.details);
                    params.details = s;
                }
            } else {
                /* perform a byte comparison of the uncompressed files */
                exitcode = filecmp(bun->fullpath, aun->fullpath);
            }

            /* clean up */
            free(bun->fullpath);
            free(bun->type);
            free(bun);

            free(aun->fullpath);
            free(aun->type);
            free(aun);

            before_uncompressed_file = NULL;
            after_uncompressed_file = NULL;
        }

        if (exitcode || params.details) {
            if (type) {
                /* get a reporting type for the message */
                if (rindex(type, '/')) {
                    comptype = rindex(type, '/') + 1;
                    assert(comptype != NULL);
                } else {
                    comptype = type;
                }

                if (rindex(comptype, '-')) {
                    comptype = rindex(comptype, '-') + 1;
                    assert(comptype != NULL);
                }
            } else {
                type = "[unknown type]";
            }

            /* the files are different, report */
            nvr = get_nevr(file->rpm_header);
            xasprintf(&params.msg, _("Compressed %s file %s changed content in %s on %s."), comptype, file->localpath, nvr, arch);
            params.verb = VERB_CHANGED;
            params.noun = file->localpath;
            add_result(ri, &params);
            reported = true;
        }

        /* cleanup */
        free(before_uncompressed_file);
        free(after_uncompressed_file);

        goto done;
    }

    /*
     * Compare gettext .mo files and report any changes.
     */
    if (!ignore && type
        && !strcmp(type, "application/x-gettext-translation") && strsuffix(file->localpath, MO_FILENAME_EXTENSION)
        && (ri->tests & INSPECT_CHANGEDFILES)) {
        /*
         * This one is somewhat complicated.  We run msgunfmt on the mo files,
         * but first we have to make temporary files for that output.  Then
         * invoke diff(1) on those files and capture the output for reporting
         * out.  It's simple, but the fact that we have to use these command
         * line programs makes it a bit complicated.  Patches welcome that do
         * this with a library call or two.
         */

        /* First, unformat the mo files */
        params.details = run_and_capture(ri->workdir, &after_tmp, ri->commands.msgunfmt, file->fullpath, &exitcode);

        if (exitcode) {
            nvr = get_nevr(file->rpm_header);
            xasprintf(&params.msg, _("Error running msgunfmt on %s in %s on %s; malformed mo file?"), file->localpath, nvr, arch);
            params.severity = RESULT_BAD;
            params.remedy = get_remedy(REMEDY_CHANGEDFILES);
            params.verb = VERB_FAILED;
            params.noun = _("msgunfmt on ${FILE}");
            add_result(ri, &params);
            reported = true;
            goto done;
        }

        params.details = run_and_capture(ri->workdir, &before_tmp, ri->commands.msgunfmt, file->peer_file->fullpath, &exitcode);

        if (exitcode) {
            nvr = get_nevr(file->peer_file->rpm_header);
            xasprintf(&params.msg, _("Error running msgunfmt on %s in %s on %s; malformed mo file?"), file->peer_file->localpath, nvr, arch);
            params.severity = RESULT_BAD;
            params.remedy = get_remedy(REMEDY_CHANGEDFILES);
            params.verb = VERB_FAILED;
            params.noun = _("msgunfmt on ${FILE}");
            add_result(ri, &params);
            reported = true;
            goto done;
        }

        /* Now diff the mo content */
        params.details = get_file_delta(before_tmp, after_tmp);

        if (params.details) {
            nvr = get_nevr(file->rpm_header);
            xasprintf(&params.msg, _("Message catalog %s changed content in %s on %s"), file->localpath, nvr, arch);
            params.severity = RESULT_INFO;
            params.remedy = get_remedy(REMEDY_CHANGEDFILES);
            params.verb = VERB_CHANGED;
            params.noun = _("${FILE}");
            add_result(ri, &params);
            reported = true;
            goto done;
        }
    }

    /*
     * Compare C and C++ header files and report changes.
     * NOTE:  We check the MIME type of the file and then see if the name
     * ends with .h, .H, .hpp, or .hxx.  The extension list could probably
     * be a configuration file change.  But more importantly, this check
     * excludes any header files that lack a file ending like this.
     */
    if (!ignore && ri->header_file_extensions) {
        TAILQ_FOREACH(entry, ri->header_file_extensions, items) {
            if (strsuffix(file->localpath, entry->data)) {
                possible_header = true;
                break;
            }
        }
    }

    if (type && !strcmp(type, "text/x-c") && possible_header && (ri->tests & INSPECT_CHANGEDFILES)) {
        /* Now diff the header content */
        errors = get_file_delta(file->peer_file->fullpath, file->fullpath);

        if (errors) {
            /*
             * Skip the diff(1) header since the output from this
             * gives context.
             */
            short_errors = errors;

            if (strlen(short_errors) >= 3) {
                while (strncmp(short_errors, "@@ ", 3)) {
                    skip_line = index(short_errors, '\n');

                    if (skip_line == NULL) {
                        short_errors = errors;
                        break;
                    }

                    short_errors = skip_line + 1;

                    if (*short_errors == '\0') {
                        short_errors = errors;
                        break;
                    }
                }
            }

            nvr = get_nevr(file->rpm_header);
            xasprintf(&params.msg, _("Public header file %s changed content in %s on %s.  A unified diff of the changes follows."), file->localpath, nvr, arch);
            params.severity = RESULT_INFO;
            params.details = short_errors;
            params.verb = VERB_CHANGED;
            params.noun = _("${FILE}");
            add_result(ri, &params);
            reported = true;

            /* details is not allocated here, freeing errors will take care of it */
            params.details = NULL;
            free(errors);

            goto done;
        }
    }

    /* Finally, anything that gets down to here just compare checksums. */
    if (!rebase && !ignore && (ri->tests & INSPECT_CHANGEDFILES)) {
        before_sum = checksum(file->peer_file);
        after_sum = checksum(file);

        if (before_sum && after_sum && strcmp(before_sum, after_sum)) {
            nvr = get_nevr(file->rpm_header);
            params.severity = RESULT_INFO;
            params.verb = VERB_CHANGED;
            params.noun = _("${FILE}");

            if (params.waiverauth == WAIVABLE_BY_SECURITY) {
                xasprintf(&params.msg, _("File %s changed content in %s on %s.  Changes to security policy related files require inspection by the Security Response Team."), file->localpath, nvr, arch);
            } else {
                xasprintf(&params.msg, _("File %s changed content in %s on %s."), file->localpath, nvr, arch);
            }

            assert(params.msg != NULL);
            add_result(ri, &params);
            reported = true;
        }
    }

done:
    free(nvr);
    free(params.msg);
    free(params.details);

    if (before_tmp) {
        if (unlink(before_tmp) == -1) {
            warn("*** unlink");
        }

        free(before_tmp);
    }

    if (after_tmp) {
        if (unlink(after_tmp) == -1) {
            warn("*** unlink");
        }

        free(after_tmp);
    }

    if (params.severity >= RESULT_VERIFY && reported) {
        return false;
    } else {
        return true;
    }
}

bool inspect_changedfiles(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    result = foreach_peer_file(ri, NAME_CHANGEDFILES, changedfiles_driver);

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_CHANGEDFILES;
        add_result(ri, &params);
    }

    return result;
}
