/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <CUnit/Basic.h>
#include "rpminspect.h"
#include "test-main.h"

int init_test_inspect_elf(void)
{
    int r = 0;
    struct rpminspect *ri = NULL;

    ri = init_rpminspect(NULL, NULL, NULL);

    if (elf_version(EV_CURRENT) == EV_NONE) {
        r = -1;
    }

    free_rpminspect(ri);

    return r;
}

int clean_test_inspect_elf(void) {
    /* NO OP */
    return 0;
}

void test_is_execstack_present(void) {
    int fd;
    Elf *elf;

    /* expected true */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(is_execstack_present(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    /* expected false */
    fd = open(_BUILDDIR_"/noexecstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(is_execstack_present(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_get_execstack_flags(void) {
    int fd;
    Elf *elf;

    /* expected non-zero return */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_EQUAL(get_execstack_flags(elf), PF_X|PF_W|PF_R);

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    /* expected zero return */
    fd = open(_BUILDDIR_"/noexecstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_EQUAL(get_execstack_flags(elf), PF_W|PF_R);

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_has_executable_program(void) {
    int fd;
    Elf *elf;

    /* expected true */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(has_executable_program(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_is_execstack_valid(void) {
    int fd;
    Elf *elf;

    /* expect true */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(is_execstack_valid(elf, get_execstack_flags(elf)));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_is_stack_executable(void) {
    int fd;
    Elf *elf;

    /* expect true */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(is_stack_executable(elf, get_execstack_flags(elf)));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    /* expect false */
    fd = open(_BUILDDIR_"/noexecstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_FALSE(is_stack_executable(elf, get_execstack_flags(elf)));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_has_textrel(void) {
    int fd;
    Elf *elf;

    /* expect false */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_FALSE(has_textrel(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_has_relro(void) {
    int fd;
    Elf *elf;

    /* expect true */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(has_relro(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_has_bind_now(void) {
    int fd;
    Elf *elf;

    /* expect false */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_FALSE(has_bind_now(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

void test_get_fortified_symbols(void) {
    /* XXX */
    return;
}

void test_get_fortifiable_symbols(void) {
    /* XXX */
    return;
}

void test_is_pic_ok(void) {
    int fd;
    Elf *elf;

    /* XXX */
    return;

    /* expect true */
    fd = open(_BUILDDIR_"/execstack", O_RDONLY);
    RI_ASSERT_NOT_EQUAL(fd, -1);

    elf = elf_begin(fd, ELF_C_READ_MMAP_PRIVATE, NULL);
    RI_ASSERT_PTR_NOT_NULL(elf);

    RI_ASSERT_TRUE(is_pic_ok(elf));

    RI_ASSERT_EQUAL(elf_end(elf), 0);
    RI_ASSERT_EQUAL(close(fd), 0);

    return;
}

CU_pSuite get_suite(void) {
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("inspect_elf", init_test_inspect_elf, clean_test_inspect_elf);
    if (pSuite == NULL) {
        return NULL;
    }

    /* add tests to the suite */
    if (CU_add_test(pSuite, "test is_execstack_present()", test_is_execstack_present) == NULL ||
        CU_add_test(pSuite, "test get_execstack_flags()", test_get_execstack_flags) == NULL ||
        CU_add_test(pSuite, "test has_executable_program()", test_has_executable_program) == NULL ||
        CU_add_test(pSuite, "test is_execstack_valid()", test_is_execstack_valid) == NULL ||
        CU_add_test(pSuite, "test is_stack_executable()", test_is_stack_executable) == NULL ||
        CU_add_test(pSuite, "test has_textrel()", test_has_textrel) == NULL ||
        CU_add_test(pSuite, "test has_relro()", test_has_relro) == NULL ||
        CU_add_test(pSuite, "test has_bind_now()", test_has_bind_now) == NULL ||
        CU_add_test(pSuite, "test get_fortified_symbols()", test_get_fortified_symbols) == NULL ||
        CU_add_test(pSuite, "test get_fortifiable_symbols()", test_get_fortifiable_symbols) == NULL ||
        CU_add_test(pSuite, "test is_pic_ok()", test_is_pic_ok) == NULL) {
        return NULL;
    }

    return pSuite;
}
