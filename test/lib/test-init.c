/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <CUnit/Basic.h>
#include "rpminspect.h"
#include "test-main.h"

int init_test_init(void) {
    return 0;
}

int clean_test_init(void) {
    return 0;
}

void test_init_rpminspect(void) {
    struct rpminspect *ri = NULL;

    ri = init_rpminspect(ri, NULL, NULL);
    RI_ASSERT_PTR_NOT_NULL(ri);
    free_rpminspect(ri);
    return;
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("init", init_test_init, clean_test_init);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test init_rpminspect()", test_init_rpminspect) == NULL) {
        return NULL;
    }

    return pSuite;
}
