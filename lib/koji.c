/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <err.h>
#include <limits.h>
#include <xmlrpc-c/client.h>
#include <xmlrpc-c/client_global.h>
#include "queue.h"
#include "rpminspect.h"

/*
 * Koji build types supported by rpminspect.
 */
struct buildtype buildtypes[] = {
    /*
     * { koji_build_type_t (add to types.h),
     *   "name",
     *   bool--true if the type is supported },
     */
    { KOJI_BUILD_IMAGE,  "image",  false },
    { KOJI_BUILD_MAVEN,  "maven",  false },
    { KOJI_BUILD_MODULE, "module", true },
    { KOJI_BUILD_RPM,    "rpm",    true },
    { KOJI_BUILD_WIN,    "win",    false },

    /* array ends here so for loops can iterate */
    { KOJI_BUILD_NULL,   NULL,     false }
};

/*
 * Koji task and build states (could not find a way to get this from
 * the API)
 */
enum {
    TASK_FREE = 0,
    TASK_OPEN = 1,
    TASK_CLOSED = 2,
    TASK_CANCELED = 3,
    TASK_ASSIGNED = 4,
    TASK_FAILED = 5
};

enum {
    BUILD_BUILDING = 0,
    BUILD_COMPLETE = 1,
    BUILD_DELETED = 2,
    BUILD_FAILED = 3,
    BUILD_CANCELED = 4
};

/*
 * Return descriptive string for the task state.  Do not free this
 * string.
 */
static const char *task_state_desc(const int state)
{
    if (state == TASK_FREE) {
        return "free";
    } else if (state == TASK_OPEN) {
        return "open";
    } else if (state == TASK_CLOSED) {
        return "closed";
    } else if (state == TASK_CANCELED) {
        return "canceled";
    } else if (state == TASK_ASSIGNED) {
        return "assigned";
    } else if (state == TASK_FAILED) {
        return "failed";
    } else {
        return "UNKNOWN";
    }
}

/*
 * Return descriptive string for the build state.  Do not free this
 * string.
 */
static const char *build_state_desc(const int state)
{
    if (state == BUILD_BUILDING) {
        return "building";
    } else if (state == BUILD_COMPLETE) {
        return "complete";
    } else if (state == BUILD_DELETED) {
        return "deleted";
    } else if (state == BUILD_FAILED) {
        return "failed";
    } else if (state == BUILD_CANCELED) {
        return "canceled";
    } else {
        return "UNKNOWN";
    }
}

/*
 * General error handler for xmlrpc failures.  Could be improved
 * a bit to be more helpful to the user.
 */
static void xmlrpc_abort_on_fault(xmlrpc_env *env)
{
    if (env->fault_occurred) {
        errx(RI_PROGRAM_ERROR, _("XML-RPC Fault: %s (%d)"), env->fault_string, env->fault_code);
    }
}

/*
 * Read a koji task info struct and store in the struct koji_task.
 */
static void read_koji_task_struct(xmlrpc_env *env, xmlrpc_value *result, struct koji_task *task)
{
    int i = 0;
    int size = 0;
    char *key = NULL;
    xmlrpc_value *k = NULL;
    xmlrpc_value *value = NULL;

    assert(env != NULL);
    assert(result != NULL);
    assert(task != NULL);

    /* read the values from the result */
    size = xmlrpc_struct_size(env, result);
    xmlrpc_abort_on_fault(env);

    for (i = 0; i < size; i++) {
        xmlrpc_struct_read_member(env, result, i, &k, &value);
        xmlrpc_abort_on_fault(env);

        /* Get the key as a string */
        xmlrpc_decompose_value(env, k, "s", &key);
        xmlrpc_abort_on_fault(env);
        xmlrpc_DECREF(k);

        /*
         * If the value of a key is nil, just skip over it.  We can't
         * do anything with nil values, so it might as well just not
         * be present in the output.
         */
        if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
            xmlrpc_DECREF(value);
            continue;
        }

        /*
         * Walk through the keys and fill in the struct.
         * This is tedious, but I would rather do it now rather
         * than unpacking XMLRPC results in later functions.
         */
        if (!strcmp(key, "weight")) {
            xmlrpc_decompose_value(env, value, "d", &task->weight);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "parent") && xmlrpc_value_type(value) != XMLRPC_TYPE_NIL) {
            xmlrpc_decompose_value(env, value, "i", &task->parent);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "completion_time")) {
            xmlrpc_decompose_value(env, value, "s", &task->completion_time);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "start_time")) {
            xmlrpc_decompose_value(env, value, "s", &task->start_time);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "start_ts")) {
            xmlrpc_decompose_value(env, value, "d", &task->start_ts);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "waiting")) {
            xmlrpc_decompose_value(env, value, "b", &task->waiting);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "awaited")) {
            xmlrpc_decompose_value(env, value, "b", &task->awaited);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "label")) {
            xmlrpc_decompose_value(env, value, "s", &task->label);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "priority")) {
            xmlrpc_decompose_value(env, value, "i", &task->priority);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "channel_id")) {
            xmlrpc_decompose_value(env, value, "i", &task->channel_id);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "state")) {
            xmlrpc_decompose_value(env, value, "i", &task->state);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "create_time")) {
            xmlrpc_decompose_value(env, value, "s", &task->create_time);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "create_ts")) {
            xmlrpc_decompose_value(env, value, "d", &task->create_ts);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "owner")) {
            xmlrpc_decompose_value(env, value, "i", &task->owner);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "host_id")) {
            xmlrpc_decompose_value(env, value, "i", &task->host_id);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "method")) {
            xmlrpc_decompose_value(env, value, "s", &task->method);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "completion_ts")) {
            xmlrpc_decompose_value(env, value, "d", &task->completion_ts);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "arch")) {
            xmlrpc_decompose_value(env, value, "s", &task->arch);
            xmlrpc_abort_on_fault(env);
        } else if (!strcmp(key, "id")) {
            xmlrpc_decompose_value(env, value, "i", &task->id);
            xmlrpc_abort_on_fault(env);
        }

        xmlrpc_DECREF(value);
        free(key);
        key = NULL;
    }

    return;
}

