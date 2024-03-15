/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <err.h>
#include <assert.h>

#include "rpminspect.h"

static bool reported = false;
static struct result_params params;

static void lost_alias(const char *alias, const string_list_t *before_modules, const string_list_t *after_modules, void *user_data)
{
    struct rpminspect *ri = (struct rpminspect *) user_data;
    string_entry_t *entry = NULL;

    assert(alias != NULL);
    assert(before_modules != NULL);
    assert(ri != NULL);

    params.remedy = get_remedy(REMEDY_KMOD_ALIAS);
    params.noun = _("${FILE} kernel module alias on ${ARCH}");

    TAILQ_FOREACH(entry, before_modules, items) {
        xasprintf(&params.msg, _("Kernel module '%s' lost alias '%s'"), entry->data, alias);
        params.verb = VERB_REMOVED;
        params.file = entry->data;
        add_result(ri, &params);
        free(params.msg);
        reported = true;
    }

    if (after_modules && !TAILQ_EMPTY(after_modules)) {
        TAILQ_FOREACH(entry, after_modules, items) {
            xasprintf(&params.msg, _("Kernel module '%s' gained alias '%s'"), entry->data, alias);
            params.verb = VERB_ADDED;
            params.file = entry->data;
            add_result(ri, &params);
            free(params.msg);
            reported = true;
        }
    }

    return;
}

