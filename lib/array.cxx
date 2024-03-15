/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <assert.h>
#include <err.h>
#include "parser.h"
#include "rpminspect.h"

/**
 * Given a string_list_t and a string, add the string to the list.  If
 * the list is NULL, initialize it to start a new list.  Caller is
 * responsible for freeing all memory associated with the list.
 *
 * @param list Pointer to a string_list_t to add entry to.
 * @param s String to add to the string_list_t.
 */
static void add_entry(string_list_t **list, const char *s)
{
    assert(list != NULL);
    assert(s != NULL);

    if (*list != NULL && list_contains(*list, s)) {
        /* do not add entry if it exists in the list */
        return;
    }

    *list = list_add(*list, s);
    return;
}

/* lambda for array() below. */
static bool array_cb(const char *entry, void *cb_data)
{
    string_list_t **list = cb_data;
    add_entry(list, entry);
    return false;
}

/* Transform configuration's array into a string_list_t. */
void array(parser_plugin *p, parser_context *ctx, const char *key1, const char *key2, string_list_t **list)
{
    if (key1 == NULL) {
        return;
    }

    if (p->strarray_foreach(ctx, key1, key2, array_cb, list)) {
        warnx(_("*** problem adding entries to array %s->%s"), key1, key2);
    }

    return;
}