/*
 * Turn the array of strings in to a string_list_t.  Used when reading task
 * results through get_koji_task().
 */
static string_list_t *read_koji_descendent_results(xmlrpc_env *env, xmlrpc_value *value)
{
    int i = 0;
    int size = 0;
    string_list_t *results = NULL;
    string_entry_t *entry = NULL;
    xmlrpc_value *s = NULL;

    assert(env != NULL);
    assert(value != NULL);

    size = xmlrpc_array_size(env, value);
    xmlrpc_abort_on_fault(env);

    results = calloc(1, sizeof(*results));
    assert(results != NULL);
    TAILQ_INIT(results);

    for (i = 0; i < size; i++) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        xmlrpc_array_read_item(env, value, i, &s);
        xmlrpc_abort_on_fault(env);
        xmlrpc_decompose_value(env, s, "s", &entry->data);
        xmlrpc_abort_on_fault(env);
        xmlrpc_DECREF(s);
        TAILQ_INSERT_TAIL(results, entry, items);
    }

    return results;
}

/*
 * Initialize a koji_buildlist_t.
 */
koji_buildlist_t *init_koji_buildlist(void)
{
    koji_buildlist_t *builds = NULL;

    builds = calloc(1, sizeof(*(builds)));
    assert(builds != NULL);
    TAILQ_INIT(builds);
    return builds;
}

/*
 * Free memory associated with a koji_buildlist_t.
 */
void free_koji_buildlist(koji_buildlist_t *builds)
{
    koji_buildlist_entry_t *entry = NULL;

    if (builds == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(builds)) {
        entry = TAILQ_FIRST(builds);
        TAILQ_REMOVE(builds, entry, builditems);

        free(entry->package_name);
        free(entry->owner_name);
        free(entry->nvr);
        free(entry->start_time);
        free(entry->creation_time);
        free(entry->completion_time);
        free(entry->tag_name);
        free(entry->version);
        free(entry->release);
        free(entry->volume_name);
        free(entry->name);

        free_koji_rpmlist(entry->rpms);

        free(entry);
    }

    free(builds);

    return;
}

/*
 * Initialize a koji_rpmlist_t.
 */
koji_rpmlist_t *init_koji_rpmlist(void)
{
    koji_rpmlist_t *rpms = NULL;

    rpms = calloc(1, sizeof(*(rpms)));
    assert(rpms != NULL);
    TAILQ_INIT(rpms);
    return rpms;
}

/*
 * Free memory associated with a koji_rpmlist_entry_t.
 */
void free_koji_rpmlist_entry(koji_rpmlist_entry_t *entry)
{
    if (entry == NULL) {
        return;
    }

    free(entry->arch);
    free(entry->name);
    free(entry->version);
    free(entry->release);
    free(entry);

    return;
}

/*
 * Free memory associated with a koji_rpmlist_t.
 */
void free_koji_rpmlist(koji_rpmlist_t *rpms)
{
    koji_rpmlist_entry_t *entry = NULL;

    if (rpms == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(rpms)) {
        entry = TAILQ_FIRST(rpms);
        TAILQ_REMOVE(rpms, entry, items);
        free_koji_rpmlist_entry(entry);
    }

    free(rpms);

    return;
}

/*
 * Initialize a struct koji_build.
 */
void init_koji_build(struct koji_build *build)
{
    assert(build != NULL);

    build->package_name = NULL;
    build->epoch = 0;
    build->name = NULL;
    build->version = NULL;
    build->release = NULL;
    build->nvr = NULL;

    build->source = NULL;

    build->creation_time = NULL;
    build->completion_time = NULL;
    build->package_id = -1;
    build->id = -1;
    build->state = -1;
    build->completion_ts = 0;
    build->owner_id = -1;
    build->owner_name = NULL;
    build->start_time = NULL;
    build->creation_event_id = -1;
    build->start_ts = 0;
    build->creation_ts = 0;
    build->task_id = -1;

    build->volume_id = -1;
    build->volume_name = NULL;

    build->original_url = NULL;

    build->modulemd_str = NULL;
    build->module_name = NULL;
    build->module_stream = NULL;
    build->module_build_service_id = -1;
    build->module_version = NULL;
    build->module_context = NULL;
    build->module_content_koji_tag = NULL;

    build->builds = init_koji_buildlist();

    return;
}

