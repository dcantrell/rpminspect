/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "rpminspect.h"

/*
 * Return the terminal width.  Used by output routines sending text to
 * stdout.  This function returns DEFAULT_TTY_WIDTH if it cannot
 * figure out the width.
 */
size_t tty_width(void)
{
    struct winsize w;

    /* get the terminal size */
    if (ioctl(0, TIOCGWINSZ, &w) == -1) {
        /*
         * We couldn't determine the real size,
         * so let's go with the default width.
         */
        return DEFAULT_TTY_WIDTH;
    }

    return w.ws_col;
}
