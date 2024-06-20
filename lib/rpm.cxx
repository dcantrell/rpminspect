/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <libgen.h>
#include <stdbool.h>
#include <assert.h>
#include <err.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/header.h>
#include <rpm/rpmmacro.h>
#include <archive.h>
#include <archive_entry.h>

#include "rpminspect.h"

/* Initialize librpm if needed */
int init_librpm(struct rpminspect *ri)
{
    int result;

    assert(ri != NULL);

    if (ri->librpm_initialized) {
        return RPMRC_OK;
    }

    rpmFreeMacros(NULL);
    rpmFreeRpmrc();
    result = rpmReadConfigFiles(NULL, NULL);
    ri->librpm_initialized = true;

    return result;
}

/* Return an RPM header struct for the given package filename. */
Header get_rpm_header(struct rpminspect *ri, const char *pkg)
{
    rpmts ts;
    FD_t fd;
    rpmRC result;
    char *head = NULL;
    char *headptr = NULL;
    char *bpkg = NULL;
    header_cache_t *hentry = NULL;

    assert(ri != NULL);
    assert(pkg != NULL);

    /* The cache stores the basename of the pkg */
    headptr = head = strdup(pkg);
    assert(headptr != NULL);
    bpkg = basename(head);

    /* First see if we can return the cached header */
    if (ri->header_cache != NULL) {
        HASH_FIND_STR(ri->header_cache, bpkg, hentry);

        if (hentry != NULL) {
            free(headptr);
            return hentry->hdr;
        }
    }

    /* No?  Then read the header in, cache it, and return it. */
    fd = Fopen(pkg, "r.ufdio");

    if (fd == NULL || Ferror(fd)) {
        warnx(_("*** Fopen failed for %s: %s"), pkg, Fstrerror(fd));

        if (fd) {
            Fclose(fd);
        }

        free(headptr);
        return NULL;
    }

    hentry = xalloc(sizeof(*hentry));
    hentry->pkg = strdup(bpkg);
    assert(hentry->pkg != NULL);

    ts = rpmtsCreate();
    rpmtsSetVSFlags(ts, _RPMVSF_NODIGESTS | _RPMVSF_NOSIGNATURES);
    result = rpmReadPackageFile(ts, fd, pkg, &hentry->hdr);
    rpmtsFree(ts);
    Fclose(fd);

    if (result != RPMRC_OK) {
        free(hentry->pkg);
        free(hentry);
        free(headptr);
        return NULL;
    }

    free(headptr);
    HASH_ADD_KEYPTR(hh, ri->header_cache, hentry->pkg, strlen(hentry->pkg), hentry);
    return hentry->hdr;
}

/*
 * Get and return the named RPM header tag as a string.
 */
char *get_rpmtag_str(Header hdr, rpmTagVal tag)
{
    char *val = NULL;
    rpmtd td = NULL;
    rpm_count_t td_size;

    /* no header means no tag value */
    if (hdr == NULL) {
        return NULL;
    }

    td = rpmtdNew();
    assert(td != NULL);

    /* NOTE: this function returns 1 for success, not RPMRC_OK */
    if (headerGet(hdr, tag, td, HEADERGET_MINMEM | HEADERGET_EXT) != 1) {
        goto val_cleanup;
    }

    td_size = rpmtdCount(td);

    if (td_size != 1) {
        goto val_cleanup;
    }

    val = strdup(rpmtdGetString(td));

val_cleanup:
    rpmtdFree(td);
    return val;
}

/*
 * Get the RPMTAG_NEVR extension tag.
 * NOTE: Caller must free this result.
 */
char *get_nevr(Header hdr)
{
    return get_rpmtag_str(hdr, RPMTAG_NEVR);
}

/*
 * Get the RPMTAG_NEVRA extension tag equivalent.  Do not use
 * RPMTAG_NEVRA directly here because on source RPMs, the "A" part of
 * NEVRA will have been written at rpmbuild time and does not report
 * "source" as the architecture but rather the architecture that you
 * ran rpmbuild on.
 * NOTE: Caller must free this result.
 */
