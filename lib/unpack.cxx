/*
 * Copyright 2012 David Cantrell <david.l.cantrell@gmail.com>
 *                Chris Lumens <chris@bangmoney.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <archive.h>
#include <archive_entry.h>

#include "rpminspect.h"

/* Local prototypes */
static int copy_data(struct archive *, struct archive *);
static int extract_entry(struct archive *, struct archive *, struct archive_entry *);

/*
 * From:
 * https://github.com/libarchive/libarchive/wiki/Examples
 * Reads data from the input archive stream and writes it to the output
 * archive stream.
 */
static int copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buf = NULL;
    size_t s;
    off_t o;

    while ((r = archive_read_data_block(ar, &buf, &s, &o)) != ARCHIVE_EOF) {
        if (r != ARCHIVE_OK) {
            return r;
        }

        r = archive_write_data_block(aw, buf, s, o);

        if (r != ARCHIVE_OK) {
            warnx("*** archive_write_data_block: %s", archive_error_string(aw));
            return r;
        }
    }

    return ARCHIVE_OK;
}

/* Extract a single entry from an archive and write it to disk. */
static int extract_entry(struct archive *input, struct archive *output, struct archive_entry *entry)
{
    int r = 0;
    int ret = 0;

    r = archive_write_header(output, entry);

    if (r != ARCHIVE_OK) {
        warnx("*** archive_write_header: %s", archive_error_string(output));
        ret = -1;
    } else if (archive_entry_size(entry) > 0) {
        if (copy_data(input, output) != ARCHIVE_OK) {
            ret = -1;
        }

        if (r != ARCHIVE_OK) {
            warnx("*** archive_write_header: %s", archive_error_string(output));
        } else if (r < ARCHIVE_WARN) {
            ret = -1;
        }
    }

    r = archive_write_finish_entry(output);

    if (r != ARCHIVE_OK) {
        warnx("*** archive_write_finish_entry: %s", archive_error_string(output));
    } else if (r < ARCHIVE_WARN) {
        ret = -1;
    }

    return ret;
}

/*
 * Unpack an archive file to a destination directory.  The directory must
 * exist before calling this function.
 */
int unpack_archive(const char *archive, const char *dest, const bool force)
{
    int flags, r, ret = 0;
    char *rfilename = NULL;
    char cwd[PATH_MAX + 1];
    struct archive *input = NULL;
    struct archive *output = NULL;
    struct archive_entry *entry = NULL;

    assert(archive != NULL);
    assert(dest != NULL);
    memset(cwd, '\0', sizeof(cwd));

    /* attributes to restore */
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    if (force) {
        /* user passed a -f flag, so try to force past errors unpacking */
        flags |= ARCHIVE_EXTRACT_UNLINK;
    }

    /* full location to the archive */
    if ((rfilename = realpath(archive, rfilename)) == NULL) {
        if (errno == ENOENT) {
            return 0;
        } else {
            warn("*** realpath: %s", archive);
            return -1;
        }
    }

    /* archive reader */
    input = archive_read_new();
#if ARCHIVE_VERSION_NUMBER < 3000000
    archive_read_support_compression_all(input);
#else
    archive_read_support_filter_all(input);
#endif
    archive_read_support_format_all(input);
    r = archive_read_open_filename(input, archive, BUFSIZ);

    if (r != ARCHIVE_OK) {
        archive_read_free(input);
        return -1;
    }

    /* change to dest */
    if (getcwd(cwd, PATH_MAX) == NULL) {
        archive_read_free(input);
        err(RI_PROGRAM_ERROR, "*** getcwd");
    }

    if (chdir(dest) != 0) {
        warn("*** chdir");
        archive_read_free(input);
        return -1;
    }

    /* handler to write archive members to disk */
    output = archive_write_disk_new();
    archive_write_disk_set_options(output, flags);
    archive_write_disk_set_standard_lookup(output);

    /* extract each archive member */
    while ((r = archive_read_next_header(input, &entry)) != ARCHIVE_EOF) {
        if (r != ARCHIVE_OK) {
            warnx("*** archive_read_next_header: %s", archive_error_string(input));
        } else if (r < ARCHIVE_WARN) {
            ret = -1;
        }

        if (extract_entry(input, output, entry)) {
            ret = -1;
        }
    }

    archive_read_free(input);
#if ARCHIVE_VERSION_NUMBER < 3000000
    archive_write_finish(output);
#else
    archive_write_free(output);
#endif

    /* change back to original directory */
    if (chdir(cwd) != 0) {
        warn("*** chdir");
        return -1;
    }

    free(rfilename);
    return ret;
}
