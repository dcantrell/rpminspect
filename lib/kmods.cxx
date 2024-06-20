/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef _LIBKMOD_HEADER_SUBDIR
#include <kmod/libkmod.h>
#else
#include <libkmod.h>
#endif /* _LIBKMOD_HEADER_SUBDIR */

#include "queue.h"
#include "uthash.h"
#include "inspect.h"
#include "rpminspect.h"

/* Filter a libkmod list and convert to a string_list_t */
static string_list_t * modinfo_to_list(const struct kmod_list *list, modinfo_to_entries convert)
{
    const struct kmod_list *iter = NULL;
    string_list_t *result;

    result = xalloc(sizeof(*result));
    TAILQ_INIT(result);

    kmod_list_foreach(iter, list) {
        convert(result, iter);
    }

    return result;
}

/* Helper for compare_parameters */
static void convert_module_parameters(string_list_t *list, const struct kmod_list *modinfo)
{
    const char *key;
    const char *value;
    char *tmp;
    string_entry_t *entry;

    key = kmod_module_info_get_key(modinfo);

    if (strcmp(key, "parmtype")) {
        return;
    }

    entry = xalloc(sizeof(*entry));

    /* The value is of the form <name>:<description>. Drop the description */
    value = kmod_module_info_get_value(modinfo);
    tmp = strchr(value, ':');

    DEBUG_PRINT("found '%s' parameter with value '%s'\n", key, value);

    if (tmp == NULL) {
        entry->data = strdup(value);
        assert(entry->data != NULL);
    } else {
        xasprintf(&entry->data, "%.*s", (int) (tmp - value), value);
    }

    TAILQ_INSERT_TAIL(list, entry, items);
}

static void convert_module_dependencies(string_list_t *list, const struct kmod_list *modinfo)
{
    const char *key = NULL;
    const char *value = NULL;
    char *value_copy = NULL;
    char *value_iter = NULL;
    char *token = NULL;

    key = kmod_module_info_get_key(modinfo);

    if (strcmp(key, "depends") && strcmp(key, "softdep")) {
        return;
    }

    value = kmod_module_info_get_value(modinfo);

    if ((value == NULL) || (*value == '\0')) {
        return;
    }

    DEBUG_PRINT("found '%s' dependency with value '%s'\n", key, value);

    /* The value is a comma-separated list of dependencies. Break it up into individual entries. */
    value_copy = strdup(value);
    assert(value_copy != NULL);
    value_iter = value_copy;

    while ((token = strsep(&value_iter, ",")) != NULL) {
        list = list_add(list, token);
    }

    free(value_copy);
    return;
}

/*
 * Compare two kernel modules to see if the after module lost
 * parameters.
 *
 * The before and after lists must be module info lists returned by
 * kmod_module_get_info.
 *
 * If after did not lose any parameters, returns true. If after lost
 * parameters, returns false, and populates "lost" with a list of the
 * missing parameters.  The "gain" list is just for informational
 * purposes and will never trigger a false return.  Only lost params
 * cause a false return.
 */
bool compare_module_parameters(const struct kmod_list *before, const struct kmod_list *after, string_list_t **lost, string_list_t **gain)
{
    string_list_t *before_parm_list = NULL;
    string_list_t *after_parm_list = NULL;
    string_list_t *difference = NULL;
    string_list_t *added = NULL;
    string_list_t *combined = NULL;
    bool result = true;

    assert(before);
    assert(after);
    assert(lost);

    /* Get the parameter list for each module */
    DEBUG_PRINT("before module\n");
    before_parm_list = modinfo_to_list(before, convert_module_parameters);
    DEBUG_PRINT("after module\n");
    after_parm_list = modinfo_to_list(after, convert_module_parameters);

    /* parameters present in both modules */
    combined = list_intersection(before_parm_list, after_parm_list);

    /* diff the parameter lists to get lost parameters */
    difference = list_difference(before_parm_list, after_parm_list);

    /* gather any new parameters */
    added = list_difference(after_parm_list, combined);

    /* If the lists are empty, everything is fine.
     * Otherwise, make a copy of difference so we can clean everything up
     */
    if (difference == NULL || TAILQ_EMPTY(difference)) {
        DEBUG_PRINT("no kernel module param differences\n");
        result = true;
    } else {
        DEBUG_PRINT("there are module param differences\n");
        result = false;
        *lost = list_copy(difference);
    }

    if (added != NULL && !TAILQ_EMPTY(added)) {
        DEBUG_PRINT("there are added module params\n");
        *gain = list_copy(added);
    }

    list_free(added, NULL);
    list_free(difference, NULL);
    list_free(before_parm_list, free);
    list_free(after_parm_list, free);
    list_free(combined, free);

    return result;
}

