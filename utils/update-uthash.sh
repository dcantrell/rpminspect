#!/bin/sh
#
# Pull down the latest uthash.h file from upstream
#

PATH=/bin:/usr/bin
TMPDIR="$(mktemp -d)"
DESTDIR="${1}"

trap 'rm -rf ${TMPDIR}' EXIT

if [ ! -d "${DESTDIR}" ]; then
    echo "*** ${DESTDIR} is not a directory" >&2
    exit 1
fi

cd "${TMPDIR}" || exit 1
git clone https://github.com/troydhanson/uthash.git
cp -p uthash/src/uthash.h "${DESTDIR}"/uthash.h

exit 0