/*
 * Initialize a struct koji_task
 */
void init_koji_task(struct koji_task *task)
{
    assert(task != NULL);

    task->weight = -1;
    task->completion_time = NULL;
    task->start_time = NULL;
    task->start_ts = -1;
    task->waiting = false;
    task->awaited = false;
    task->label = NULL;
    task->priority = -1;
    task->channel_id = -1;
    task->state = -1;
    task->create_time = NULL;
    task->create_ts = -1;
    task->owner = -1;
    task->host_id = -1;
    task->method = NULL;
    task->completion_ts = -1;
    task->arch = NULL;
    task->id = -1;
    task->descendents = NULL;

    return;
}

/*
 * Initialize a struct koji_task_list
 */
void init_koji_task_entry(koji_task_entry_t *entry)
{
    assert(entry != NULL);

    entry->task = malloc(sizeof(*entry->task));
    assert(entry->task != NULL);
    init_koji_task(entry->task);

    entry->brootid = -1;

    entry->srpms = malloc(sizeof(*entry->srpms));
    assert(entry->srpms != NULL);
    TAILQ_INIT(entry->srpms);

    entry->rpms = malloc(sizeof(*entry->rpms));
    assert(entry->rpms != NULL);
    TAILQ_INIT(entry->rpms);

    entry->logs = malloc(sizeof(*entry->logs));
    assert(entry->logs != NULL);
    TAILQ_INIT(entry->logs);

    return;
}

/*
 * Free dynamically allocated memory associated with a struct koji_build.
 */
void free_koji_build(struct koji_build *build)
{
    if (build == NULL) {
        return;
    }

    free(build->package_name);
    free(build->name);
    free(build->version);
    free(build->release);
    free(build->nvr);

    free(build->source);

    free(build->creation_time);
    free(build->completion_time);
    free(build->owner_name);
    free(build->start_time);

    free(build->volume_name);
    free(build->cg_name);

    free(build->original_url);

    free(build->modulemd_str);
    free(build->module_name);
    free(build->module_stream);
    free(build->module_version);
    free(build->module_context);
    free(build->module_content_koji_tag);

    free_koji_buildlist(build->builds);

    free(build);
    build = NULL;

    return;
}

/*
 * Free dynamically allocated memory associated with a koji_task_entry_t.
 */
void free_koji_task_entry(koji_task_entry_t *entry)
{
    if (entry == NULL) {
        return;
    }

    free_koji_task(entry->task);
    list_free(entry->srpms, free);
    list_free(entry->rpms, free);
    list_free(entry->logs, free);
    free(entry);
    entry = NULL;

    return;
}

/*
 * Free dynamically allocated memory associated with a struct koji_task.
 */
void free_koji_task(struct koji_task *task)
{
    koji_task_entry_t *descendent = NULL;

    if (task == NULL) {
        return;
    }

    free(task->completion_time);
    free(task->start_time);
    free(task->label);
    free(task->create_time);
    free(task->method);
    free(task->arch);

    if (task->descendents) {
        while (!TAILQ_EMPTY(task->descendents)) {
            descendent = TAILQ_FIRST(task->descendents);
            TAILQ_REMOVE(task->descendents, descendent, items);
            free_koji_task_entry(descendent);
        }

        free(task->descendents);
    }

    free(task);
    task = NULL;

    return;
}

/*
 * Look up a Koji build and return the information in a struct koji_build.
 */
struct koji_build *get_koji_build(struct rpminspect *ri, const char *buildspec)
{
    struct koji_build *build = NULL;
    int i = 0;
    int j = 0;
    int k = 0;
    int size = 0;
    int subsize = 0;
    int modsize = 0;
    xmlrpc_env env;
    xmlrpc_value *key = NULL;
    xmlrpc_value *value = NULL;
    xmlrpc_value *subv = NULL;
    xmlrpc_value *modv = NULL;
    xmlrpc_value *result = NULL;
    xmlrpc_value *element = NULL;
    char *keyname = NULL;
    koji_buildlist_entry_t *buildentry = NULL;
    koji_rpmlist_entry_t *rpm = NULL;

    assert(ri != NULL);

    if (buildspec == NULL) {
        return NULL;
    }

    /* if there is no koji system specified in the configuration, stop */
    if (ri->kojihub == NULL) {
        return NULL;
    }

