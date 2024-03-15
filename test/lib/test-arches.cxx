/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <CUnit/Basic.h>
#include "rpminspect.h"
#include "test-main.h"

struct rpminspect *ri = NULL;

int init_test_arches(void) {
    ri = init_rpminspect(ri, NULL, NULL);
    ri->arches = list_add(ri->arches, "x86_64");
    return 0;
}

int clean_test_arches(void) {
    free_rpminspect(ri);
    return 0;
}

void test_allowed_arch(void) {
    RI_ASSERT_TRUE(allowed_arch(ri, "x86_64"));
    RI_ASSERT_FALSE(allowed_arch(ri, "RISC-V"));
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("arches", init_test_arches, clean_test_arches);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test allowed_arch()", test_allowed_arch) == NULL) {
        return NULL;
    }

    return pSuite;
}
