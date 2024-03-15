/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <unistd.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"

#include "test-main.h"

int init_test_tty(void) {
    return 0;
}

int clean_test_tty(void) {
    return 0;
}

void test_tty_width(void) {
    size_t w;
    w = tty_width();
    RI_ASSERT(w > 0);
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;
    CU_pTest pTtyWidthTest = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("tty", init_test_tty, clean_test_tty);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if ((pTtyWidthTest = CU_add_test(pSuite, "test tty_width()", test_tty_width)) == NULL) {
        return NULL;
    }

    /* Only run the tty_width test if we have a tty */
    if (!isatty(STDIN_FILENO)) {
        CU_set_test_active(pTtyWidthTest, CU_FALSE);
    }

    return pSuite;
}