char *get_nevra(Header hdr)
{
    char *r = NULL;

    r = get_nevr(hdr);
    assert(r != NULL);
    r = strappend(r, ".", get_rpm_header_arch(hdr), NULL);
    return r;
}

/*
 * Returns the RPMTAG_ARCH or "src" if it's a source RPM.
 * NOTE: Do not free() what this function returns.
 */
const char *get_rpm_header_arch(Header h)
{
    assert(h != NULL);

    if (headerIsSource(h)) {
        return SRPM_ARCH_NAME;
    } else {
        return headerGetString(h, RPMTAG_ARCH);
    }
}

/**
 * @brief Given an RPM header and a tag constant, retrieve the tag
 * value and add each array element as a member of a newly allocated
 * string_list_t.  The caller is responsible for freeing the returned
 * string_list_t.  The tag constant must be a string array in the RPM
 * header.  If the tag cannot be found or is empty, this function
 * returns NULL.
 *
 * @param hdr RPM header
 * @param tag RPM header tag constant (e.g., RPMTAG_SOURCE)
 * @return Newly allocated string_list_t containing tag values.
 */
string_list_t *get_rpm_header_string_array(Header hdr, rpmTagVal tag)
{
    string_list_t *list = NULL;
    rpmtd td = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    const char *val = NULL;

    if (hdr == NULL) {
        return NULL;
    }

    td = rpmtdNew();

    if (!headerGet(hdr, tag, td, flags)) {
        rpmtdFree(td);
        return NULL;
    }

    /* walk the tag and cram everything in to a list */
    while ((val = rpmtdNextString(td)) != NULL) {
        list = list_add(list, val);
    }

    rpmtdFree(td);
    return list;
}

/*
 * Helper function for functions below.  Create an rpmtd and position
 * the td index at the named file for the given header tag.  Return
 * the rpmtd.  Caller is responsible for freeing the rpmtd.
 */
static int _get_rpm_header_array_value_helper(rpmtd *td, const rpmfile_entry_t *file, rpmTag tag)
{
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;

    assert(td != NULL);
    assert(file != NULL);
    assert(file->idx >= 0);

    /* new header transaction */
    *td = rpmtdNew();

    /* find the header tag we want to extract values from */
    if (!headerGet(file->rpm_header, tag, *td, flags)) {
        rpmtdFree(*td);
        return -1;
    }

    /* set the array index */
    if (rpmtdSetIndex(*td, file->idx) == -1) {
        warn(_("*** file index %d is out of bounds for %s"), file->idx, file->fullpath);
        rpmtdFree(*td);
        return -1;
    }

    return 0;
}

/*
 * Given an RPM header tag, get that header tag array and return the
 * string that matches the index value for this file.  That's complex,
 * but some tags are arrays of strings (or ints) and what we need to
 * do is first get the array, then knowing the index entry for the file
 * we have, pull that array index out and return it.  NULL return means
 * an empty value or the tag was not present in the header.
 *
 * Limitations:
 * "tag" must refer to an s[] tag (see rpmtag.h from librpm)
 * "file" must have a usable array index value (idx)
 *
 * Returned value must be free'd by caller.
 */
char *get_rpm_header_string_array_value(const rpmfile_entry_t *file, rpmTag tag)
{
    rpmtd td = NULL;
    const char *val = NULL;
    char *ret = NULL;

    /* new header transaction */
    if (_get_rpm_header_array_value_helper(&td, file, tag) != 0) {
        return NULL;
    }

    /* get the tag we are looking for and copy the value */
    val = rpmtdGetString(td);

    if (val) {
        ret = strdup(val);
    }

    rpmtdFree(td);
    return ret;
}

/*
 * Given an RPM header tag, get that header tag array and return the
 * string that matches the index value for this file.  That's complex,
 * but some tags are arrays of strings (or ints) and what we need to
 * do is first get the array, then knowing the index entry for the file
 * we have, pull that array index out and return it.  NULL return means
 * an empty value or the tag was not present in the header.
 *
 * Limitations:
 * "tag" must refer to an h[] tag (see rpmtag.h from librpm)
 * "file" must have a usable array index value (idx)
 */
