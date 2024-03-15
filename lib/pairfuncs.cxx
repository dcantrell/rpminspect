/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdlib.h>
#include "queue.h"
#include "rpminspect.h"

/**
 * @brief Check if a pair list has a key.
 * @param The pair_list_t to check.
 * @param The key to look for.
 * @returns True if the pair list contains the key.
 */
bool pair_contains_key(const pair_list_t *list, const char *key)
{
    pair_entry_t *pair = NULL;

    if (list == NULL) {
        return false;
    }

    TAILQ_FOREACH(pair, list, items) {
        if (key == NULL && pair->key == key) {
            return true;
        } else if (key && pair->key && !strcmp(key, pair->key)) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Free a pair_list_t and all member data.
 * @param The pair_list_t to free.
 */
void free_pair(pair_list_t *list)
{
    pair_entry_t *pair = NULL;

    if (list == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(list)) {
        pair = TAILQ_FIRST(list);
        TAILQ_REMOVE(list, pair, items);
        free(pair->key);
        free(pair->value);
        free(pair);
    }

    free(list);
    return;
}
