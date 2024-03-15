/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>

#include <rpm/header.h>
#include <rpm/rpmtag.h>

#ifdef MANDOC_INCLUDE_SUBDIR
#include <mandoc/man.h>
#include <mandoc/mandoc.h>
#else
#include <man.h>
#include <mandoc.h>
#endif

/* Required for libmandoc >= 1.14.5 */
#ifdef NEWLIBMANDOC
#ifdef MANDOC_INCLUDE_SUBDIR
#include <mandoc/roff.h>
#include <mandoc/mandoc_parse.h>
#else
#include <roff.h>
#include <mandoc_parse.h>
#endif
#endif

#include "rpminspect.h"

static FILE *error_stream = NULL;
static regex_t sections_regex;

/* Old API used an error message callback */
#ifndef NEWLIBMANDOC
static void error_handler(enum mandocerr errtype, enum mandoclevel level,
                          const char *file, int line, int col, const char *msg)
{
    fprintf(error_stream, _("Error parsing %s:%d:%d: %s: %s: %s\n"), basename(file), line, col, mparse_strlevel(level), mparse_strerror(errtype), msg);
}
#endif

/* Free the memory used by mandoc */
static void inspect_manpage_free(void)
{
    mchars_free();
    regfree(&sections_regex);
}

/* Allocate memory used by inspect_manpage */
static bool inspect_manpage_alloc(void)
{
    int reg_result;
    char reg_error[BUFSIZ];
    char *tmp = NULL;

    mchars_alloc();

    /* extract the directory section to match 1, and the filename section to match 2.
     * For the directory section, look for /man<section>
     * For the filename section, look for <name>.<section>.gz
     */
    xasprintf(&tmp, "/man([^/]+)/[^/]+\\.([^.]+)\\%s$", GZIPPED_FILENAME_EXTENSION);
    reg_result = regcomp(&sections_regex, tmp, REG_EXTENDED);
    free(tmp);
    if (reg_result != 0) {
        regerror(reg_result, &sections_regex, reg_error, sizeof(reg_error));
        warnx(_("*** unable to compile man page path regular expression: %s"), reg_error);
        inspect_manpage_free();
        return false;
    }

    return true;
}

/*
 * Check that a man page is in the correct directory for its section.
 * The directory section (/usr/share/man/man<section>) must be a prefix
 * of the filename section (manpage.<section>[.gz]). The filename section
 * can include additional trailing characters. e.g., man1/x509.1ssl.gz is valid.
 * man1x/imake.1.gz is not.
 */
static bool inspect_manpage_path(const char *path)
{
    /* 0 is the whole match, 1 is the directory section, 2 is the filename section */
    regmatch_t section_matches[3];

    int pos;
    char directory_section[128];
    char filename_section[128];

    /* If there was no match, or if the match is bigger than our buffer,
     * assume something is wrong with the path and return false.
     */
    if (regexec(&sections_regex, path, 3, section_matches, 0) != 0) {
        return false;
    }

    if (section_matches[1].rm_so < 0) {
        return false;
    }

    if (section_matches[2].rm_so < 0) {
        return false;
    }

    if ((unsigned int) (section_matches[1].rm_eo - section_matches[1].rm_so) > sizeof(directory_section)) {
        return false;
    }

    if ((unsigned int) (section_matches[2].rm_eo - section_matches[2].rm_so) > sizeof(directory_section)) {
        return false;
    }

    pos = section_matches[1].rm_eo - section_matches[1].rm_so;
    snprintf(directory_section, sizeof(directory_section), "%.*s", pos, path + section_matches[1].rm_so);

    pos = section_matches[2].rm_eo - section_matches[2].rm_so;
    snprintf(filename_section, sizeof(filename_section), "%.*s", pos, path + section_matches[2].rm_so);

    return strprefix(filename_section, directory_section);
}

/*
 * Validate a man page file by parsing it with mandoc. Additionally check that
 * the man page is compressed.
 *
 * Returns NULL on success, otherwise returns an error message as a string.
 * The returned string is statically allocated.
 */
