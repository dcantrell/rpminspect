/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include <libgen.h>
#include <string.h>
#include <regex.h>
#include <rpm/rpmbuild.h>
#include <rpm/rpmmacro.h>
#include <rpm/rpmspec.h>
#include "rpminspect.h"

/*
 * Convert deprecated '%patchN' syntax to '%patch -P N'.  Recent
 * versions of rpm now error if you use the deprecated syntax.
 * rpminspect may be run on a newer platform to validate packages
 * built for an older platform that uses an older version of rpm.  For
 * now, convert the old style patch macro to one we know works on all
 * versions of rpm.
 *
 * Returns a string containing a path to the temporary file where the
 * filtered spec file was written.  The caller is responsible for
 * freeing this string as well as removing the file that string
 * references.
 */
static char *filter_spec_file(struct rpminspect *ri, const char *specfile)
{
    char *r = NULL;
    char *sfc = NULL;
    char *tmp = NULL;
    string_list_t *lines = NULL;
    string_entry_t *line = NULL;
    int fd = -1;
    FILE *fp = NULL;
    int reg_result = 0;
    regex_t filter_regex;
    char reg_error[BUFSIZ];
    bool filter = false;

    assert(ri != NULL);
    assert(specfile != NULL);

    /* read in the current spec file */
    lines = read_file(specfile);

    if (lines == NULL || TAILQ_EMPTY(lines)) {
        return strdup(specfile);
    }

    /* create an output file */
    sfc = strdup(specfile);
    assert(sfc != NULL);
    xasprintf(&tmp, "%s.XXXXXX", basename(sfc));
    assert(tmp != NULL);
    free(sfc);

    sfc = strdup(specfile);
    assert(sfc != NULL);
    r = joindelim(PATH_SEP, dirname(sfc), tmp, NULL);
    assert(r != NULL);
    free(sfc);
    free(tmp);

    fd = mkstemp(r);

    if (fd == -1) {
        warn("*** mkstemp");

        if (!debug_mode && unlink(r) == -1) {
            warn("*** unlink");
        }

        free(r);
        list_free(lines, free);
        return strdup(specfile);
    }

    fp = fdopen(fd, "w");

    if (fp == NULL) {
        warn("*** fdopen");

        if (close(fd) == -1) {
            warn("*** close");
        }

        if (!debug_mode && unlink(r) == -1) {
            warn("*** unlink");
        }

        free(r);
        list_free(lines, free);
        return strdup(specfile);
    }

    /* create the regular expression to filter with */
    reg_result = regcomp(&filter_regex, "^%patch[[:digit:]]+[[:space:]]*", REG_EXTENDED | REG_NEWLINE);

    if (reg_result != 0) {
        regerror(reg_result, &filter_regex, reg_error, sizeof(reg_error));
        warn("*** regcomp: %s", reg_error);
    } else {
        filter = true;
    }

    /* filter spec file */
    TAILQ_FOREACH(line, lines, items) {
        if (filter && (regexec(&filter_regex, line->data, 0, NULL, 0) == 0)) {
            tmp = strreplace(line->data, "%patch", "%patch -P ");
            assert(tmp != NULL);
            fprintf(fp, "%s\n", tmp);
            free(tmp);
        } else {
            fprintf(fp, "%s\n", line->data);
        }
    }

    regfree(&filter_regex);

    /* close things up */
    if (fclose(fp) == -1) {
        warn("*** fclose");

        if (!debug_mode && unlink(r) == -1) {
            warn("*** unlink");
        }

        free(r);
        r = strdup(specfile);
    }

    list_free(lines, free);
    return r;
}

/*
 * Given a spec file's full path, read it in and have librpm fully
 * parse it and expand macros and then return the resulting fully
 * parsed spec file text as a string_list_t that we can iterate over
 * line by line.  Caller is responsible for freeing the returned list.
 * Returns NULL on failure.
 */
string_list_t *read_spec(struct rpminspect *ri, const char *specfile)
{
    string_list_t *r = NULL;
    char *sfc = NULL;
    char *sd = NULL;
    char *dir = NULL;
    char *filtered = NULL;
    rpmSpec spec = NULL;
    const char *s = NULL;

    assert(specfile != NULL);

    /* get the spec file subdirectory */
    sfc = strdup(specfile);
    assert(sfc != NULL);
    sd = dirname(sfc);
    assert(sd != NULL);

    /* filter the spec file */
    filtered = filter_spec_file(ri, specfile);

    /*
     * For the purposes of our spec file parser, define the rpm
     * SOURCES and SPECS directories to be the same directory where
     * the spec file lives.  This is because of how rpminspect unpacks
     * the RPM so all files that would be in SOURCES are actually all
     * in the same directory as the spec file.
     */
    xasprintf(&dir, "_sourcedir %s", sd);
    assert(dir != NULL);

    if (rpmDefineMacro(rpmGlobalMacroContext, dir, 0) != 0) {
        warn("*** rpmDefineMacro");
    }

    free(dir);

    xasprintf(&dir, "_specdir %s", sd);
    assert(dir != NULL);

    if (rpmDefineMacro(rpmGlobalMacroContext, dir, 0) != 0) {
        warn("*** rpmDefineMacro");
    }

    free(dir);
    free(sfc);

    /* try to read and parse the spec file */
    spec = rpmSpecParse(filtered, RPMSPEC_NONE, NULL);

    if (!debug_mode && unlink(filtered) == -1) {
        warn("*** unlink");
    }

    free(filtered);

    if (spec == NULL) {
        warn("*** rpmSpecParse");
        return NULL;
    }

    /* get the fully expanded spec file */
    s = rpmSpecGetSection(spec, RPMBUILD_NONE);

    if (s == NULL) {
        warn("*** rpmSpecGetSection");
        rpmSpecFree(spec);
        return NULL;
    }

    /* split in to lines */
    r = strsplit(s, "\n\r");

    /* cleanup */
    rpmSpecFree(spec);

    return r;
}