static bool kmod_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    int err = -1;
    bool result_parm = true;
    bool result_deps = true;
    bool result_aliases = true;
    struct kmod_ctx *kctx = NULL;
    struct kmod_module *beforekmod = NULL;
    struct kmod_module *afterkmod = NULL;
    struct kmod_list *beforeinfo = NULL;
    struct kmod_list *afterinfo = NULL;
    const char *before_kmod_name = NULL;
    const char *after_kmod_name = NULL;
    kernel_alias_data_t *beforealiases = NULL;
    kernel_alias_data_t *afteraliases = NULL;
    string_list_t *lost = NULL;
    string_list_t *gain = NULL;
    string_entry_t *entry = NULL;
    const char *beforename = NULL;
    const char *aftername = NULL;
    const char *beforever = NULL;
    const char *afterver = NULL;

    assert(ri != NULL);
    assert(file != NULL);

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Skip debuginfo and debugsource packages */
    if (is_debuginfo_rpm(file->rpm_header) || is_debugsource_rpm(file->rpm_header)) {
        return true;
    }

    /* No peer file, skip */
    if (file->peer_file == NULL) {
        return true;
    }

    /* Only perform this inspection on regular files */
    if (!S_ISREG(file->st.st_mode)) {
        return true;
    }

    /* Skip files in the debug source path */
    if (strprefix(file->localpath, DEBUG_PATH)) {
        return true;
    }

    /* Restrict our view to files that match kernel module paths */
    if (!strstr(file->localpath, KERNEL_MODULES_DIR) || !strstr(file->localpath, KERNEL_MODULE_FILENAME_EXTENSION)) {
        /*
         * File is not in the known kernel module path or lacks the
         * expected extension (could be compressed too)
         */
        return true;
    }

    /* Set reporting conditions based on the package name and version */
    beforename = headerGetString(file->peer_file->rpm_header, RPMTAG_NAME);
    aftername = headerGetString(file->rpm_header, RPMTAG_NAME);
    beforever = headerGetString(file->peer_file->rpm_header, RPMTAG_VERSION);
    afterver = headerGetString(file->rpm_header, RPMTAG_VERSION);
    assert(beforename != NULL);
    assert(aftername != NULL);
    assert(beforever != NULL);
    assert(afterver != NULL);

    /* Read in the kernel modules */
    kctx = kmod_new(NULL, NULL);

    if (kctx == NULL) {
        warn("*** kmod_new");
        return true;
    }

    err = kmod_module_new_from_path(kctx, file->peer_file->fullpath, &beforekmod);

    if (err < 0) {
        /* not a kernel module */
        kmod_unref(kctx);
        return true;
    } else {
        before_kmod_name = kmod_module_get_name(beforekmod);
    }

    kmod_unref(kctx);
    kctx = kmod_new(NULL, NULL);

    if (kctx == NULL) {
        warn("*** kmod_new");
        return true;
    }

    err = kmod_module_new_from_path(kctx, file->fullpath, &afterkmod);

    if (err < 0) {
        /* not a kernel module */
        kmod_module_unref(beforekmod);
        kmod_unref(kctx);
        return true;
    } else {
        after_kmod_name = kmod_module_get_name(afterkmod);
    }

    /* Gather module parameters */
    err = kmod_module_get_info(beforekmod, &beforeinfo);

    if (err < 0) {
        warn("*** kmod_module_get_info");
        kmod_module_unref(beforekmod);
        kmod_module_unref(afterkmod);
        kmod_unref(kctx);
        return true;
    }

    err = kmod_module_get_info(afterkmod, &afterinfo);

    if (err < 0) {
        warn("*** kmod_module_get_info");
        kmod_module_info_free_list(beforeinfo);
        kmod_module_unref(beforekmod);
        kmod_module_unref(afterkmod);
        kmod_unref(kctx);
        return true;
    }

    /* Compute lost and gained module parameters */
    result_parm = compare_module_parameters(beforeinfo, afterinfo, &lost, &gain);

    /* Report parameters */
    if (!result_parm && lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            xasprintf(&params.msg, _("Kernel module %s removes parameter '%s' (was present in %s)."), file->localpath, entry->data, file->peer_file->localpath);
            params.remedy = get_remedy(REMEDY_KMOD_PARM);
            params.verb = VERB_REMOVED;
            params.noun = _("${FILE} kernel module parameter on ${ARCH}");
            params.file = file->localpath;
            params.arch = get_rpm_header_arch(file->rpm_header);
            add_result(ri, &params);
            free(params.msg);
            reported = true;
        }

        list_free(lost, free);
        lost = NULL;
    }

    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            xasprintf(&params.msg, _("Kernel module %s adds parameter '%s' (was not present in %s)."), file->localpath, entry->data, file->peer_file->localpath);
            params.remedy = NULL;
            params.verb = VERB_ADDED;
            params.noun = _("${FILE} kernel module parameter on ${ARCH}");
            params.file = file->localpath;
            params.arch = get_rpm_header_arch(file->rpm_header);
            add_result(ri, &params);
            free(params.msg);
            reported = true;
        }

        list_free(gain, free);
        gain = NULL;
    }

    /* Compute lost and gained module dependencies */
    result_deps = compare_module_dependencies(beforeinfo, afterinfo, &lost, &gain);

    /* Report dependencies */
    if (!result_deps && lost != NULL && !TAILQ_EMPTY(lost)) {
        TAILQ_FOREACH(entry, lost, items) {
            xasprintf(&params.msg, _("Kernel module %s removes dependency '%s' (was present in %s)."), file->localpath, entry->data, file->peer_file->localpath);
            params.remedy = get_remedy(REMEDY_KMOD_DEPS);
            params.verb = VERB_REMOVED;
            params.noun = _("${FILE} kernel module dependency on ${ARCH}");
            params.file = file->localpath;
            params.arch = get_rpm_header_arch(file->rpm_header);
            add_result(ri, &params);
            free(params.msg);
            reported = true;
        }

        list_free(lost, free);
        lost = NULL;
    }

    if (gain != NULL && !TAILQ_EMPTY(gain)) {
        TAILQ_FOREACH(entry, gain, items) {
            xasprintf(&params.msg, _("Kernel module %s adds dependency '%s' (was not present in %s)."), file->localpath, entry->data, file->peer_file->localpath);
            params.remedy = get_remedy(REMEDY_KMOD_DEPS);
            params.verb = VERB_ADDED;
            params.noun = _("${FILE} kernel module parameter");
            params.file = file->localpath;
            add_result(ri, &params);
            free(params.msg);
            reported = true;
        }

        list_free(gain, free);
        gain = NULL;
    }

    /* Compute lost PCI device IDs in kernel modules */
    beforealiases = gather_module_aliases(before_kmod_name, beforeinfo);
    afteraliases = gather_module_aliases(after_kmod_name, afterinfo);
    result_aliases = compare_module_aliases(beforealiases, afteraliases, lost_alias, ri);

    /* Clean up libkmod usage */
    kmod_module_info_free_list(beforeinfo);
    kmod_module_info_free_list(afterinfo);
    kmod_module_unref(beforekmod);
    kmod_module_unref(afterkmod);
    kmod_unref(kctx);

    /* Our own stuff */
    free_module_aliases(beforealiases);
    free_module_aliases(afteraliases);

    DEBUG_PRINT("result_parm=%d, result_deps=%d, result_aliases=%d\n", result_parm, result_deps, result_aliases);
    return true;
}

/*
 * Main driver for the 'kmod' inspection.
 */
bool inspect_kmod(struct rpminspect *ri)
{
    bool result;

    assert(ri != NULL);

    /* run the kmod inspection across all RPM files */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.waiverauth = NOT_WAIVABLE;
    params.header = NAME_KMOD;
    params.verb = VERB_OK;
    result = foreach_peer_file(ri, NAME_KMOD, kmod_driver);

    /* if everything was fine, just say so */
    if (result && !reported) {
        params.severity = RESULT_OK;
        params.verb = VERB_OK;
        add_result(ri, &params);
    }

    return result;
}
