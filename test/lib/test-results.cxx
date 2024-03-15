/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <CUnit/Basic.h>
#include "rpminspect.h"
#include "test-main.h"

struct result_params params_emptyrpm;
struct result_params params_license;
struct rpminspect *ri = NULL;

int init_test_results(void) {
    ri = init_rpminspect(ri, NULL, NULL);

    init_result_params(&params_emptyrpm);
    params_emptyrpm.severity = RESULT_DIAG;
    params_emptyrpm.waiverauth = WAIVABLE_BY_ANYONE;
    params_emptyrpm.header = NAME_EMPTYRPM;

    init_result_params(&params_license);
    params_license.severity = RESULT_SKIP;
    params_license.waiverauth = WAIVABLE_BY_SECURITY;
    params_license.header = NAME_LICENSE;

    return 0;
}

int clean_test_results(void) {
    free_rpminspect(ri);
    return 0;
}

void test_init_results(void) {
    ri->results = init_results();
    RI_ASSERT_PTR_NOT_NULL(ri->results);
    RI_ASSERT_TRUE(TAILQ_EMPTY(ri->results));
    return;
}

void test_add_result_entry(void) {
    add_result_entry(&ri->results, &params_emptyrpm);
    RI_ASSERT_FALSE(TAILQ_EMPTY(ri->results));

    RI_ASSERT_STRING_EQUAL(NAME_EMPTYRPM, TAILQ_LAST(ri->results, results_s)->header);
    RI_ASSERT_EQUAL(RESULT_DIAG, TAILQ_LAST(ri->results, results_s)->severity);
    RI_ASSERT_EQUAL(WAIVABLE_BY_ANYONE, TAILQ_LAST(ri->results, results_s)->waiverauth);
    return;
}

void test_add_result(void) {
    add_result(ri, &params_license);
    RI_ASSERT_FALSE(TAILQ_EMPTY(ri->results));

    RI_ASSERT_STRING_EQUAL(NAME_LICENSE, TAILQ_LAST(ri->results, results_s)->header);
    RI_ASSERT_EQUAL(RESULT_SKIP, TAILQ_LAST(ri->results, results_s)->severity);
    RI_ASSERT_EQUAL(WAIVABLE_BY_SECURITY, TAILQ_LAST(ri->results, results_s)->waiverauth);
    return;
}

void test_suppressed_results(void) {
    RI_ASSERT_FALSE(suppressed_results(ri->results, NAME_LICENSE, RESULT_NULL));
    RI_ASSERT_TRUE(suppressed_results(ri->results, NAME_EMPTYRPM, RESULT_OK));
    RI_ASSERT_FALSE(suppressed_results(ri->results, NAME_DIAGNOSTICS, RESULT_INFO));
    return;
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("results", init_test_results, clean_test_results);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test init_results()", test_init_results) == NULL ||
        CU_add_test(pSuite, "test add_result_entry()", test_add_result_entry) == NULL ||
        CU_add_test(pSuite, "test add_result()", test_add_result) == NULL ||
        CU_add_test(pSuite, "test suppressed_results()", test_suppressed_results) == NULL) {
        return NULL;
    }

    return pSuite;
}