    /* initialize everything and get XMLRPC ready */
    build = calloc(1, sizeof(*build));
    assert(build != NULL);
    init_koji_build(build);
    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, SIZE_MAX);
    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, SOFTWARE_NAME, PACKAGE_VERSION, NULL, 0);
    xmlrpc_abort_on_fault(&env);

    /* call 'getBuild' on the koji hub */
    result = xmlrpc_client_call(&env, ri->kojihub, "getBuild", "(s)", buildspec);

    if (env.fault_occurred) {
        if (env.fault_code >= 1000 || env.fault_code < 0) {
            /* server side error which means Koji protocol error */
            xmlrpc_env_clean(&env);
            xmlrpc_client_cleanup();
            free_koji_build(build);
            return NULL;
        } else {
            /* we have no idea, so just fail */
            xmlrpc_abort_on_fault(&env);
        }
    }

    /* is this a valid build? */
    if (xmlrpc_value_type(result) == XMLRPC_TYPE_NIL) {
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        xmlrpc_DECREF(result);
        free_koji_build(build);
        return NULL;
    }

    /* read the values from the result */
    size = xmlrpc_struct_size(&env, result);
    xmlrpc_abort_on_fault(&env);
    i = 0;

    while (i < size) {
        xmlrpc_struct_read_member(&env, result, i, &key, &value);
        xmlrpc_abort_on_fault(&env);

        /*
         * If the value of a key is nil, just skip over it.  We can't
         * do anything with nil values, so it might as well just not
         * be present in the output.
         */
        if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
            xmlrpc_decompose_value(&env, value, "n");
            xmlrpc_DECREF(key);
            xmlrpc_DECREF(value);
            i++;
            continue;
        }

        /* Get the key as a string */
        xmlrpc_decompose_value(&env, key, "s", &keyname);
        xmlrpc_abort_on_fault(&env);
        xmlrpc_DECREF(key);

        /*
         * Walk through the keys and fill in the struct.
         * This is tedious, but I would rather do it now rather
         * than unpacking XMLRPC results in later functions.
         */
        if (!strcmp(keyname, "package_name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->package_name);
        } else if (!strcmp(keyname, "epoch")) {
            xmlrpc_decompose_value(&env, value, "i", &build->epoch);
        } else if (!strcmp(keyname, "name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->name);
        } else if (!strcmp(keyname, "version")) {
            xmlrpc_decompose_value(&env, value, "s", &build->version);
        } else if (!strcmp(keyname, "release")) {
            xmlrpc_decompose_value(&env, value, "s", &build->release);
        } else if (!strcmp(keyname, "nvr")) {
            xmlrpc_decompose_value(&env, value, "s", &build->nvr);
        } else if (!strcmp(keyname, "source")) {
            xmlrpc_decompose_value(&env, value, "s", &build->source);
        } else if (!strcmp(keyname, "creation_time")) {
            xmlrpc_decompose_value(&env, value, "s", &build->creation_time);
        } else if (!strcmp(keyname, "completion_time")) {
            xmlrpc_decompose_value(&env, value, "s", &build->completion_time);
        } else if (!strcmp(keyname, "package_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->package_id);
        } else if (!strcmp(keyname, "id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->id);
        } else if (!strcmp(keyname, "build_id")) {
            /* we hit this on regular packages, modules handled below */
            buildentry = calloc(1, sizeof(*buildentry));
            assert(buildentry != NULL);

            xmlrpc_decompose_value(&env, value, "i", &buildentry->build_id);

            if (build->package_name != NULL) {
                buildentry->package_name = strdup(build->package_name);
            }

            buildentry->rpms = calloc(1, sizeof(*buildentry->rpms));
            assert(buildentry->rpms != NULL);
            TAILQ_INIT(buildentry->rpms);

            TAILQ_INSERT_TAIL(build->builds, buildentry, builditems);
        } else if (!strcmp(keyname, "state")) {
            xmlrpc_decompose_value(&env, value, "i", &build->state);
        } else if (!strcmp(keyname, "completion_ts")) {
            xmlrpc_decompose_value(&env, value, "d", &build->completion_ts);
        } else if (!strcmp(keyname, "owner_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->owner_id);
        } else if (!strcmp(keyname, "owner_name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->owner_name);
        } else if (!strcmp(keyname, "start_time")) {
            xmlrpc_decompose_value(&env, value, "s", &build->start_time);
        } else if (!strcmp(keyname, "creation_event_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->creation_event_id);
        } else if (!strcmp(keyname, "start_ts")) {
            xmlrpc_decompose_value(&env, value, "d", &build->start_ts);
        } else if (!strcmp(keyname, "volume_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->volume_id);
        } else if (!strcmp(keyname, "creation_ts")) {
            xmlrpc_decompose_value(&env, value, "d", &build->creation_ts);
        } else if (!strcmp(keyname, "task_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->task_id);
        } else if (!strcmp(keyname, "volume_name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->volume_name);
        } else if (!strcmp(keyname, "cg_name")) {
            xmlrpc_decompose_value(&env, value, "s", &build->cg_name);
        } else if (!strcmp(keyname, "cg_id")) {
            xmlrpc_decompose_value(&env, value, "i", &build->cg_id);
        } else if (!strcmp(keyname, "extra")) {
            /*
             * EXTRA METADATA HANDLING
             * This is where module metadata hides, but there can also
             * be some stuff for regular builds.  Handle accordingly
             * and collect the information.
             */

            /* clear keyname */
            free(keyname);
            keyname = NULL;

            /* read the values from the result */
            subsize = xmlrpc_struct_size(&env, value);
            xmlrpc_abort_on_fault(&env);
            j = 0;

            while (j < subsize) {
                xmlrpc_struct_read_member(&env, value, j, &key, &subv);
                xmlrpc_abort_on_fault(&env);

                /* Skip nil values */
                if (xmlrpc_value_type(subv) == XMLRPC_TYPE_NIL) {
                    xmlrpc_decompose_value(&env, subv, "n");
                    xmlrpc_DECREF(key);
                    xmlrpc_DECREF(subv);
                    j++;
                    continue;
                }

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, key, "s", &keyname);
                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(key);

                /* look for the module information structs */
                if (!strcmp(keyname, "source") || !strcmp(keyname, "typeinfo") || !strcmp(keyname, "module")) {
                    free(keyname);
                    keyname = NULL;

                    modsize = xmlrpc_struct_size(&env, subv);
                    xmlrpc_abort_on_fault(&env);

                    /* drill down to the real struct */
                    if (modsize == 1) {
                        subsize = modsize;
                        xmlrpc_DECREF(value);
                        value = subv;
                        j = 0;
                        continue;
                    }

                    k = 0;

                    while (k < modsize) {
                        xmlrpc_struct_read_member(&env, subv, k, &key, &modv);
                        xmlrpc_abort_on_fault(&env);

                        /* Skip nil values */
                        if (xmlrpc_value_type(modv) == XMLRPC_TYPE_NIL) {
                            xmlrpc_DECREF(key);
                            xmlrpc_DECREF(modv);
                            k++;
                            continue;
                        }

                        /* Get the key as a string */
                        xmlrpc_decompose_value(&env, key, "s", &keyname);
                        xmlrpc_abort_on_fault(&env);
                        xmlrpc_DECREF(key);

                        /* Grab the values we need */
                        if (!strcmp(keyname, "original_url")) {
                            xmlrpc_decompose_value(&env, modv, "s", &build->original_url);
                        } else if (!strcmp(keyname, "name")) {
                            xmlrpc_decompose_value(&env, modv, "s", &build->module_name);
                        } else if (!strcmp(keyname, "stream")) {
                            xmlrpc_decompose_value(&env, modv, "s", &build->module_stream);
                        } else if (!strcmp(keyname, "module_build_service_id")) {
                            xmlrpc_decompose_value(&env, modv, "i", &build->module_build_service_id);
                        } else if (!strcmp(keyname, "version")) {
                            xmlrpc_decompose_value(&env, modv, "s", &build->module_version);
                        } else if (!strcmp(keyname, "context")) {
                            xmlrpc_decompose_value(&env, modv, "s", &build->module_context);
                        } else if (!strcmp(keyname, "content_koji_tag")) {
                            xmlrpc_decompose_value(&env, modv, "s", &build->module_content_koji_tag);
                        } else if (!strcmp(keyname, "modulemd_str") && ri->buildtype == KOJI_BUILD_NULL) {
                            ri->buildtype = KOJI_BUILD_MODULE;
                            xmlrpc_decompose_value(&env, modv, "s", &build->modulemd_str);
                        }

                        xmlrpc_abort_on_fault(&env);
                        xmlrpc_DECREF(modv);
                        free(keyname);
                        keyname = NULL;
                        k++;
                    }
                }

                xmlrpc_DECREF(subv);
                free(keyname);
                keyname = NULL;
                j++;
            }
        }

        xmlrpc_abort_on_fault(&env);
        xmlrpc_DECREF(value);
        free(keyname);
        keyname = NULL;
        i++;
    }

    xmlrpc_DECREF(result);

    /* build must be complete */
    if (build->state != BUILD_COMPLETE) {
        warnx(_("Koji build state is %s for %s, cannot continue."), build_state_desc(build->state), buildspec);
        free_koji_build(build);
        return NULL;
    }

    /* assume the build type is rpm if not set yet */
    if (ri->buildtype == KOJI_BUILD_NULL) {
        ri->buildtype = KOJI_BUILD_RPM;
    }

    /* Modules have multiple builds, so collect the IDs */
    if (ri->buildtype == KOJI_BUILD_MODULE) {
        result = xmlrpc_client_call(&env, ri->kojihub, "getLatestBuilds", "(s)", build->module_content_koji_tag);
        xmlrpc_abort_on_fault(&env);

        /* read the values from the result */
        size = xmlrpc_array_size(&env, result);
        xmlrpc_abort_on_fault(&env);
        i = 0;

        while (i < size) {
            xmlrpc_array_read_item(&env, result, i, &element);
            xmlrpc_abort_on_fault(&env);

            /* each array element is a struct */
            subsize = xmlrpc_struct_size(&env, element);
            xmlrpc_abort_on_fault(&env);

            buildentry = calloc(1, sizeof(*buildentry));
            assert(buildentry != NULL);

            j = 0;

            while (j < subsize) {
                xmlrpc_struct_read_member(&env, element, j, &key, &value);
                xmlrpc_abort_on_fault(&env);

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, key, "s", &keyname);
                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(key);

                /* Skip nil values */
                if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
                    xmlrpc_DECREF(value);
                    free(keyname);
                    keyname = NULL;
                    j++;
                    continue;
                }

                /* Grab the values we need */
                if (!strcmp(keyname, "build_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->build_id);
                } else if (!strcmp(keyname, "completion_time")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->completion_time);
                } else if (!strcmp(keyname, "create_event")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->create_event);
                } else if (!strcmp(keyname, "creation_event_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->creation_event_id);
                } else if (!strcmp(keyname, "creation_time")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->creation_time);
                } else if (!strcmp(keyname, "epoch")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->epoch);
                } else if (!strcmp(keyname, "id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->id);
                } else if (!strcmp(keyname, "name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->name);
                } else if (!strcmp(keyname, "nvr")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->nvr);
                } else if (!strcmp(keyname, "owner_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->owner_id);
                } else if (!strcmp(keyname, "owner_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->owner_name);
                } else if (!strcmp(keyname, "package_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->package_id);
                } else if (!strcmp(keyname, "package_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->package_name);
                } else if (!strcmp(keyname, "release")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->release);
                } else if (!strcmp(keyname, "start_time")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->start_time);
                } else if (!strcmp(keyname, "state")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->state);
                } else if (!strcmp(keyname, "tag_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->tag_id);
                } else if (!strcmp(keyname, "tag_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->tag_name);
                } else if (!strcmp(keyname, "task_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->task_id);
                } else if (!strcmp(keyname, "version")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->version);
                } else if (!strcmp(keyname, "volume_id")) {
                    xmlrpc_decompose_value(&env, value, "i", &buildentry->volume_id);
                } else if (!strcmp(keyname, "volume_name")) {
                    xmlrpc_decompose_value(&env, value, "s", &buildentry->volume_name);
                }

                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(value);
                free(keyname);
                keyname = NULL;
                j++;
            }

            xmlrpc_DECREF(element);

            buildentry->rpms = calloc(1, sizeof(*buildentry->rpms));
            assert(buildentry->rpms != NULL);
            TAILQ_INIT(buildentry->rpms);

            TAILQ_INSERT_TAIL(build->builds, buildentry, builditems);

            i++;
        }

        xmlrpc_DECREF(result);
    }

    /* Call 'listBuildRPMs' on the koji hub for each build_id */
    TAILQ_FOREACH(buildentry, build->builds, builditems) {
        result = xmlrpc_client_call(&env, ri->kojihub, "listBuildRPMs", "(i)", buildentry->build_id);
        xmlrpc_abort_on_fault(&env);

        /* read the values from the result */
        size = xmlrpc_array_size(&env, result);
        xmlrpc_abort_on_fault(&env);

        for (i = 0; i < size; i++) {
            xmlrpc_array_read_item(&env, result, i, &element);
            xmlrpc_abort_on_fault(&env);

            /* each array element is a struct */
            subsize = xmlrpc_struct_size(&env, element);
            xmlrpc_abort_on_fault(&env);

            /* create a new rpm list entry */
            rpm = calloc(1, sizeof(*rpm));
            assert(rpm != NULL);

            for (j = 0; j < subsize; j++) {
                xmlrpc_struct_read_member(&env, element, j, &key, &value);
                xmlrpc_abort_on_fault(&env);

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, key, "s", &keyname);
                xmlrpc_abort_on_fault(&env);

                /* Skip nil values */
                if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
                    xmlrpc_DECREF(value);
                    xmlrpc_DECREF(key);
                    free(keyname);
                    keyname = NULL;
                    continue;
                }

                /* Grab the values we need */
                if (!strcmp(keyname, "arch")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->arch);
                } else if (!strcmp(keyname, "name")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->name);
                } else if (!strcmp(keyname, "version")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->version);
                } else if (!strcmp(keyname, "release")) {
                    xmlrpc_decompose_value(&env, value, "s", &rpm->release);
                } else if (!strcmp(keyname, "epoch")) {
                    xmlrpc_decompose_value(&env, value, "i", &rpm->epoch);
                } else if (!strcmp(keyname, "size")) {
                    if (xmlrpc_value_type(value) == XMLRPC_TYPE_INT) {
                        xmlrpc_decompose_value(&env, value, "i", &rpm->size);
                    } else if (xmlrpc_value_type(value) == XMLRPC_TYPE_I8) {
                        xmlrpc_decompose_value(&env, value, "I", &rpm->size);
                    } else {
                        /*
                         * XXX: have no idea what we got back here
                         */
                        rpm->size = 0;
                    }
                }

                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(value);
                xmlrpc_DECREF(key);
                free(keyname);
                keyname = NULL;
            }

            /* add this rpm to the list */
            if (allowed_arch(ri, rpm->arch)) {
                build->total_size += rpm->size;
                TAILQ_INSERT_TAIL(buildentry->rpms, rpm, items);
            } else {
                free_koji_rpmlist_entry(rpm);
            }

            xmlrpc_DECREF(element);
        }

        xmlrpc_DECREF(result);
    }

    /* Cleanup */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return build;
}

/*
 * Look up a Koji task and return the information in a struct koji_build.
 */
struct koji_task *get_koji_task(struct rpminspect *ri, const char *taskspec)
{
    int i, j, k;
    int size, dsize, rsize;
    xmlrpc_env env;
    xmlrpc_value *result = NULL;
    xmlrpc_value *xk = NULL;
    xmlrpc_value *xv = NULL;
    xmlrpc_value *tr_k = NULL;
    xmlrpc_value *tr_v = NULL;
    xmlrpc_value *dstruct = NULL;
    xmlrpc_value *dresult = NULL;
    char *key = NULL;
    struct koji_task *task = NULL;
    koji_task_entry_t *descendent = NULL;

    assert(ri != NULL);

    if (taskspec == NULL) {
        return NULL;
    }

    /* if there is no koji system specified in the configuration, stop */
    if (ri->kojihub == NULL) {
        return NULL;
    }

    /* initialize everything and get XMLRPC ready */
    task = calloc(1, sizeof(*task));
    assert(task != NULL);
    init_koji_task(task);
    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, SIZE_MAX);
    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, SOFTWARE_NAME, PACKAGE_VERSION, NULL, 0);
    xmlrpc_abort_on_fault(&env);

    /* call 'getTaskInfo' on the koji hub */
    result = xmlrpc_client_call(&env, ri->kojihub, "getTaskInfo", "(s)", taskspec);

    if (env.fault_occurred) {
        if (env.fault_code >= 1000 || env.fault_code < 0 || env.fault_code == 1) {
            /* server side error which means Koji protocol error */
            xmlrpc_env_clean(&env);
            xmlrpc_client_cleanup();
            free_koji_task(task);
            return NULL;
        } else {
            /* we have no idea, so just fail */
            xmlrpc_abort_on_fault(&env);
        }
    }

    /* is this a valid build? */
    if (xmlrpc_value_type(result) == XMLRPC_TYPE_NIL) {
        xmlrpc_DECREF(result);
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        free_koji_task(task);
        return NULL;
    }

    read_koji_task_struct(&env, result, task);
    xmlrpc_DECREF(result);

    /* task must be closed */
    if (task->state != TASK_CLOSED) {
        warnx(_("Koji task state is %s for task %s, cannot continue."), task_state_desc(task->state), taskspec);
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        free_koji_task(task);
        return NULL;
    }

    /* call 'getTaskDescendents' on the task ID */
    result = xmlrpc_client_call(&env, ri->kojihub, "getTaskDescendents", "(s)", taskspec);
    xmlrpc_abort_on_fault(&env);

    /* read the values from the result */
    size = xmlrpc_struct_size(&env, result);
    xmlrpc_abort_on_fault(&env);

    task->descendents = calloc(1, sizeof(*task->descendents));
    assert(task->descendents != NULL);
    TAILQ_INIT(task->descendents);

    for (i = 0; i < size; i++) {
        xmlrpc_struct_read_member(&env, result, i, &xk, &xv);
        xmlrpc_abort_on_fault(&env);

        /* Continue if the value is empty */
        dsize = xmlrpc_array_size(&env, xv);

        if (dsize == 0) {
            continue;
        }

        /* Read the value array */
        for (j = 0; j < dsize; j++) {
            /* get the descendent task struct */
            xmlrpc_array_read_item(&env, xv, j, &dstruct);
            xmlrpc_abort_on_fault(&env);

            /* initialize a struct and read the results */
            descendent = calloc(1, sizeof(*descendent));
            assert(descendent != NULL);
            init_koji_task_entry(descendent);
            read_koji_task_struct(&env, dstruct, descendent->task);

            /* gather the task results */
            dresult = xmlrpc_client_call(&env, ri->kojihub, "getTaskResult", "(i)", descendent->task->id);
            xmlrpc_abort_on_fault(&env);

            if (xmlrpc_value_type(dresult) == XMLRPC_TYPE_NIL) {
                /* some task IDs may be nothing, so ignore */
                xmlrpc_DECREF(dresult);
                continue;
            }

            rsize = xmlrpc_struct_size(&env, dresult);

            for (k = 0; k < rsize; k++) {
                /* Read the result struct */
                xmlrpc_struct_read_member(&env, dresult, k, &tr_k, &tr_v);
                xmlrpc_abort_on_fault(&env);

                /* Get the key as a string */
                xmlrpc_decompose_value(&env, tr_k, "s", &key);
                xmlrpc_abort_on_fault(&env);
                xmlrpc_DECREF(tr_k);

                /* Read the values */
                if (!strcmp(key, "brootid")) {
                    xmlrpc_decompose_value(&env, tr_v, "i", &descendent->brootid);
                    xmlrpc_abort_on_fault(&env);
                } else if (!strcmp(key, "srpms") && xmlrpc_value_type(tr_v) == XMLRPC_TYPE_ARRAY) {
                    descendent->srpms = read_koji_descendent_results(&env, tr_v);
                } else if (!strcmp(key, "rpms")) {
                    descendent->rpms = read_koji_descendent_results(&env, tr_v);
                } else if (!strcmp(key, "logs")) {
                    descendent->logs = read_koji_descendent_results(&env, tr_v);
                }

                xmlrpc_DECREF(tr_v);
                free(key);
            }

            /* save this descendent in the list */
            TAILQ_INSERT_TAIL(task->descendents, descendent, items);

            xmlrpc_DECREF(dresult);
        }
    }

    xmlrpc_DECREF(result);

    /* Cleanup */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return task;
}

