/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"

#include "test-main.h"

int init_test_abspath(void) {
    return 0;
}

int clean_test_abspath(void) {
    return 0;
}

void test_abspath(void) {
    char *r = NULL;

    r = abspath("/../lib/jli");
    RI_ASSERT_PTR_NOT_NULL(r);

    if (r) {
        RI_ASSERT_EQUAL(strcmp(r, "/lib/jli"), 0);
        free(r);
    }

    r = abspath("/../lib64/");
    RI_ASSERT_PTR_NOT_NULL(r);

    if (r) {
        RI_ASSERT_EQUAL(strcmp(r, "/lib64"), 0);
        free(r);
    }

    r = abspath("/usr/lib/../lib64/../lib");
    RI_ASSERT_PTR_NOT_NULL(r);

    if (r) {
        RI_ASSERT_EQUAL(strcmp(r, "/usr/lib"), 0);
        free(r);
    }

    r = abspath("/usr/lib/../lib64/../lib/");
    RI_ASSERT_PTR_NOT_NULL(r);

    if (r) {
        RI_ASSERT_EQUAL(strcmp(r, "/usr/lib"), 0);
        free(r);
    }

    return;
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("abspath", init_test_abspath, clean_test_abspath);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test abspath()", test_abspath) == NULL) {
        return NULL;
    }

    return pSuite;
}
