/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"
#include "rpminspect.h"

string_list_t *sorted_list = NULL;

/**
 * @brief Join all members of a string_list_t in to a single string.
 *
 * Given a string_list_t, combine all the members in to a newly
 * allocated string.  An optional delimiter can be provided by passing
 * a string as the delimiter argument.  If NULL given as the
 * delimited, all strings will be concatenated together.  Caller is
 * responsible for freeing memory allocated by this function.
 *
 * @param list string_list_t containing members to join
 * @param delimiter Optional delimiter string to put between each list
 *        member (NULL to disable)
 * @return Newly allocated string of concatenated list members; caller
 *         must free.
 */
char *list_to_string(const string_list_t *list, const char *delimiter)
{
    size_t pos = 0;
    char *s = NULL;
    string_entry_t *entry = NULL;

    if (list == NULL || TAILQ_EMPTY(list)) {
        return NULL;
    }

    s = strdup("");
    assert(s != NULL);

    TAILQ_FOREACH(entry, list, items) {
        if (pos > 0 && delimiter != NULL) {
            s = strappend(s, delimiter, NULL);
        }

        s = strappend(s, entry->data, NULL);
        pos++;
    }

    return s;
}

/*
 * Lightweight conversion of a string_list_t to an array of pointers
 * to the entry->data strings.  The caller should take care to only
 * free the entry->data pointers once; either with list_free(list,
 * free) or by iterating this array and calling free() on each.  If
 * the latter, then list_free() should still be used on list, but pass
 * NULL as the free func.
 */
char **list_to_array(const string_list_t *list)
{
    int i = 0;
    char **array = NULL;
    string_entry_t *entry = NULL;

    assert(list != NULL);

    array = xcalloc(list_len(list), sizeof(*array));

    TAILQ_FOREACH(entry, list, items) {
        array[i] = entry->data;
        i++;
    }

    return array;
}

/* Convert given string_list_t in a strhash hash table. */
string_map_t *list_to_table(const string_list_t *list)
{
    string_map_t *table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter;

    if (list == NULL) {
        return NULL;
    }

    /* Allocate the table */
    TAILQ_FOREACH(iter, list, items) {
        hentry = (string_map_t *) xalloc(sizeof(*hentry));
        hentry->key = strdup(iter->data);
        hentry->value = strdup(iter->data);
        HASH_ADD_KEYPTR(hh, table, hentry->key, strlen(hentry->key), hentry);
    }

    return table;
}

/* Return a new list of entries that are in list a but are not in list b */
string_list_t *list_difference(const string_list_t *a, const string_list_t *b)
{
    string_map_t *b_table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter = NULL;
    string_list_t *ret = NULL;

    /* Simple cases */
    if ((a == NULL || TAILQ_EMPTY(a)) && (b == NULL || TAILQ_EMPTY(b))) {
        return NULL;
    } else if ((a == NULL || TAILQ_EMPTY(a)) && (b != NULL && !TAILQ_EMPTY(b))) {
        return list_copy(b);
    } else if ((a != NULL && !TAILQ_EMPTY(a)) && (b == NULL || TAILQ_EMPTY(b))) {
        return list_copy(a);
    }

    /* Copy list b into a hash table */
    b_table = list_to_table(b);

    if (b_table == NULL) {
        return NULL;
    }

    /* Iterate through list a looking for things not in list b */
    TAILQ_FOREACH(iter, a, items) {
        HASH_FIND_STR(b_table, iter->data, hentry);

        if (hentry == NULL) {
            ret = list_add(ret, iter->data);
        }
    }

    /* Free the hash table */
    free_string_map(b_table);

    return ret;
}

/* Return a new list of entries that are both in list a and list b */
string_list_t *list_intersection(const string_list_t *a, const string_list_t *b)
{
    string_map_t *b_table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter = NULL;
    string_list_t *ret = NULL;

    /* Copy list b into a hash table */
    b_table = list_to_table(b);

    if (b_table == NULL) {
        return NULL;
    }

    /* Iterate through list a looking for things in list b */
    TAILQ_FOREACH(iter, a, items) {
        HASH_FIND_STR(b_table, iter->data, hentry);

        if (hentry != NULL) {
            ret = list_add(ret, iter->data);
        }
    }

    /* Free the hash table */
    free_string_map(b_table);

    return ret;
}