/*
 * Return a list of all architectures supported by this Koji instance.
 * NOTE: This should not be called until after config file initialization
 * because we need the Koji settings from the conf file in order to make
 * the XML-RPC calls.
 */
string_list_t *get_all_arches(const struct rpminspect *ri)
{
    string_list_t *arches = NULL;
    int size = 0;
    int i = 0;
    xmlrpc_env env;
    xmlrpc_server_info *server = NULL;
    xmlrpc_value *fake_params = NULL;
    xmlrpc_value *result = NULL;
    xmlrpc_value *value = NULL;
    char *element = NULL;
    bool have_noarch = false;
    bool have_src = false;

    assert(ri != NULL);

    /* if there is no koji system specified in the configuration, stop */
    if (ri->kojihub == NULL) {
        err(EXIT_FAILURE, "*** missing Koji Hub setting from configuration file");
    }

    /* initialize our list of architectures, always allow 'src' */
    arches = list_add(arches, SRPM_ARCH_NAME);

    /* initialize everything and get XMLRPC ready */
    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, SIZE_MAX);
    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, SOFTWARE_NAME, PACKAGE_VERSION, NULL, 0);
    xmlrpc_abort_on_fault(&env);

    /*
     * call 'getAllArches' on the koji hub
     * Why not use xmlrpc_client_call() here?  I'm glad you asked.  That
     * function takes a format string and argument list and builds a
     * parameter array for the call.  There's just one problem.  You can't
     * tell it NULL for the format string for calls that require no
     * parameters.  This is a problem for this call because we need to
     * pass nothing.  Within libxmlrpc there is another call we can use
     * but we do still have to create an empty argument array and pass it
     * anyway.  I believe it's better to use xmlrpc_client_call_server_params
     * rather than the xmlrpc_client_call function.  This is what the command
     * line xmlrpc(1) tool does and it works with basically anything.
     */
    server = xmlrpc_server_info_new(&env, ri->kojihub);
    xmlrpc_abort_on_fault(&env);
    fake_params = xmlrpc_array_new(&env);              /* super empty array */
    result = xmlrpc_client_call_server_params(&env, server, "getAllArches", fake_params);
    xmlrpc_abort_on_fault(&env);
    xmlrpc_DECREF(fake_params);
    xmlrpc_server_info_free(server);

    /* is this a valid return value? */
    if (xmlrpc_value_type(result) != XMLRPC_TYPE_ARRAY) {
        xmlrpc_env_clean(&env);
        xmlrpc_client_cleanup();
        xmlrpc_DECREF(result);
        return NULL;
    }

    /* read the values from the result */
    size = xmlrpc_array_size(&env, result);
    xmlrpc_abort_on_fault(&env);

    for (i = 0; i < size; i++) {
        xmlrpc_array_read_item(&env, result, i, &value);
        xmlrpc_abort_on_fault(&env);

        /* Get the array element as a string */
        xmlrpc_decompose_value(&env, value, "s", &element);
        xmlrpc_abort_on_fault(&env);
        xmlrpc_DECREF(value);

        /* Flag what we have */
        if (!strcmp(element, RPM_NOARCH_NAME)) {
            have_noarch = true;
        } else if (!strcmp(element, SRPM_ARCH_NAME)) {
            have_src = true;
        }

        /*
         * If the value of an element is nil, just skip over it.  We can't
         * do anything with nil values, so it might as well just not
         * be present in the output.
         */
        if (xmlrpc_value_type(value) == XMLRPC_TYPE_NIL) {
            xmlrpc_DECREF(value);
            free(element);
            element = NULL;
            continue;
        }

        /* add this architecture to the list */
        arches = list_add(arches, element);
    }

    xmlrpc_DECREF(result);

    /* Always add 'noarch' and 'src' to this list */
    if (!have_noarch) {
        arches = list_add(arches, RPM_NOARCH_NAME);
    }

    if (!have_src) {
        arches = list_add(arches, SRPM_ARCH_NAME);
    }

    /* Cleanup */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    return arches;
}

const char *buildtype_desc(const koji_build_type_t type)
{
    switch (type) {
        case KOJI_BUILD_RPM:
            return _("RPM package build");
        case KOJI_BUILD_MODULE:
            return _("Module build consisting of multiple RPM package builds");
        default:
            return NULL;
    }
}
