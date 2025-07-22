#!/bin/sh
#
# Sync filesmatch results in debug mode against the RHIVOS SRPMs to my
# public_html directory.
#

PATH=/usr/bin:${HOME}/bin
CWD="$(pwd)"
DEST="${HOME}"/public_html/filesmatch/rhivos

if [ ! -d "${CWD}"/rhivos ]; then
    echo "*** Missing RHIVOS subdirectory, cannot continue." >&2
    exit 1
fi

if [ ! -d "${DEST}" ]; then
    echo "*** Missing ${DEST}, cannot continue." >&2
    exit 1
fi

cd "${CWD}"/rhivos || exit 1
rsync -avz --delete --progress . "${HOME}"/public_html/filesmatch/rhivos/
