/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <unistd.h>

/*
 * Write *all* of the supplied buffer out to a fd.
 * Do multiple writes if necessary.
 * Returns the amount written, or -1 if error was seen
 * on the very first write.
 * The write will be incomplete only if an error occurs
 * on one of subsequent writes.
 */
ssize_t full_write(int fd, const void *buf, size_t len)
{
    ssize_t total = 0;

    while (len != 0) {
        ssize_t cc = write(fd, buf, len);

        if (cc < 0) {
            if (total) {
                /* we already wrote some! */
                /* user can do another write to know the error code */
                return total;
            }

            return cc;  /* write() returns -1 on failure. */
        }

        total += cc;
        buf = ((const char *)buf) + cc;
        len -= cc;
    }

    return total;
}
