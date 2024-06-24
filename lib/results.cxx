/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include "queue.h"
#include "rpminspect.h"

/*
 * Initialize a struct result_params.
 */
void init_result_params(struct result_params *params)
{
    assert(params != NULL);

    params->severity = RESULT_OK;
    params->waiverauth = NULL_WAIVERAUTH;
    params->header = NULL;
    params->msg = NULL;
    params->details = NULL;
    params->remedy = NULL;
    params->verb = VERB_NIL;
    params->noun = NULL;
    params->arch = NULL;
    params->file = NULL;

    return;
}

/*
 * Initialize a new results_t list.
 */
results_t *init_results(void)
{
    results_t *results = NULL;

    results = (results_t *) xalloc(sizeof(*results));
    TAILQ_INIT(results);
    return results;
}

/*
 * Free memory associated with an results_t list.
 */
void free_results(results_t *results)
{
    results_entry_t *entry = NULL;

    if (results == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(results)) {
        entry = TAILQ_FIRST(results);
        TAILQ_REMOVE(results, entry, items);
        free(entry->msg);
        free(entry->details);
        free(entry->noun);
        free(entry->arch);
        free(entry->file);

        /* these are all consts */
        entry->header = NULL;

        free(entry);
    }

    free(results);

    return;
}

/*
 * Function to print one result for debugging purposes.
 */
void debug_print_result(const results_entry_t *result)
{
    const char *v = NULL;

    assert(result != NULL);

    switch (result->verb) {
        case VERB_NIL:
            v = "nil";
            break;
        case VERB_ADDED:
            v = "added";
            break;
        case VERB_REMOVED:
            v = "removed";
            break;
        case VERB_CHANGED:
            v = "changed";
            break;
        case VERB_FAILED:
            v = "failed";
            break;
        case VERB_OK:
            v = "ok";
            break;
        case VERB_SKIP:
            v = "skip";
            break;
        default:
            v = "UnKnOwN";
            break;
    }

    DEBUG_PRINT("result:\n");
    DEBUG_PRINT("    severity=|%s|\n", strseverity(result->severity));
    DEBUG_PRINT("  waiverauth=|%s|\n", strwaiverauth(result->waiverauth));
    DEBUG_PRINT("      header=|%s|\n", result->header ? result->header : "(null)");
    DEBUG_PRINT("         msg=|%s|\n", result->msg ? result->msg : "(null)");
    DEBUG_PRINT("     details=|%s|\n", result->details ? result->details : "(null)");
    DEBUG_PRINT("      remedy=|%s|\n", result->remedy ? result->remedy : "(null)");
    DEBUG_PRINT("        verb=|%s|\n", v);
    DEBUG_PRINT("        noun=|%s|\n", result->noun ? result->noun : "(noun)");
    DEBUG_PRINT("        arch=|%s|\n", result->arch ? result->arch : "(arch)");
    DEBUG_PRINT("        file=|%s|\n", result->file ? result->file : "(null)");

    return;
}

/*
 * Add the specified result to the list of results.  The parameters are the
 * members of the results_entry_t struct.  severity, waiverauth, header, and
 * msg are required.
 *
 * Strings passed in become results_entry_t members and will be freed when
 * the results_t is freed later.  Keep that in mind when building and
 * passing strings to this function.
 *
 * Pass NULL for any optional strings that you have no data for.
 */
void add_result_entry(results_t **results, struct result_params *params)
{
    results_entry_t *entry = NULL;

    assert(params != NULL);
    assert(params->severity >= 0);
    assert(params->header != NULL);

    if (*results == NULL) {
        *results = init_results();
    }

    entry = (results_entry_t *) xalloc(sizeof(*entry));

    entry->severity = params->severity;
    entry->waiverauth = params->waiverauth;
    entry->header = params->header;

    if (params->msg != NULL) {
        entry->msg = strdup(params->msg);
    }

    if (params->details != NULL) {
        entry->details = strdup(params->details);
    }

    if (params->remedy != NULL) {
        entry->remedy = strdup(params->remedy);
    }

    entry->verb = params->verb;

    if (params->noun != NULL) {
        entry->noun = strdup(params->noun);
    }

    if (params->arch != NULL) {
        entry->arch = strdup(params->arch);
    }

    if (params->file != NULL) {
        entry->file = strdup(params->file);
    }

    TAILQ_INSERT_TAIL(*results, entry, items);
    return;
}

/*
 * Shortcut to call add_result_entry() by giving the struct rpminspect.
 */
void add_result(struct rpminspect *ri, struct result_params *params)
{
    assert(ri != NULL);
    assert(params != NULL);
    assert(params->severity >= 0);

    if (params->severity > ri->worst_result) {
        ri->worst_result = params->severity;
    }

    add_result_entry(&ri->results, params);
    return;
}

/*
 * Returns true if all the results for the named inspection are
 * suppressed.
 */
bool suppressed_results(const results_t *results, const char *header, const severity_t suppress)
{
    results_entry_t *result = NULL;

    assert(results != NULL);
    assert(header != NULL);

    /* always output diagnostics */
    if (!strcmp(header, NAME_DIAGNOSTICS)) {
        return false;
    }

    TAILQ_FOREACH(result, results, items) {
        if (!strcmp(header, result->header) && result->severity >= suppress) {
            return false;
        }
    }

    return true;
}