uint64_t get_rpm_header_num_array_value(const rpmfile_entry_t *file, rpmTag tag)
{
    rpmtd td = NULL;
    uint64_t ret = 0;

    /* new header transaction */
    if (_get_rpm_header_array_value_helper(&td, file, tag) != 0) {
        return 0;
    }

    /* get the tag we are looking for and copy the value */
    ret = rpmtdGetNumber(td);
    rpmtdFree(td);

    return ret;
}

/**
 * Given a path to an RPM package, extract the payload to a tar file
 * for later use with extract_rpm().  This happens in cases where
 * libarchive cannot detect the cpio stream in an opened RPM file.
 * The caller must free the returned path string.
 *
 * A lot of this is adapted from rpm2archive.c from the rpm sources.
 *
 * @param rpm The full path to the RPM.
 * @return Full path to the tar file containing the payload or NULL on
 *         error.
 */
#ifdef _HAVE_OLD_RPM_API
char *extract_rpm_payload(__attribute__((unused)) const char *rpm)
{
    /*
     * only support payload conversion with newer librpm releases
     * which include the rpmfiles.h and rpmarchive.h headers
     */

    return NULL;
#else
char *extract_rpm_payload(const char *rpm)
{
    char *payload = NULL;
    rpmts ts;
    rpmVSFlags vsflags = RPMVSF_MASK_NODIGESTS | RPMVSF_MASK_NOSIGNATURES | RPMVSF_NOHDRCHK;
    Header hdr = NULL;
    FD_t fdi = NULL;
    FD_t gzdi = NULL;
    const char *compr = NULL;
    char *rpmio_flags = NULL;
    rpmfiles files = NULL;
    rpmfi fi = NULL;
    struct archive *archive = NULL;
    struct archive_entry *entry = NULL;
    char *buf = NULL;
    char *hardlink = NULL;
    rpm_mode_t mode = 0;
    int nlink = 0;
    int rc = 0;
    const char *dn = NULL;
    char *filename = NULL;
    rpm_loff_t left;
    size_t len = 0;
    size_t read = 0;

    assert(rpm != NULL);

    /* create librpm widgets */
    ts = rpmtsCreate();
    rpmtsSetVSFlags(ts, vsflags);

    /* open the package */
    fdi = Fopen(rpm, "r.ufdio");
    rc = rpmReadPackageFile(ts, fdi, COMMAND_NAME, &hdr);

    if (rc == RPMRC_NOTFOUND || rc == RPMRC_FAIL) {
        warn("*** rpmReadPackageFile");
        goto cleanup;
    }

    /* determine how to read the payload */
    compr = headerGetString(hdr, RPMTAG_PAYLOADCOMPRESSOR);
    xasprintf(&rpmio_flags, "r.%s", compr ? compr : "gzip");
    assert(rpmio_flags != NULL);

    /* open the payload */
    gzdi = Fdopen(fdi, rpmio_flags);
    free(rpmio_flags);

    if (gzdi == NULL) {
        warnx("*** Fdopen: %s", Fstrerror(gzdi));
        goto cleanup;
    }

    files = rpmfilesNew(NULL, hdr, 0, RPMFI_KEEPHEADER);
    fi = rpmfiNewArchiveReader(gzdi, files, RPMFI_ITER_READ_ARCHIVE_CONTENT_FIRST);

    /* create a new archive with the payload data */
    archive = archive_write_new();

    if (archive_write_add_filter_gzip(archive) != ARCHIVE_OK) {
        warnx("*** archive_write_add_filter_gzip: %s", archive_error_string(archive));
        goto cleanup;
    }

    if (archive_write_set_format_pax_restricted(archive) != ARCHIVE_OK) {
        warnx("*** archive_write_set_format_pax_restricted: %s", archive_error_string(archive));
        goto cleanup;
    }

    xasprintf(&payload, "%s.tar", rpm);
    assert(payload != NULL);

    if (archive_write_open_filename(archive, payload) != ARCHIVE_OK) {
        warnx("*** archive_write_open_filename: %s", archive_error_string(archive));
        goto cleanup;
    }

    /* iterate over every entry in the payload */
    entry = archive_entry_new();
    buf = xalloc(BUFSIZ);

    while (rc >= 0) {
        rc = rpmfiNext(fi);

        if (rc == RPMERR_ITER_END) {
            break;
        }

        mode = rpmfiFMode(fi);
        nlink = rpmfiFNlink(fi);

        archive_entry_clear(entry);
        dn = rpmfiDN(fi);

        if (!strcmp(dn, "")) {
            dn = "/";
        }

        xasprintf(&filename, ".%s%s", dn, rpmfiBN(fi));
        assert(filename != NULL);
        archive_entry_copy_pathname(entry, filename);
        free(filename);

        archive_entry_set_size(entry, rpmfiFSize(fi));
        archive_entry_set_filetype(entry, mode & S_IFMT);
        archive_entry_set_perm(entry, mode);
        archive_entry_set_uname(entry, rpmfiFUser(fi));
        archive_entry_set_gname(entry, rpmfiFGroup(fi));
        archive_entry_set_rdev(entry, rpmfiFRdev(fi));
        archive_entry_set_mtime(entry, rpmfiFMtime(fi), 0);

        if (S_ISLNK(mode)) {
            archive_entry_set_symlink(entry, rpmfiFLink(fi));
        }

        if (nlink > 1) {
            if (rpmfiArchiveHasContent(fi)) {
                free(hardlink);
                hardlink = strdup(archive_entry_pathname(entry));
                assert(hardlink != NULL);
            } else {
                archive_entry_set_hardlink(entry, hardlink);
            }
        }

        archive_write_header(archive, entry);

        if (S_ISREG(mode) && (nlink == 1 || rpmfiArchiveHasContent(fi))) {
            left = rpmfiFSize(fi);

            while (left) {
                len = (left > BUFSIZ ? BUFSIZ : left);
                read = rpmfiArchiveRead(fi, buf, len);

                if (read == len) {
                    archive_write_data(archive, buf, len);
                } else {
                    warnx(_("*** error reading file from RPM payload"));
                    break;
                }

                left -= len;
            }
        }
    }

cleanup:
    free(hardlink);
    free(buf);
    Fclose(gzdi);
    archive_entry_free(entry);
    archive_write_close(archive);
    archive_write_free(archive);
    rpmfilesFree(files);
    rpmfiFree(fi);
    headerFree(hdr);
    rpmtsFree(ts);

    return payload;
#endif
}

static bool _is_debug_rpm_helper(Header hdr, const char *provide, const char *suffix)
{
    rpmtd req = NULL;
    rpmFlags flags = HEADERGET_MINMEM | HEADERGET_EXT | HEADERGET_ARGV;
    const char *p = NULL;
    const char *n = NULL;
    bool rn = false;
    bool rp = false;

    assert(hdr != NULL);

    /* look at the package name as well */
    if (suffix) {
        n = headerGetString(hdr, RPMTAG_NAME);

        if (strsuffix(n, suffix)) {
            rn = true;
        }
    }

    /* start new header transactions */
    if (provide) {
        req = rpmtdNew();

        if (headerGet(hdr, RPMTAG_PROVIDENAME, req, flags)) {
            /* collect all of the rules for this package */
            while (rpmtdNext(req) != -1) {
                p = rpmtdGetString(req);

                if (provide && !strcmp(p, provide)) {
                    rp = true;
                    break;
                }
            }
        }

        rpmtdFreeData(req);
        rpmtdFree(req);
    }

    return (rn || rp);
}

/*
 * Given the Header, return true if the package is a debuginfo
 * package, false otherwise.
 */
bool is_debuginfo_rpm(Header hdr)
{
    return _is_debug_rpm_helper(hdr, DEBUGINFO_PROVIDE, DEBUGINFO_SUFFIX);
}

/*
 * Given the Header, return true if the package is a debugsource
 * package, false otherwise.
 */
bool is_debugsource_rpm(Header hdr)
{
    return _is_debug_rpm_helper(hdr, NULL, DEBUGSOURCE_SUFFIX);
}
