/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <err.h>
#include "rpminspect.h"

/*
 * Returns true if the specified file is LLVM IR Bitcode, false
 * otherwise.
 */
bool is_llvm_ir_bitcode(const char *file)
{
    int fd;
    int flags = O_RDONLY | O_CLOEXEC;
    unsigned char target[4] = { '\x42', '\x43', '\xC0', '\xDE' };
    unsigned char magic[4];

    assert(file != NULL);

    /* read the first 4 bytes of the file in question */
#ifdef O_LARGEFILE
    flags |= O_LARGEFILE;
#endif
    fd = open(file, flags);

    if (fd == -1) {
        warn("*** open");
        return false;
    }

    if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
        if (close(fd) == -1) {
            warn("*** close");
        }

        return false;
    }

    if (close(fd) == -1) {
        warn("*** close");
        return false;
    }

    /* LLVM IR bitcode files begin with 0x4243C0DE */
    if ((magic[0] == target[0]) && (magic[1] == target[1]) && (magic[2] == target[2]) && (magic[3] == target[3])) {
        return true;
    }

    return false;
}