/*
 * Compare two kernel modules to see if the dependencies changed.
 *
 * Any change in dependencies is considered bad. If dependencies
 * changed, the function will return false, and the "before_deps" and
 * "after_deps" parameters will be populated with the dependencies
 * found for the given modules.
 */
bool compare_module_dependencies(const struct kmod_list *before, const struct kmod_list *after, string_list_t **before_deps, string_list_t **after_deps)
{
    string_list_t *before_depends_list;
    string_list_t *after_depends_list;
    string_list_t *difference;

    assert(before);
    assert(after);
    assert(before_deps);
    assert(after_deps);

    DEBUG_PRINT("before module\n");
    before_depends_list = modinfo_to_list(before, convert_module_dependencies);
    DEBUG_PRINT("after module\n");
    after_depends_list = modinfo_to_list(after, convert_module_dependencies);

    difference = list_symmetric_difference(before_depends_list, after_depends_list);

    /* If the list is empty, everything is fine, free everything. */
    if (difference == NULL || TAILQ_EMPTY(difference)) {
        DEBUG_PRINT("no kernel module deps differences\n");

        list_free(difference, NULL);
        list_free(before_depends_list, free);
        list_free(after_depends_list, free);

        return true;
    }

    /* Otherwise, just free the difference, and return the before and after dependencies */
    DEBUG_PRINT("there are kernel module deps differences\n");
    list_free(difference, NULL);

    *before_deps = before_depends_list;
    *after_deps = after_depends_list;

    return false;
}

/*
 * Add a module's alias information to a kernel_alias_data collection.
 * The returned kernel_alias_data_t must be freed with free_module_aliases.
 */
kernel_alias_data_t *gather_module_aliases(const char *module_name, const struct kmod_list *modinfo_list)
{
    kernel_alias_data_t *r = NULL;
    kernel_alias_data_t *kentry = NULL;
    const struct kmod_list *iter = NULL;
    const char *key = NULL;
    const char *value = NULL;

    assert(module_name != NULL);
    assert(modinfo_list != NULL);

    kmod_list_foreach(iter, modinfo_list) {
        /* Only gather pci aliases */
        key = kmod_module_info_get_key(iter);

        if (strcmp(key, "alias") != 0) {
            continue;
        }

        value = kmod_module_info_get_value(iter);

        if (!strprefix(value, "pci:")) {
            continue;
        }

        HASH_FIND_STR(r, value, kentry);

        if (kentry == NULL) {
            kentry = xalloc(sizeof(*kentry));

            kentry->alias = strdup(value);
            assert(kentry->alias != NULL);

            kentry->modules = list_add(kentry->modules, module_name);

            HASH_ADD_KEYPTR(hh, r, kentry->alias, strlen(kentry->alias), kentry);
        } else {
            kentry->modules = list_add(kentry->modules, module_name);
        }
    }

    return r;
}

/* Free the kernel_alias_data struct created by gather_module_aliases */
void free_module_aliases(kernel_alias_data_t *data)
{
    kernel_alias_data_t *entry = NULL;
    kernel_alias_data_t *tmp_entry = NULL;

    if (data == NULL) {
        return;
    }

    HASH_ITER(hh, data, entry, tmp_entry) {
        HASH_DEL(data, entry);
        list_free(entry->modules, free);
        free(entry->alias);
        free(entry);
    }

    free(data);
    return;
}

