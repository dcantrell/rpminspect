/* {{{ Apache License version 2.0
 */
/*
 * Copyright 2004-2020 David Shea <david@reallylongword.org>
 *                     Chris Lumens <chris@bangmoney.org>
 *                     David Cantrell <david.l.cantrell@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* }}} */

/**
 * @file checksums.c
 * @author David Cantrell &lt;david.l.cantrell@gmail.com&gt;
 * @author Chris Lumens &lt;chris@bangmoney.org&gt;
 * @author David Shea &lt;david@reallylongword.org&gt;
 * @date 2004-2020
 * @brief Calculate an MD5, SHA-1, SHA-224, SHA-256, SHA-384, or SHA-512 checksum for a file.
 * @copyright Apache-2.0
 */

/*
 * XXX: temporary until we have support for the OpenSSL 3.0 API
 */
#define OPENSSL_API_COMPAT 0x101010bfL

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include "rpminspect.h"

/**
 * @brief Take in a file, return a checksum.
 *
 * Given a file, its **mode_t**, and a valid checksum type, compute
 * the checksum and return the human-readable digest string for that
 * checksum.  This function allocates memory for the string and the
 * caller must free it when done.
 *
 * @param filename Filename the function should use.
 * @param st_mode The **mode_t** for the specified file, gathered from **stat(2)**.
 * @param type Which checksum type to calculate.
 * @note Caller must free returned string when done.
 * @return String containing the human-readable checksum digest, or NULL on failure.
 */
char *compute_checksum(const char *filename, mode_t *st_mode, int type)
{
    struct stat sb;
    mode_t *mode = NULL;
    unsigned char buf[BUFSIZ];
    int input, len, i;
    char *ret = NULL;
    unsigned char digest[BUFSIZ];
    MD5_CTX md5c;
    SHA_CTX sha1c;
    SHA256_CTX sha256c;
    SHA512_CTX sha512c;

    /* if the user did not provide a mode_t, get it */
    if (st_mode == NULL) {
        if (lstat(filename, &sb) != 0) {
            return NULL;
        }

        mode = &sb.st_mode;
    } else {
        mode = st_mode;
    }

    /* don't calculate the checksum of a device node */
    if (S_ISCHR(*mode) || S_ISBLK(*mode) || S_ISFIFO(*mode) || S_ISSOCK(*mode)) {
        warnx(_("*** %s is a FIFO"), filename);
        return NULL;
    }

    /*
     * This holds all possible digest type computations.  if any future digest
     * is larger than BUFSIZ, hopefully we update this and get faster systems.
     */
    memset(&buf, '\0', BUFSIZ);
    memset(&digest, '\0', BUFSIZ);

    /* Initialize the correct context based on the checksum type. */
    if (type == MD5SUM) {
        MD5_Init(&md5c);
    } else if (type == SHA1SUM) {
        SHA1_Init(&sha1c);
    } else if (type == SHA224SUM || type == SHA256SUM) {
        SHA256_Init(&sha256c);
    } else if (type == SHA384SUM || type == SHA512SUM) {
        SHA512_Init(&sha512c);
    }

    /* read in the file to generate the requested checksum */
    input = open(filename, O_RDONLY);

    if (input == -1) {
        warn("*** open");
        return NULL;
    }

    len = read(input, buf, sizeof(buf));

    if (len == -1) {
        warn("*** read");
        return NULL;
    }

    while (len > 0) {
        /* update the correct context based on the checksum type */
        if (type == MD5SUM) {
            MD5_Update(&md5c, buf, len);
        } else if (type == SHA1SUM) {
            SHA1_Update(&sha1c, buf, len);
        } else if (type == SHA224SUM || type == SHA256SUM) {
            SHA256_Update(&sha256c, buf, len);
        } else if (type == SHA384SUM || type == SHA512SUM) {
            SHA512_Update(&sha512c, buf, len);
        }

        len = read(input, buf, sizeof(buf));

        if (len == -1) {
            warn("*** read");
            return NULL;
        }
    }

    if (close(input) == -1) {
        warn("*** close");
        return NULL;
    }

    /* finalize the correct context type and determine the loop length */
    if (type == MD5SUM) {
        MD5_Final(digest, &md5c);
    } else if (type == SHA1SUM) {
        SHA1_Final(digest, &sha1c);
    } else if (type == SHA224SUM || type == SHA256SUM) {
        SHA256_Final(digest, &sha256c);
    } else if (type == SHA384SUM || type == SHA512SUM) {
        SHA512_Final(digest, &sha512c);
    }

    if (type == MD5SUM) {
        len = MD5_DIGEST_LENGTH;
    } else if (type == SHA1SUM) {
        len = SHA_DIGEST_LENGTH;
    } else if (type == SHA224SUM) {
        len = SHA224_DIGEST_LENGTH;
    } else if (type == SHA256SUM) {
        len = SHA256_DIGEST_LENGTH;
    } else if (type == SHA384SUM) {
        len = SHA384_DIGEST_LENGTH;
    } else if (type == SHA512SUM) {
        len = SHA512_DIGEST_LENGTH;
    }

    /* this is our human readable digest, caller must free */
    ret = (char *) xcalloc(len + 1, sizeof(char *));

    for (i = 0; i < len; ++i) {
        sprintf(&ret[i*2], "%02x", (unsigned int) digest[i]);
    }

    return ret;
}

/**
 * @brief Return checksum string of the given **rpmfile_entry_t**.
 *
 * The **rpmfile_entry_t** will contain a cached checksum string or
 * not.  If it does, this function returns the cached string.  If the
 * string is NULL, this function calculates the checksum, caches it,
 * and returns the string.
 *
 * @param file The **rpmfile_entry_t** specifying the file to use.
 * @note Do not free the result returned, that is handled by
 *       **free_files()**.
 * @return String containing the human-readable checksum digest, or
 *         NULL on failure.
 */
char *checksum(rpmfile_entry_t *file)
{
    assert(file != NULL);

    if (file->checksum) {
        return file->checksum;
    }

    file->checksum = compute_checksum(file->fullpath, &file->st_mode, DEFAULT_MESSAGE_DIGEST);
    return file->checksum;
}
