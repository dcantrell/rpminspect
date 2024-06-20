/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <assert.h>
#include <libgen.h>
#include <errno.h>
#include <err.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <rpm/rpmspec.h>
#include <rpm/rpmbuild.h>
#include <rpm/rpmlog.h>
#include <unicode/ustdio.h>
#include <unicode/ustring.h>
#include "rpminspect.h"

/* subdirectories to create or link for the rpmbuild structure */
static const char *subdirs[] = { RPMBUILD_BUILDDIR,
                                 RPMBUILD_BUILDROOTDIR,
                                 RPMBUILD_RPMDIR,
                                 RPMBUILD_SOURCEDIR,
                                 RPMBUILD_SPECDIR,
                                 RPMBUILD_SRPMDIR,
                                 NULL };

/* has the SRPM been checked? */
static bool seen = false;

/* these globals are used by the nftw() helper */
static char *root = NULL;
static char *build = NULL;
static bool uses_unpack_base = false;
static struct rpminspect *globalri = NULL;
static bool globalresult = true;
static UChar32_list_t *forbidden = NULL;
static const char *globalspec = NULL;
static const char *globalarch = NULL;
static rpmfile_entry_t *globalfile = NULL;

/*
 * Template for mkdtemp() when manual extraction of source archives
 * has to happen.  This would be used when rpminspect cannot execute
 * the spec file through the %prep section.
 */
#define UNPACK_BASE     "unpack-"
#define UNPACK_TEMPLATE UNPACK_BASE"XXXXXX"

/*
 * Helper function to determine if we should skip the named file based
 * on its MIME type.
 */
static bool is_excluded_mime_type(const char *path)
{
    const char *type = NULL;
    string_entry_t *entry = NULL;

    assert(globalri != NULL);

    /* get the MIME type */
    type = mime_type(globalri, path);

    if (type == NULL) {
        return false;
    }

    /* check to see if this MIME type is explicitly excluded */
    if (globalri->unicode_excluded_mime_types != NULL && !TAILQ_EMPTY(globalri->unicode_excluded_mime_types)) {
        TAILQ_FOREACH(entry, globalri->unicode_excluded_mime_types, items) {
            if (!strcmp(type, entry->data)) {
                return true;
            }
        }
    }

    /* ignore any non-text files */
    if (!strprefix(type, "text/")) {
        return true;
    }

    return false;
}

/*
 * Helper function to create a ~/rpmbuild tree in the working directory.
 */
static char *make_source_dirs(const char *worksubdir, const char *fullpath)
{
    int i = 0;
    int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    char *fp = NULL;
    char *shortname = NULL;
    char *sub = NULL;
    char *topdir = NULL;

    assert(worksubdir != NULL);
    assert(fullpath != NULL);

    /* use the already existing source subdirectory */
    fp = strdup(fullpath);
    assert(fp != NULL);
    shortname = dirname(fp);
    assert(shortname != NULL);

    /* create rpmbuild-like directory structure and set macros */
    xasprintf(&topdir, "%s/%s", worksubdir, RPMBUILD_TOPDIR);
    assert(topdir != NULL);

    if (mkdirp(topdir, mode) == -1) {
        free(topdir);
        return NULL;
    }

    for (i = 0; subdirs[i] != NULL; i++) {
        xasprintf(&sub, "%s/%s/%s", worksubdir, RPMBUILD_TOPDIR, subdirs[i]);
        assert(sub != NULL);

        if (access(sub, R_OK | W_OK | X_OK) == 0) {
            free(sub);
            continue;
        }

        if (!strcmp(subdirs[i], RPMBUILD_SOURCEDIR) || !strcmp(subdirs[i], RPMBUILD_SPECDIR)) {
            /* symlinks SOURCES and SPECS to where the SRPM is already extracted */
            if (symlink(shortname, sub) == -1) {
                warn("*** symlink");
            }
        } else {
            (void) mkdirp(sub, mode);
        }

        free(sub);
    }

    free(fp);
    return topdir;
}

/*
 * Given a spec file for a SRPM, do the equivalent of 'rpmbuild -bp'
 * to get an extracted and prepared source tree (e.g., patched).
 * Returns an allocated string containing the path to the rpmbuild
 * BUILD subdirectory.  The caller is responsible for freeing the
 * returned string.
 *
 * A NULL return value indicates a failure to prepare the source tree.
 */
