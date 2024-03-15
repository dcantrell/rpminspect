/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>

int some_function(void) {
    return 47;
}

int main(void) {
    char *s = "Hello, world.";
    printf("%s\n", s);
    return EXIT_SUCCESS;
}
