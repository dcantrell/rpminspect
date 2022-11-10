/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"

#include "test-main.h"

string_list_t *forbidden_words = NULL;

int init_test_badwords(void) {
    string_entry_t *entry;

    /* create a list of forbidden words */
    if ((forbidden_words = malloc(sizeof(*forbidden_words))) == NULL) {
        return -1;
    }

    TAILQ_INIT(forbidden_words);

    /* add test words */
    if ((entry = calloc(1, sizeof(*entry))) == NULL) {
        return -1;
    }

    entry->data = strdup("foo");
    TAILQ_INSERT_TAIL(forbidden_words, entry, items);

    if ((entry = calloc(1, sizeof(*entry))) == NULL) {
        return -1;
    }

    entry->data = strdup("bar");
    TAILQ_INSERT_TAIL(forbidden_words, entry, items);

    if ((entry = calloc(1, sizeof(*entry))) == NULL) {
        return -1;
    }

    entry->data = strdup("baz");
    TAILQ_INSERT_TAIL(forbidden_words, entry, items);

    if ((entry = calloc(1, sizeof(*entry))) == NULL) {
        return -1;
    }

    entry->data = strdup("qux");
    TAILQ_INSERT_TAIL(forbidden_words, entry, items);

    return 0;
}

int clean_test_badwords(void) {
    list_free(forbidden_words, free);
    return 0;
}

void test_has_bad_word(void) {
    RI_ASSERT(has_bad_word("foo", forbidden_words) == true);
    RI_ASSERT(has_bad_word("bar", forbidden_words) == true);
    RI_ASSERT(has_bad_word("baz", forbidden_words) == true);
    RI_ASSERT(has_bad_word("qux", forbidden_words) == true);
    RI_ASSERT(has_bad_word("flargenblarfle", forbidden_words) == false);
    RI_ASSERT(has_bad_word("cocacola", forbidden_words) == false);
    RI_ASSERT(has_bad_word("suse", forbidden_words) == false);
    RI_ASSERT(has_bad_word("supermonkeyball", forbidden_words) == false);

    /* Ensure bad words match at the start or end of a word, but not the middle */
    RI_ASSERT(has_bad_word("bazzing", forbidden_words) == true);
    RI_ASSERT(has_bad_word("is bazzing", forbidden_words) == true);
    RI_ASSERT(has_bad_word("motherbaz", forbidden_words) == true);
    RI_ASSERT(has_bad_word("motherbaz other words", forbidden_words) == true);
    RI_ASSERT(has_bad_word("bebazzled", forbidden_words) == false);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("badwords", init_test_badwords, clean_test_badwords);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test has_bad_word()", test_has_bad_word) == NULL) {
        return NULL;
    }

    return pSuite;
}
