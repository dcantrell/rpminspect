/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <err.h>
#include <limits.h>
#include <rpm/rpmfi.h>

#include "rpminspect.h"

static bool reported = false;

static bool doc_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    rpmfileAttrs before_doc = 0;
    rpmfileAttrs after_doc = 0;
    char *diff_output = NULL;
    int exitcode;
    const char *name = NULL;
    const char *arch = NULL;
    struct result_params params;

    assert(ri != NULL);
    assert(file != NULL);

    /* no peer file, cannot compare...will be handled by "addedfiles" */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* only compare regular files and symlinks */
    if (S_ISDIR(file->st.st_mode) || S_ISCHR(file->st.st_mode) || S_ISBLK(file->st.st_mode) || S_ISFIFO(file->st.st_mode) || S_ISSOCK(file->st.st_mode)) {
        return true;
    }

    /*
     * rpm marks man pages with RPMFILE_DOC, but we check those in the
     * man page inspection.  Exclude them here.
     */
    if (process_file_path(file, ri->manpage_path_include, ri->manpage_path_exclude) ||
        process_file_path(file->peer_file, ri->manpage_path_include, ri->manpage_path_exclude)) {
        return true;
    }

    /* the package name is used for reporting */
    name = headerGetString(file->rpm_header, RPMTAG_NAME);

    /* skip debuginfo and debugsource packages */
    if (strsuffix(name, DEBUGINFO_SUFFIX) || strsuffix(name, DEBUGSOURCE_SUFFIX)) {
        return true;
    }

    /* the architecture is used in reporting */
    arch = get_rpm_header_arch(file->rpm_header);

    /* set up result parameters */
    init_result_params(&params);
    params.header = NAME_DOC;
    params.arch = arch;
    params.file = file->localpath;
    params.remedy = REMEDY_DOC;
    params.verb = VERB_CHANGED;
    params.noun = _("%doc ${FILE}");

    if (is_rebase(ri)) {
        params.severity = RESULT_INFO;
        params.waiverauth = NOT_WAIVABLE;
    } else {
        params.severity = RESULT_VERIFY;
        params.waiverauth = WAIVABLE_BY_ANYONE;
    }

    /* verify %doc values */
    before_doc |= file->peer_file->flags & RPMFILE_DOC;
    after_doc |= file->flags & RPMFILE_DOC;

    if (before_doc && after_doc) {
        /* compare the files */
        exitcode = filecmp(file->peer_file->fullpath, file->fullpath);

        if (exitcode) {
            /* the files differ, see if it's only whitespace changes */
            diff_output = get_file_delta(file->peer_file->fullpath, file->fullpath);

            /* always report content changes on %doc files as INFO */
            params.severity = RESULT_INFO;
            params.waiverauth = NOT_WAIVABLE;

            if (diff_output) {
                xasprintf(&params.msg, _("%%doc file content change for %s in %s on %s\n"), file->localpath, name, arch);
            }

            /* only add the diff output for text content */
            if (strprefix(get_mime_type(file->peer_file), "text/") && strprefix(get_mime_type(file), "text/")) {
                params.details = diff_output;
            }

            add_result(ri, &params);
            free(params.msg);
            free(diff_output);
            reported = true;
            result = true;
        }
    } else if (before_doc || after_doc) {
        xasprintf(&params.msg, _("%%doc file change for %s in %s on %s (%smarked as %%doc -> %smarked as %%doc)\n"), file->localpath, name, arch,
                  (before_doc ? "" : _("not ")), (after_doc ? "" : _("not ")));
        add_result(ri, &params);
        free(params.msg);
        result = !(params.severity >= RESULT_VERIFY);
        reported = true;
    }

    return result;
}

/*
 * Main driver for the 'doc' inspection.
 */
bool inspect_doc(struct rpminspect *ri)
{
    bool result = true;
    struct result_params params;

    assert(ri != NULL);

    result = foreach_peer_file(ri, NAME_DOC, doc_driver);

    if (result && !reported) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.header = NAME_DOC;
        add_result(ri, &params);
    }

    return result;
}