static char *rpm_prep_source(struct rpminspect *ri, const rpmfile_entry_t *file, char **details)
{
    int pfd[2];
    pid_t proc = 0;
    int status = 0;
    FILE *reader = NULL;
    char *tail = NULL;
    size_t n = BUFSIZ;
    char *buf = NULL;
    rpmSpec spec = NULL;
    char *macro = NULL;
    rpmts ts = NULL;
    BTA_t ba = NULL;
    char *topdir = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* perform the %prep step in a subprocess to capture stdout/stderr */
    if (pipe(pfd) == -1) {
        warn("*** pipe");
        return NULL;
    }

    proc = fork();

    if (proc == 0) {
        /* connect the output */
        if (dup2(pfd[STDOUT_FILENO], STDOUT_FILENO) == -1 || dup2(pfd[STDOUT_FILENO], STDERR_FILENO) == -1) {
            warn("*** dup2");
            _exit(EXIT_FAILURE);
        }

        /* close pipes */
        if (close(pfd[STDIN_FILENO]) == -1 || close(pfd[STDOUT_FILENO]) == -1) {
            warn("*** close");
            _exit(EXIT_FAILURE);
        }

        setlinebuf(stdout);
        setlinebuf(stderr);

        /* define our top dir */
        topdir = make_source_dirs(ri->worksubdir, file->fullpath);
        assert(topdir != NULL);

        xasprintf(&macro, "_topdir %s", topdir);
        assert(macro != NULL);
        (void) rpmDefineMacro(NULL, macro, 0);
        free(macro);

        /* read in the spec file */
        spec = rpmSpecParse(file->fullpath, RPMBUILD_PREP | RPMSPEC_ANYARCH, topdir);
        free(topdir);

        if (spec == NULL) {
            warn("*** rpmSpecParse");

            if (close(STDOUT_FILENO) == -1 || close(STDERR_FILENO) == -1) {
                warn("*** close");
            }

            _exit(EXIT_FAILURE);
        }

        /* run through the %prep stage */
        ba = xalloc(sizeof(*ba));
        ba->buildAmount |= RPMBUILD_PREP;

        ts = rpmtsCreate();
        (void) rpmtsSetRootDir(ts, ri->worksubdir);
        rpmtsSetFlags(ts, rpmtsFlags(ts) | RPMTRANS_FLAG_NOPLUGINS | RPMTRANS_FLAG_NOSCRIPTS);

        /* normal noise level */
        rpmSetVerbosity(RPMLOG_NOTICE);

        /* try to perform the rpm %prep step */
#ifdef _HAVE_OLD_RPM_API
        if (rpmSpecBuild(spec, ba)) {
#else
        if (rpmSpecBuild(ts, spec, ba)) {
#endif
            status = 2;
        } else {
            status = EXIT_SUCCESS;
        }

        /* clean up and exit */
        rpmSpecFree(spec);
        rpmtsFree(ts);
        rpmFreeMacros(NULL);
        rpmFreeRpmrc();
        free(ba);

        if (close(STDOUT_FILENO) == -1 || close(STDERR_FILENO) == -1) {
            warn("*** close");
        }

        _exit(status);
    } else if (proc == -1) {
        /* failure */
        warn("*** fork");
    } else {
        /* close the unused part */
        if (close(pfd[STDOUT_FILENO]) == -1) {
            warn("*** close");
        }

        /* Read the child output back which would be what 'rpmbuild -bp' runs */
        reader = fdopen(pfd[STDIN_FILENO], "r");

        if (reader == NULL) {
            warn("*** fdopen");
            return NULL;
        }

        free(*details);
        *details = NULL;
        buf = xalloc(n);

        while (getline(&buf, &n, reader) != -1) {
            *details = strappend(*details, buf, NULL);
        }

        free(buf);

        /* remember that fclose() closes underlying file handles */
        if (fclose(reader) == -1) {
            warn("*** fclose");
        }

        /* wait for the child to exit */
        if (waitpid(proc, &status, 0) == -1) {
            warn("*** waitpid");
        }

        /* where unpacked sources can be found */
        xasprintf(&build, "%s/%s/%s", ri->worksubdir, RPMBUILD_TOPDIR, RPMBUILD_BUILDDIR);

        /* wipe the working directory if %prep failed */
        if (WEXITSTATUS(status) == 2) {
            (void) rmtree(build, true, true);
            free(build);
            build = NULL;
        }
    }

    /* trim trailing newlines from details */
    if (*details != NULL) {
        tail = rindex(*details, '\n');

        if (tail != NULL) {
            tail[strcspn(tail, "\n")] = 0;
        }

        *details = xrealloc(*details, strlen(*details) + 1);
    }

    return build;
}

/*
 * Given a spec file for a SRPM, manually unpack source archives and
 * uncompress files listed in the header.  This function is used if
 * rpm_prep_source() fails.  Returns an allocated string containing
 * the path to the rpmbuild BUILD subdirectory.  The caller is
 * responsible for freeing the returned string.
 *
 * A NULL return value indicates a failure to prepare the source tree.
 */
static char *manual_prep_source(struct rpminspect *ri, const rpmfile_entry_t *file)
{
    char *topdir = NULL;
    char *fp = NULL;
    char *srpmdir = NULL;
    char *srcfile = NULL;
    const char *mime = NULL;
    char *extractdir = NULL;
    string_list_t *sources = NULL;
    string_entry_t *entry = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* get the directory for the SRPM files */
    fp = strdup(file->fullpath);
    assert(fp != NULL);
    srpmdir = dirname(fp);
    assert(srpmdir != NULL);

    /* create extract location */
    topdir = make_source_dirs(ri->worksubdir, file->fullpath);
    assert(topdir != NULL);

    /* extract to the same location 'rpmbuild' would use */
    xasprintf(&build, "%s/%s", topdir, RPMBUILD_BUILDDIR);
    assert(build != NULL);
    free(topdir);

    /* iterate over a list of all the source files in the SRPM */
    sources = get_rpm_header_string_array(file->rpm_header, RPMTAG_SOURCE);

    if (sources != NULL && !TAILQ_EMPTY(sources)) {
        TAILQ_FOREACH(entry, sources, items) {
            xasprintf(&srcfile, "%s/%s", srpmdir, entry->data);
            assert(srcfile != NULL);

            /* get the MIME type of the source file */
            mime = mime_type(ri, srcfile);

            /* skip text files */
            if (strprefix(mime, "text/")) {
                free(srcfile);
                continue;
            }

            /* create a unique subdirectory for this source file */
            xasprintf(&extractdir, "%s/%s", build, UNPACK_TEMPLATE);
            assert(extractdir != NULL);
            extractdir = mkdtemp(extractdir);
            assert(extractdir != NULL);
            uses_unpack_base = true;

            /* try to unpack the file */
            if (unpack_archive(srcfile, extractdir, true)) {
                rmtree(extractdir, true, false);
            }

            free(extractdir);
            free(srcfile);
        }
    }

    free(fp);
    list_free(sources, free);
    return build;
}

/*
 * Returns true if the UChar is what we consider a line ending.
 *
 * This function contains code adapted from this blog post about
 * Unicode with the ICU library:
 *
 * https://begriffs.com/posts/2019-05-23-unicode-icu.html
 */
static bool end_of_line(const UChar c)
{
    if ((c >= 0xA && c <= 0xD) || c == 0x85 || c == 0x2028 || c == 0x2029) {
        return true;
    }

    return false;
}

/*
 * nftw() helper used to validate each source file.
 *
 * This function contains code adapted from this blog post about
 * Unicode with the ICU library:
 *
 * https://begriffs.com/posts/2019-05-23-unicode-icu.html
 *
 * NOTE: The global 'build' is used in this function, so make sure any
 * calls to free build are done after calls to validate_file().
 */
static int validate_file(const char *fpath, __attribute__((unused)) const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
    const char *localpath = fpath;
    UFILE *src = NULL;
    UChar32 c;
    UChar *line = NULL;
    UChar *line_new = NULL;
    size_t i = 0;
    size_t sz = BUFSIZ;
    UChar *needle = NULL;
    long int linenum = 0;
    long int colnum = 0;
    UChar32_entry_t *uentry = NULL;
    struct result_params params;

    assert(globalri != NULL);

    /* Only looking at regular files */
    if (tflag == FTW_D || tflag == FTW_DNR || tflag == FTW_DP || tflag == FTW_NS) {
        return 0;
    }

    /* check for exclusion by regular expression */
    if ((globalri->unicode_exclude != NULL) && (regexec(globalri->unicode_exclude, fpath, 0, NULL, 0) == 0)) {
        return 0;
    }

    /* check for exclusion by MIME type */
    if (is_excluded_mime_type(fpath)) {
        return 0;
    }

    /* check for exclusion by ignore list */
    if (build && strprefix(localpath, build)) {
        /*
         * this is a file in the prepared source tree, so trim the
         * build sub dir and make the path strings look like this:
         *
         *     rpminspect-1.47.0/lib/magic.c
         */
        localpath += strlen(build);

        /* trim the leading slash */
        while (*localpath == '/' && *localpath != '\0') {
            localpath++;
        }

        /*
         * for manual_prep_source() runs, also account for a potential
         * unpack-XXXXXX/ leading directory and trim that too
         */
        if (uses_unpack_base && strprefix(localpath, UNPACK_BASE)) {
            localpath += strlen(UNPACK_TEMPLATE);
        }
    } else if (root && strprefix(localpath, root)) {
        /* this is a source file directly in the SRPM */
        localpath += strlen(root);
    }

    if (localpath) {
        while (*localpath == '/' && *localpath != '\0') {
            localpath++;
        }
    }

    if (localpath == NULL) {
        warnx(_("*** empty localpath on %s"), fpath);
        return 0;
    }

    /* initialize reporting results */
    init_result_params(&params);
    params.header = NAME_UNICODE;
    params.arch = globalarch;
    params.file = localpath;
    params.noun = _("forbidden code point in ${FILE} on ${ARCH}");
    params.remedy = get_remedy(REMEDY_UNICODE);

    /* Read in the file as Unicode data */
    src = u_fopen(fpath, "r", NULL, NULL);

    if (src == NULL) {
        warn("*** u_fopen");
        return 0;
    }

    line = xcalloc(sz, sizeof(*line));
    linenum = 1;

    while (!u_feof(src)) {
        /* read in one whole line of text from the file */
        for (i = 0; (line[i] = u_fgetcx(src)) != U_EOF && !end_of_line(line[i]); i++) {
            /* increase the buffer size if necessary */
            if (i >= sz) {
                sz *= 2;
                errno = 0;
#ifdef _HAVE_REALLOCARRAY
                line_new = xreallocarray(line, sz, sizeof(*line));
#else
                line_new = xrealloc(line, sz * sizeof(*line));
#endif
                line = line_new;
            }
        }

        /* eat newline if terminated by a carriage return */
        if (line[i] == 0xD && (c = u_fgetcx(src)) != 0xA) {
            u_fungetc(c, src);
        }

        /* this is either U_EOF or newline, trim it */
        line[i] = '\0';

        /* check this line for any prohibited characters */
        TAILQ_FOREACH(uentry, forbidden, items) {
            needle = u_strchr32(line, uentry->data);

            /* forbidden code point found */
            if (needle != NULL) {
                /* build a pretend rpmfile_entry_t to look up the secrule */
                globalfile->localpath = strdup(localpath);
                assert(globalfile->localpath != NULL);

                /* get reporting severity */
                params.severity = get_secrule_result_severity(globalri, globalfile, SECRULE_UNICODE);

                /* this will be recycled as nftw() runs validate_file() */
                free(globalfile->localpath);
                globalfile->localpath = NULL;

                /* report result based on the secrule */
                if (params.severity != RESULT_NULL && params.severity != RESULT_SKIP) {
                    if (params.severity == RESULT_INFO) {
                        params.waiverauth = NOT_WAIVABLE;
                        params.verb = VERB_OK;
                    } else {
                        params.waiverauth = WAIVABLE_BY_SECURITY;
                        params.verb = VERB_FAILED;
                        globalresult = false;
                    }

                    colnum = u_strlen(line) - u_strlen(needle);
                    xasprintf(&params.msg, _("A forbidden code point, 0x%04X, was found in the %s source file on line %ld at column %ld.  This source file is used by %s."), *needle, localpath, linenum, colnum, globalspec);
                    add_result(globalri, &params);
                    free(params.msg);
                }
            }
        }

        /* advance the line counter */
        linenum++;
    }

    u_fclose(src);
    free(line);

    return 0;
}

static bool unicode_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool prepped = false;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);
    assert(ri->workdir != NULL);

    /* skip binary packages */
    if (!headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip files of explicitly excluded MIME types */
    if (is_excluded_mime_type(file->fullpath)) {
        return true;
    }

    /* for reporting results */
    globalarch = get_rpm_header_arch(file->rpm_header);
    assert(globalarch != NULL);

    globalfile = xalloc(sizeof(*globalfile));
    globalfile->rpm_header = file->rpm_header;
    assert(globalfile->rpm_header != NULL);

    /* initialize result parameters */
    init_result_params(&params);

    /* when the spec file is found, prepare the source tree and check each file there */
    if (strsuffix(file->localpath, SPEC_FILENAME_EXTENSION)) {
        /* for the spec file, examine each file in the prepared source tree */
        build = rpm_prep_source(ri, file, &params.details);

        if (build) {
            prepped = true;
        }

        /* try to fall back on unpacking archives manually */
        if (!prepped) {
            build = manual_prep_source(ri, file);

            if (build) {
                prepped = true;
                free(params.details);
                params.details = NULL;
            }
        }

        /* failure case where we can't prep the source tree or manually unpack archives */
        if (!prepped) {
            params.severity = RESULT_BAD;
            params.waiverauth = NOT_WAIVABLE;
            params.header = NAME_UNICODE;
            params.arch = globalarch;
            params.file = file->localpath;
            params.noun = _("unable to run %prep in ${FILE}");
            params.verb = VERB_FAILED;
            params.remedy = get_remedy(REMEDY_UNICODE_PREP_FAILED);
            xasprintf(&params.msg, _("Unable to run through the %%prep section in %s or manually unpack sources for further scanning."), file->localpath);
            add_result(globalri, &params);
            free(params.msg);
            free(params.details);

            seen = true;
            (void) rmtree(build, true, false);

            free(build);
            free(globalfile);

            build = NULL;
            globalfile = NULL;

            return false;
        }

        /* copy the name of the spec file */
        globalspec = file->localpath;
        assert(globalspec != NULL);

        /* our tree dive result is saved in 'globalresult', -1 here is an internal error */
        if (nftw(build, validate_file, FOPEN_MAX, FTW_MOUNT|FTW_PHYS) == -1) {
            warn("*** nftw");
        }

        seen = true;
        (void) rmtree(build, true, false);

        free(params.details);
    }

    /* check the individual file */
    (void) validate_file(file->fullpath, NULL, FTW_F, NULL);

    /* cleanup */
    free(globalfile);
    free(build);
    globalfile = NULL;
    build = NULL;

    return globalresult;
}

