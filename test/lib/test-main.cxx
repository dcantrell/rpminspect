/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <CUnit/Basic.h>

#include "test-main.h"

/* Special exit codes used by automake */
#define EXIT_SKIP       (77)
#define EXIT_HARD_ERROR (99)

/* Wrappers around CU_assertImplementation to print better messages */
CU_BOOL RI_assert_impl(CU_BOOL value, unsigned int line, const char *file, const char *format, ...)
{
    va_list ap;
    char *msg = NULL;

    va_start(ap, format);

    if (vasprintf(&msg, format, ap) == -1) {
        CU_assertImplementation(CU_FALSE, line, "*** Unable to allocate memory for assertion", file, "", CU_TRUE);
    } else {
        CU_assertImplementation(value, line, msg, file, "", CU_FALSE);
    }

    va_end(ap);
    free(msg);
    return value;
}

/* Given a function that returns a test suite, initialize the test registry,
 * run the test suite, cleanup, and exit with the appropriate error code */
int main(void)
{
    CU_pSuite pSuite;
    unsigned int failures;
    unsigned int tests_run;
    unsigned int tests_skipped;

    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf(stderr, "*** Unable to initialize test registry: %s\n", CU_get_error_msg());
        return EXIT_HARD_ERROR;
    }

    /* Don't fail on inactive tests */
    CU_set_fail_on_inactive(CU_FALSE);

    pSuite = get_suite();
    if (pSuite == NULL) {
        fprintf(stderr, "*** Unable to initialize test suite: %s\n", CU_get_error_msg());
        CU_cleanup_registry();
        return EXIT_HARD_ERROR;
    }

    /* run all tests using the CUnit basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    if (CU_basic_run_tests() != CUE_SUCCESS) {
        fprintf(stderr, "*** Error running tests: %s\n", CU_get_error_msg());
        CU_cleanup_registry();
        return EXIT_HARD_ERROR;
    }

    failures = CU_get_number_of_tests_failed();
    tests_run = CU_get_number_of_tests_run();
    tests_skipped = CU_get_number_of_tests_inactive();

    CU_cleanup_registry();

    /* If any tests failed, return failure */
    if (failures > 0) {
        return EXIT_FAILURE;
    }

    /* If all tests were skipped, return EXIT_SKIP */
    if ((tests_run == 0) && (tests_skipped > 0)) {
        return EXIT_SKIP;
    }

    return EXIT_SUCCESS;
}
