#!/bin/sh
#
# Run filesmatch in debug mode against the RHIVOS SRPMs
#

PATH=/usr/bin:${HOME}/bin
CWD="$(pwd)"

if [ ! -d "${CWD}"/rhivos ]; then
    echo "*** Missing RHIVOS subdirectory, cannot continue." >&2
    exit 1
fi

if [ ! -f "${CWD}"/rhivos/list.txt ]; then
    echo "*** Missing RHIVOS package list.txt file, cannot continue." >&2
    exit 1
fi

cd "${CWD}"/rhivos || exit 1

while read -r srpm ; do
    rh-rpminspect -T filesmatch -d -o "${CWD}"/rhivos/"${srpm}"/results.log "${srpm}" 2>&1 | tee "${CWD}"/rhivos/"${srpm}"/filesmatch.log
done < "${CWD}"/rhivos/list.txt
