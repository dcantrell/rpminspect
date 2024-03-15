/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <CUnit/Basic.h>
#include "rpminspect.h"

#include "test-main.h"


int init_test_humansize(void) {
    return 0;
}

int clean_test_humansize(void) {
    return 0;
}

void test_humansize(void) {
    RI_ASSERT_EQUAL(strncmp(human_size(12), "12 B", 4), 0);
    RI_ASSERT_EQUAL(strncmp(human_size(12288), "12 KiB", 6), 0);
    RI_ASSERT_EQUAL(strncmp(human_size(12582912), "12 MiB", 6), 0);
    RI_ASSERT_EQUAL(strncmp(human_size(3221225472), "3 GiB", 5), 0);
    RI_ASSERT_NOT_EQUAL(strncmp(human_size(13), "12 B", 4), 0);
    RI_ASSERT_NOT_EQUAL(strncmp(human_size(13288), "12 KiB", 6), 0);
    RI_ASSERT_NOT_EQUAL(strncmp(human_size(13582912), "12 MiB", 6), 0);
    RI_ASSERT_NOT_EQUAL(strncmp(human_size(2147483648), "3 GiB", 5), 0);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("humansize", init_test_humansize, clean_test_humansize);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test human_size()", test_humansize) == NULL) {
        return NULL;
    }

    return pSuite;
}