static char *inspect_manpage_validity(const char *path, const char *localpath)
{
    struct mparse *parser = NULL;
    int fd = -1;
    char magic[2];
    int parseopts = MPARSE_UTF8 | MPARSE_LATIN1;
#ifndef NEWLIBMANDOC
    struct roff_man *man = NULL;
    enum mandoclevel result_tmp;
    enum mandoclevel result = MANDOCLEVEL_OK;
#endif

    /* Use a nested function to gather error messages, since the callback doesn't
     * take any parameters that would tie it back to this call's context
     */
    char *error_buffer = NULL;
    size_t error_buffer_size = 0;

    /* Initialize the error buffer */
    error_stream = open_memstream(&error_buffer, &error_buffer_size);
    assert(error_stream != NULL);
#ifdef NEWLIBMANDOC
    mandoc_msg_setoutfile(error_stream);
#endif

    /* Allocate a new manpage parsing context */
#ifdef NEWLIBMANDOC
    parser = mparse_alloc(parseopts | MPARSE_VALIDATE, MANDOC_OS_OTHER, NULL);
#else
    parser = mparse_alloc(parseopts, MANDOCERR_ERROR, error_handler, MANDOC_OS_OTHER, NULL);
#endif
    assert(parser != NULL);

    /* Open the file */
    if ((fd = mparse_open(parser, path)) == -1) {
        fprintf(error_stream, _("Unable to open man page %s\n"), path);
        goto end;
    }

    /* Ensure the file is compressed. The file *should* end in .gz, and if it
     * does make sure that it's actually gzipped.
     */
    if (!strsuffix(path, GZIPPED_FILENAME_EXTENSION)) {
        fprintf(error_stream, _("Man page %s does not end in %s\n"), path, GZIPPED_FILENAME_EXTENSION);
    } else {
        if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            fprintf(error_stream, "read: %s\n", strerror(errno));
            goto end;
        }

        if (magic[0] != '\x1F' || magic[1] != '\x8B') {
            fprintf(error_stream, _("man page with %s suffix is not really compressed with gzip\n"), GZIPPED_FILENAME_EXTENSION);
        }

        /* Reset the fd and continue */
        if (lseek(fd, 0, SEEK_SET) == -1) {
            fprintf(error_stream, "lseek: %s\n", strerror(errno));
            goto end;
        }
    }

    mparse_reset(parser);

    /* Parse the file */
#ifdef NEWLIBMANDOC
    mparse_readfd(parser, fd, path);
#else
    result_tmp = mparse_readfd(parser, fd, path);
    if (result_tmp > result) {
        result = result_tmp;
    }
#endif

    /* Retrieve the syntax tree */
#ifdef NEWLIBMANDOC
    mparse_result(parser);
#else
    mparse_result(parser, &man, NULL);

    /* Validate the man page */
    if (man != NULL) {
        man_validate(man);
    }
#endif

    /* Check for validation errors */
#ifdef NEWLIBMANDOC
    if (mandoc_msg_getrc() > MANDOCLEVEL_OK) {
#else
    mparse_updaterc(parser, &result);
    if (result > MANDOCLEVEL_OK) {
#endif
        fprintf(error_stream, _("Errors found validating %s\n"), (localpath == NULL) ? path : localpath);
    }

end:
    mparse_free(parser);

    if (fd != -1) {
        close(fd);
    }

    fclose(error_stream);

    /* If there were no errors, return NULL */
    if ((error_buffer == NULL) || (error_buffer[0] == '\0')) {
        free(error_buffer);
        return NULL;
    }

    return error_buffer;
}

static bool manpage_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    int r = 0;
    char *uncompressed_man_page = NULL;
    struct stat sb;
    char *manpage_errors;
    bool result = true;
    const char *pkg = NULL;
    struct result_params params;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Is this a man page? */
    if (!file->fullpath || !S_ISREG(file->st.st_mode)) {
        return true;
    }

    if (!process_file_path(file, ri->manpage_path_include, ri->manpage_path_exclude)) {
        return true;
    }

    /* the package name is used for reporting */
    pkg = headerGetString(file->rpm_header, RPMTAG_NAME);

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = NAME_MANPAGE;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;
    params.verb = VERB_FAILED;

    /* check for empty man pages */
    uncompressed_man_page = uncompress_file(ri, file->fullpath, NAME_MANPAGE);
    if (uncompressed_man_page != NULL) {
        r = stat(uncompressed_man_page, &sb);

        if (r == 0 && sb.st_size == 0) {
            xasprintf(&params.msg, _("Man page %s is possibly empty on %s in %s"), file->localpath, params.arch, pkg);
            params.remedy = get_remedy(REMEDY_MAN_ERRORS);
            params.details = NULL;
            params.noun = _("empty man page ${FILE} on ${ARCH}");
            add_result(ri, &params);
            result = false;
            free(params.msg);
        } else if (r == -1) {
            warn("*** stat");
        }

        free(uncompressed_man_page);
    }

    /* check man page validity */
    if ((manpage_errors = inspect_manpage_validity(file->fullpath, file->localpath)) != NULL) {
        xasprintf(&params.msg, _("Man page checker reported problems with %s on %s in %s"), file->localpath, params.arch, pkg);
        params.remedy = get_remedy(REMEDY_MAN_ERRORS);
        params.details = manpage_errors;
        params.noun = _("man page ${FILE} on ${ARCH} has errors");
        add_result(ri, &params);
        free(params.msg);
        free(manpage_errors);
        result = false;
    }

    /* check man page location on the filesystem */
    if (!inspect_manpage_path(file->fullpath)) {
        xasprintf(&params.msg, _("Man page %s has incorrect path on %s in %s"), file->localpath, params.arch, pkg);
        params.remedy = get_remedy(REMEDY_MAN_PATH);
        params.details = NULL;
        params.noun = _("man page ${FILE} on ${ARCH} has incorrect path");
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    return result;
}

bool inspect_manpage(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    inspect_manpage_alloc();
    result = foreach_peer_file(ri, NAME_MANPAGE, manpage_driver);
    inspect_manpage_free();

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_MANPAGE;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