/* Return a new list of entries that are in either list a or list b */
string_list_t *list_union(const string_list_t *a, const string_list_t *b)
{
    string_map_t *u_table = NULL;
    string_map_t *hentry = NULL;
    const string_entry_t *iter = NULL;
    string_list_t *ret = NULL;

    /*
     * Iterate over both lists, adding each entry to u_table. If it's not already in
     * u_table, add it to the list to be returned.
     */
    TAILQ_FOREACH(iter, a, items) {
        HASH_FIND_STR(u_table, iter->data, hentry);

        if (hentry == NULL) {
            hentry = (string_map_t *) xalloc(sizeof(*hentry));
            hentry->key = strdup(iter->data);
            HASH_ADD_KEYPTR(hh, u_table, hentry->key, strlen(hentry->key), hentry);

            ret = list_add(ret, iter->data);
        }
    }

    TAILQ_FOREACH(iter, b, items) {
        HASH_FIND_STR(u_table, iter->data, hentry);

        if (hentry == NULL) {
            hentry = (string_map_t *) xalloc(sizeof(*hentry));
            hentry->key = strdup(iter->data);
            HASH_ADD_KEYPTR(hh, u_table, hentry->key, strlen(hentry->key), hentry);

            ret = list_add(ret, iter->data);
        }
    }

    free_string_map(u_table);

    return ret;
}

/* Return a new list of entries that in either list a or list b, but not both */
string_list_t * list_symmetric_difference(const string_list_t *a, const string_list_t *b)
{
    string_list_t *a_minus_b = NULL;
    string_list_t *b_minus_a = NULL;
    string_list_t *combination = NULL;

    a_minus_b = list_difference(a, b);

    if (a_minus_b == NULL) {
        return NULL;
    }

    b_minus_a = list_difference(b, a);

    if (b_minus_a == NULL) {
        list_free(a_minus_b, NULL);
        return NULL;
    }

    combination = list_union(a_minus_b, b_minus_a);

    list_free(a_minus_b, NULL);
    list_free(b_minus_a, NULL);

    return combination;
}

/*
 * Helper function to free a string_list_t and each entry->data.  If
 * the free_func is NULL, nothing is done to the entry->data values.
 */
void list_free(string_list_t *list, list_entry_data_free_func free_func)
{
    string_entry_t *entry = NULL;

    if (list == NULL) {
        return;
    }

    while (!TAILQ_EMPTY(list)) {
        entry = TAILQ_FIRST(list);
        TAILQ_REMOVE(list, entry, items);

        if (free_func != NULL) {
            free_func(entry->data);
        }

        free(entry);
    }

    free(list);
    return;
}

static int compare_entries(string_map_t *data1, string_map_t *data2)
{
    return strcmp(data1->key, data2->key);
}

/*
 * Return a sorted copy of the list.
 *
 * The data pointers used by the sorted list entries are the same as those
 * used in the original list.
 */
string_list_t *list_sort(const string_list_t *list)
{
    string_list_t *sorted_list = NULL;
    string_entry_t *iter = NULL;
    string_map_t *map = NULL;
    string_map_t *entry = NULL;
    string_map_t *tmp_entry = NULL;

    /* create a sorted hash table of the entries */
    TAILQ_FOREACH(iter, list, items) {
        entry = (string_map_t *) xalloc(sizeof(*entry));
        entry->key = strdup(iter->data);
        assert(entry->key != NULL);
        HASH_ADD_KEYPTR_INORDER(hh, map, &entry->key, strlen(entry->key), entry, compare_entries);
    }

    /* build a new string_list_t from the sorted hash table */
    HASH_ITER(hh, map, entry, tmp_entry) {
        HASH_DEL(map, entry);
        sorted_list = list_add(sorted_list, entry->key);
        free(entry->key);
        free(entry);
    }

    free(map);
    return sorted_list;
}

