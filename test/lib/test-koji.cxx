/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <CUnit/Basic.h>
#include "rpminspect.h"

#include "test-main.h"

struct koji_build *build = NULL;
koji_rpmlist_t *list = NULL;

int init_test_koji(void) {
    return 0;
}

int clean_test_koji(void) {
    return 0;
}

void test_init_koji_build(void) {
    build = calloc(1, sizeof(*build));
    RI_ASSERT_PTR_NOT_NULL(build);
    init_koji_build(build);
    RI_ASSERT_PTR_NOT_NULL(build);
}

void test_init_koji_rpmlist(void) {
    list = init_koji_rpmlist();
    RI_ASSERT_PTR_NOT_NULL(list);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("koji", init_test_koji, clean_test_koji);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test init_koji_build()", test_init_koji_build) == NULL ||
        CU_add_test(pSuite, "test init_koji_rpmlist()", test_init_koji_rpmlist) == NULL) {
        return NULL;
    }

    return pSuite;
}