static string_list_t *wildcard_alias_search(const char *alias, kernel_alias_data_t *data)
{
    string_list_t *r = NULL;
    string_entry_t *iter = NULL;
    kernel_alias_data_t *kentry = NULL;
    kernel_alias_data_t *tmp_kentry = NULL;

    assert(alias != NULL);
    assert(data != NULL);

    HASH_ITER(hh, data, kentry, tmp_kentry) {
        if (fnmatch(kentry->alias, alias, 0) == 0) {
            TAILQ_FOREACH(iter, kentry->modules, items) {
                r = list_add(r, iter->data);
            }
        }
    }

    return r;
}

/*
 * For each module alias in "before", ensure that the alias is
 * provided by the same modules in "after".  If "after" lost
 * providers, call the provided callback with the before and after
 * lists, and return false.
 *
 * Module aliases use glob-style wildcards, so not all changes in
 * strings are a regression. For example, in 2.6.25, cxgb3 changed all
 * of its sub-device values to '*', so
 * "pci:v00001425d00000020sv*sd00000001bc*sc*i*" became
 * "pci:v00001425d00000020sv*sd*bc*sc*i*". The after string still
 * matches the before string, so this is not a regression.
 *
 * However, since matching up module aliases does involve
 * arbitrary-length wildcards, this function effectively needs to run
 * fnmatch() between every combination of before and after aliases,
 * resulting in a complexity of O(n^2).  To speed things up in the
 * (hopefully) usual case, the wildcard search is only run when an
 * exact string match of an alias (using hash tables) results in an
 * apparent regression.
 */
bool compare_module_aliases(kernel_alias_data_t *before, kernel_alias_data_t *after, module_alias_callback callback, void *user_data)
{
    kernel_alias_data_t *iter = NULL;
    kernel_alias_data_t *tmp_iter = NULL;
    kernel_alias_data_t *after_entry = NULL;
    string_list_t *before_modules = NULL;
    string_list_t *after_modules = NULL;
    string_list_t *difference = NULL;
    string_list_t empty;
    bool wildcard_search = false;
    bool result = true;

    assert(callback != NULL);

    /* Before empty, nothing to check for */
    if (before == NULL) {
        return true;
    }

    /* If after is NULL, fake the results with an empty list */
    if (after == NULL) {
        TAILQ_INIT(&empty);
    }

    /* For each alias in before, look for the matching alias in after */
    HASH_ITER(hh, before, iter, tmp_iter) {
        before_modules = iter->modules;

        /*
         * If the after list was NULL, no need to do a wildcard search. Just call the
         * function with an empty after list.
         */
        if (after == NULL) {
            callback(iter->alias, before_modules, &empty, user_data);
            continue;
        }

        HASH_FIND_STR(after, iter->alias, after_entry);
        wildcard_search = false;

        /* No match found, do a wildcard search */
        if (after_entry == NULL) {
            after_modules = wildcard_alias_search(iter->alias, after);
            wildcard_search = true;
        } else {
            after_modules = after_entry->modules;
            difference = list_difference(before_modules, after_modules);

            /* If the lists differ, do a wildcard search */
            if (difference != NULL && !TAILQ_EMPTY(difference)) {
                after_modules = wildcard_alias_search(iter->alias, after);
                wildcard_search = true;
            }

            list_free(difference, NULL);
        }

        /* Compare the results */
        difference = list_difference(before_modules, after_modules);

        if (difference != NULL && !TAILQ_EMPTY(difference)) {
            callback(iter->alias, before_modules, after_modules, user_data);
            result = false;
        }

        /* If after_modules was created from a wildcard search, free it */
        if (wildcard_search) {
            list_free(after_modules, NULL);
        }
    }

    return result;
}