/* Returns the number of entries in the list */
size_t list_len(const string_list_t *list)
{
    string_entry_t *iter = NULL;
    size_t len = 0;

    if (list == NULL || TAILQ_EMPTY(list)) {
        return len;
    }

    TAILQ_FOREACH(iter, list, items) {
        len++;
    }

    return len;
}

/*
 * Returns a malloc'ed copy of the given list.  Caller must use
 * list_free(list, free) on the returned list.
 */
string_list_t * list_copy(const string_list_t *list)
{
    const string_entry_t *iter = NULL;
    string_list_t *result = NULL;

    if (list == NULL) {
        return NULL;
    }

    TAILQ_FOREACH(iter, list, items) {
        result = list_add(result, iter->data);
    }

    return result;
}

/**
 * Given a NULL terminated array of strings, create a newly allocated
 * string_list_t containing all of the array members.  Returned the
 * new list.  Caller is responsible for freeing memory allocated by
 * this function.
 *
 * @param array NULL terminated array of strings.
 * @return Newly allocated string_list_t representing the array.
 */
string_list_t *list_from_array(const char **array)
{
    int i = 0;
    string_list_t *list = NULL;

    assert(array != NULL);

    for (i = 0; array[i] != NULL; i++) {
        list = list_add(list, array[i]);
    }

    return list;
}

/*
 * Return true if the given string list contains the given string.  It
 * compares string values, so the string in the list need not be the
 * same string requested.
 */
bool list_contains(const string_list_t *list, const char *s)
{
    string_entry_t *entry = NULL;

    if (list == NULL || TAILQ_EMPTY(list)) {
        return false;
    }

    if (s == NULL) {
        return false;
    }

    TAILQ_FOREACH(entry, list, items) {
        if (!strcmp(entry->data, s)) {
            return true;
        }
    }

    return false;
}

/*
 * Append the string to the string_list_t and return the
 * string_list_t.  A NULL string is not added and the caller just gets
 * back a pointer to the same string_list_t.  A NULL list may be
 * specified, in which case the function will start a new list and add
 * the string to it.  Caller responsible for all memory management.
 */
string_list_t *list_add(string_list_t *list, const char *s)
{
    string_entry_t *entry = NULL;

    if (s == NULL || (s && !strcmp(s, ""))) {
        return list;
    }

    if (list == NULL) {
        list = (string_list_t *) xalloc(sizeof(*list));
        TAILQ_INIT(list);
    }

    entry = (string_entry_t *) xalloc(sizeof(*entry));
    entry->data = strdup(s);
    assert(entry->data != NULL);
    TAILQ_INSERT_TAIL(list, entry, items);

    return list;
}

/*
 * Remove an entry matching the string 's'.
 */
void list_remove(string_list_t *list, const char *s)
{
    string_entry_t *entry = NULL;

    if (list == NULL || s == NULL) {
        return;
    }

    TAILQ_FOREACH(entry, list, items) {
        if (!strcmp(entry->data, s)) {
            TAILQ_REMOVE(list, entry, items);
            free(entry->data);
            free(entry);
        }
    }

    return;
}

/*
 * Given a string_list_t, trim out any empty list members.
 * Optionally, trim out any list members with a string prefix matching
 * prefix.  Modifies the list in place and returns a pointer to the
 * list.
 */
string_list_t *list_trim(string_list_t *list, const char *prefix)
{
    string_entry_t *entry = NULL;
    string_entry_t *next = NULL;

    if (list == NULL) {
        return list;
    }

    if (list_len(list)) {
        entry = TAILQ_FIRST(list);

        while (entry) {
            next = TAILQ_NEXT(entry, items);

            if (entry->data == NULL) {
                TAILQ_REMOVE(list, entry, items);
                free(entry);
            } else if (!strcmp(entry->data, "") || (prefix && strprefix(entry->data, prefix))) {
                TAILQ_REMOVE(list, entry, items);
                free(entry->data);
                free(entry);
            }

            entry = next;
        }
    }

    if (TAILQ_EMPTY(list)) {
        free(list);
        list = NULL;
    }

    return list;
}
