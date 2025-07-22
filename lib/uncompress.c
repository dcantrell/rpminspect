/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <archive.h>
#include "rpminspect.h"

/*
 * Create a temporary file containing the uncompressed contents of the
 * specified file.  If the file is not compressed, this function just
 * duplicates it over to the temporary file location.  The optional
 * subdir may specify a subdirectory in worksubdir where the temporary
 * file should go (the directory will be created if necessary).
 * Useful for files that may or may not be compressed but if they are
 * we want to decompress them first and look at the uncompressed
 * output for changes.  The caller must both unlink the created output
 * file and free the returned string (which is the path to the created
 * output file).
 */
char *uncompress_file(struct rpminspect *ri, const char *infile, const char *subdir)
{
    int fd = -1;
    FILE *fp = NULL;
    struct stat sb;
    char *outfile = NULL;
    char *base = NULL;
    static int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    int r = -1;
    size_t size = 0;
    void *buf[BUFSIZ];
    struct archive *input = NULL;
    struct archive_entry *entry = NULL;

    assert(ri != NULL);
    assert(ri->workdir != NULL);
    assert(infile != NULL);

    /* subdirectory where the output file goes */
    if (subdir == NULL) {
        outfile = strdup(ri->worksubdir);
    } else {
        xasprintf(&outfile, "%s/%s", ri->worksubdir, subdir);
    }

    assert(outfile != NULL);

    /* see if the output directory exists */
    errno = 0;
    r = stat(outfile, &sb);

    if ((r == -1) && (errno != ENOENT)) {
        warn("*** stat");
        goto error1;
    }

    /*
     * only create the subdirectory if we need it (avoids lots of stat
     * calls later)
     */
    if (errno == ENOENT) {
        if (mkdirp(outfile, mode) == -1) {
            goto error1;
        }
    }

    free(outfile);

    /* create the output file that will be uncompressed */
    base = rindex(infile, PATH_SEP) + 1;
    assert(base != NULL);

    if (subdir == NULL) {
        xasprintf(&outfile, "%s/%s.XXXXXX", ri->worksubdir, base);
    } else {
        xasprintf(&outfile, "%s/%s/%s.XXXXXX", ri->worksubdir, subdir, base);
    }

    assert(outfile != NULL);

    fd = mkstemp(outfile);

    if (fd == -1) {
        warn("*** mkstemp");
        goto error1;
    }

    /*
     * Read in the input file and uncompress it if necessary.  The
     * uncompressed data is written to the output file we created and
     * that is used for later diff(1) calls.  Use libarchive here so
     * we can handle a wide range of compression formats.
     */
    input = archive_read_new();

    /* initialize only compression filters in libarchive */
#if ARCHIVE_VERSION_NUMBER < 3000000
#ifdef ARCHIVE_COMPRESSION_BZIP2
    archive_read_support_compression_bzip2(input);
#endif
#ifdef ARCHIVE_COMPRESSION_COMPRESS
    archive_read_support_compression_compress(input);
#endif
#ifdef ARCHIVE_COMPRESSION_GZIP
    archive_read_support_compression_gzip(input);
#endif
#ifdef ARCHIVE_COMPRESSION_GRZIP
    archive_read_support_compression_grzip(input);
#endif
#ifdef ARCHIVE_COMPRESSION_LRZIP
    archive_read_support_compression_lrzip(input);
#endif
#ifdef ARCHIVE_COMPRESSION_LZ4
    archive_read_support_compression_lz4(input);
#endif
#ifdef ARCHIVE_COMPRESSION_LZMA
    archive_read_support_compression_lzma(input);
#endif
#ifdef ARCHIVE_COMPRESSION_LZOP
    archive_read_support_compression_lzop(input);
#endif
#ifdef ARCHIVE_COMPRESSION_XZ
    archive_read_support_compression_xz(input);
#endif
#ifdef ARCHIVE_COMPRESSION_NONE
    archive_read_support_compression_none(input);
#endif
#else /* ARCHIVE_VERSION_NUMBER */
#ifdef ARCHIVE_FILTER_BZIP2
    archive_read_support_filter_bzip2(input);
#endif
#ifdef ARCHIVE_FILTER_COMPRESS
    archive_read_support_filter_compress(input);
#endif
#ifdef ARCHIVE_FILTER_GZIP
    archive_read_support_filter_gzip(input);
#endif
#ifdef ARCHIVE_FILTER_GRZIP
    archive_read_support_filter_grzip(input);
#endif
#ifdef ARCHIVE_FILTER_LRZIP
    archive_read_support_filter_lrzip(input);
#endif
#ifdef ARCHIVE_FILTER_LZ4
    archive_read_support_filter_lz4(input);
#endif
#ifdef ARCHIVE_FILTER_LZMA
    archive_read_support_filter_lzma(input);
#endif
#ifdef ARCHIVE_FILTER_LZOP
    archive_read_support_filter_lzop(input);
#endif
#ifdef ARCHIVE_FILTER_XZ
    archive_read_support_filter_xz(input);
#endif
#ifdef ARCHIVE_FILTER_NONE
    archive_read_support_filter_none(input);
#endif
#endif /* ARCHIVE_VERSION_NUMBER */

    /*
     * add raw and empty to account for uncompressed files and
     * compressed empty files
     */
    archive_read_support_format_raw(input);
    archive_read_support_format_empty(input);

    /* open the input file, decompress, and write to output */
    r = archive_read_open_filename(input, infile, BUFSIZ);

    if (r != ARCHIVE_OK) {
        /* just stop trying to uncompress if this errors */
        goto error2;
    }

    r = archive_read_next_header(input, &entry);

    if (r == ARCHIVE_WARN || r == ARCHIVE_FAILED || r == ARCHIVE_FATAL) {
        warn("*** archive_read_next_header: %s", archive_error_string(input));
        goto error2;
    }

    if (r == ARCHIVE_OK) {
        while (1) {
            size = archive_read_data(input, buf, BUFSIZ);

            if (size == 0) {
                break;
            }

            if (write(fd, buf, size) == -1) {
                warn("*** write");
                goto error2;
            }
        }
    } else {
        /* we hit an EOF or non-existent file, write an empty file */
        fp = fdopen(fd, "w");

        if (fp == NULL) {
            warn("*** fdopen");
            goto error2;
        }

        /*
         * NOTE: calling fclose() closes the underlying file
         * descriptor, so after this call make no calls to close() on
         * the fd
         */
        if (fclose(fp) == -1) {
            warn("*** fclose");
            goto error2;
        }

        fd = 0;
    }

    archive_read_free(input);

    /* close up our uncompressed file */
    if (fd && close(fd) == -1) {
        warn("*** close");
        goto error1;
    }

    return outfile;

error2:
    close(fd);

error1:
    free(outfile);
    return NULL;
}
