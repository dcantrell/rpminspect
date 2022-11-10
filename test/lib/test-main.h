/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _LIBRPMINSPECT_TEST_MAIN_H
#define _LIBRPMINSPECT_TEST_MAIN_H

#include <CUnit/Basic.h>
#include <stdint.h>
#include <string.h>

/* Each test module needs to export this function */
CU_pSuite get_suite(void);

/* Improved assertions: show what "actual" evaluates to, instead of just displaying the cpp token */
CU_BOOL RI_assert_impl(CU_BOOL, unsigned int, const char *, const char *, ...)
    __attribute__ ((format(printf, 4, 5)));

/*
 * These assertions from CUnit are fine, since it's obvious what the result of the expression is (true, not true).
 * Provide redefinitions so we don't have a mix of CI_ASSERT_* and RI_ASSERT_*.
 */
#define RI_ASSERT       CU_ASSERT
#define RI_ASSERT_TRUE  CU_ASSERT_TRUE
#define RI_ASSERT_FALSE CU_ASSERT_FALSE

/* For the rest of the assertions, provide new wrappers around CU_assertImplementation (via RI_assert_impl)
 * that replaces the "strCondition" argument with what the expression actually evaluates to.
 * i.e., instead of displaying `CU_ASSERT_STRING_EQUAL(actual, expected)` as the test failure,
 * display `Assertion 'strcmp(expected, actual) == 0' failed: expected == "foo", actual == "bar"`
 *
 * Macros are based on the assertions from libcheck.
 */

/* always store "expected" and "actual" in new variables so they only get evaluated once. */
#define _RI_ASSERT_INT(X, OP, Y) { \
    intmax_t _x = (X);\
    intmax_t _y = (Y);\
    RI_assert_impl(_x OP _y, __LINE__, __FILE__, "Assertion '%s' failed: %s == %jd, %s == %jd", #X" "#OP" "#Y, #X, _x, #Y, _y);\
}

#define _RI_ASSERT_PTR(X, OP, Y) {\
    void *_x = (X);\
    void *_y = (Y);\
    RI_assert_impl(_x OP _y, __LINE__, __FILE__, "Assertion '%s' failed: %s == %p, %s == %p", #X" "#OP" "#Y, #X, _x, #Y, _y);\
}

#define _RI_ASSERT_STR(X, OP, Y) {\
    const char *_x = (X);\
    const char *_y = (Y);\
    RI_assert_impl(strcmp(_x, _y) OP 0, __LINE__, __FILE__, "Assertion '%s' failed: %s == \"%s\", %s == \"%s\"", #X" "#OP" "#Y, #X, _x, #Y, _y);\
}

#define RI_ASSERT_EQUAL(actual, expected)               _RI_ASSERT_INT(expected, ==, actual)
#define RI_ASSERT_NOT_EQUAL(actual, expected)           _RI_ASSERT_INT(expected, !=, actual)

#define RI_ASSERT_PTR_NULL(actual)                      _RI_ASSERT_PTR(NULL, ==, actual)
#define RI_ASSERT_PTR_NOT_NULL(actual)                  _RI_ASSERT_PTR(NULL, !=, actual)

#define RI_ASSERT_STRING_EQUAL(actual, expected)        _RI_ASSERT_STR(expected, ==, actual)
#define RI_ASSERT_STRING_NOT_EQUAL(actual, expected)    _RI_ASSERT_STR(expected, !=, actual)

#define ASSERT_AND_FREE(expr, expected) {\
    char *actual = (expr);\
    RI_ASSERT_STRING_EQUAL(actual, expected);\
    free(actual);\
}

#endif