/*
 * Main driver for the 'unicode' inspection.
 */
bool inspect_unicode(struct rpminspect *ri)
{
    bool result = true;
    UChar32_entry_t *entry = NULL;
    string_entry_t *sentry = NULL;
    rpmpeer_entry_t *peer = NULL;
    rpmfile_entry_t *file = NULL;
    struct result_params params;

    assert(ri != NULL);

    /* only run if there are forbidden code points */
    if (ri->unicode_forbidden_codepoints != NULL && !TAILQ_EMPTY(ri->unicode_forbidden_codepoints)) {
        /* convert code points to UChar values */
        forbidden = xalloc(sizeof(*forbidden));
        TAILQ_INIT(forbidden);

        TAILQ_FOREACH(sentry, ri->unicode_forbidden_codepoints, items) {
            entry = xalloc(sizeof(*entry));
            errno = 0;
            entry->data = strtol(sentry->data, NULL, 16);

            if (errno == ERANGE || errno == EINVAL) {
                warn("*** strtol");
                free(entry);
                continue;
            }

            TAILQ_INSERT_TAIL(forbidden, entry, items);
        }

        /* so the nftw() helper can report results */
        globalri = ri;

        /* run the inspection */
        TAILQ_FOREACH(peer, ri->peers, items) {
            if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
                continue;
            }

            /* this line is why we can't use foreach_peer_file() here */
            root = peer->after_root;

            TAILQ_FOREACH(file, peer->after_files, items) {
                if (!unicode_driver(ri, file)) {
                    result = false;
                }
            }
        }

        /* free the forbidden list memory */
        while (!TAILQ_EMPTY(forbidden)) {
            entry = TAILQ_FIRST(forbidden);
            TAILQ_REMOVE(forbidden, entry, items);
            free(entry);
        }

        free(forbidden);
    }

    /* report */
    init_result_params(&params);
    params.header = NAME_UNICODE;
    params.verb = VERB_OK;

    if (result) {
        params.severity = RESULT_OK;
        add_result(ri, &params);
    } else if (!seen) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
        xasprintf(&params.msg, _("The unicode inspection is only for source packages, skipping."));
        add_result(ri, &params);
        free(params.msg);

        /*
         * There's no reason to fail this test for an informational message.
         */
        result = true;
    }

    return result;
}
