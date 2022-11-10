#!/bin/sh
# Helper script to test rpminspect builds in copr during build
#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Arguments:
# $1   Full path to rpminspect command to use
# $2   Full path to rpminspect configuration file to use (optional)
#

PATH=/bin:/usr/bin
CMD="$(basename "${0}")"
RPMINSPECT="${1}"

usage() {
    echo "Usage: ${CMD} [exec]"
    echo "Arguments:"
    echo "    exec     Full path to 'rpminspect' executable"
}

if [ ! -x "${RPMINSPECT}" ]; then
    usage
    exit 1
fi

# trap sig 11
trap failure SEGV
failure() {
    echo "ERROR: Caught Signal 11: Aborting" >&2
    exit 1
}

# Packages to diff to consider this build stable-enough
packages="zsh kernel python3 firefox mutt emacs tmux elfutils"

# Validate programs we need are present
reqs="rpmbuild koji"
for req in $reqs; do
    if ! $req --help > /dev/null 2>&1; then
        echo "ERROR: Required program $req missing." >&2
        exit 1
    fi
done

# Find our build environment (fc32 for example)
dist=$(rpmbuild --eval "%{dist}")
if [ -z "${dist}" ] || [ "${dist}" = "%{dist}" ]; then
    echo "ERROR: Unable to find build environment." >&2
    exit 1
fi

# Set up rpminspect-data-fedora environment
trap 'rm -rf "${TMPDATADIR}"' EXIT
TMPDATADIR="$(mktemp -d)"
cd "${TMPDATADIR}" || exit 1
git clone https://github.com/rpminspect/rpminspect-data-fedora.git
cd rpminspect-data-fedora || exit 1
CONF="${TMPDATADIR}/rpminspect-data-fedora/fedora.yaml"
PROFILES="${TMPDATADIR}/rpminspect-data-fedora/profiles/fedora"
TMPCONF="$(mktemp -p "${TMPDATADIR}" -t rpminspect.yaml.XXXXXX)"
sed -e "s|profiledir:.*$|profiledir: ${PROFILES}|g" < "${CONF}" > "${TMPCONF}"
sed -i -e "s|vendor_data_dir:.*$|vendor_data_dir: $(dirname "${CONF}")|g" "${TMPCONF}"

run_rpminspect() {
    ${RPMINSPECT} -c "${TMPCONF}" -a x86_64,noarch,src -v "$@";
}

# Run comparison against the latest packages, or fail
for package in ${packages}; do
    builds=$(koji list-builds --package="${package}" --state=COMPLETE | grep "${dist}" | cut -d ' ' -f 1 | sort -n | tail -n 2 | xargs)

    if [ -z "${builds}" ]; then
        echo "ERROR: Unable to find builds for ${package}" >&2
        exit 1
    fi

    echo "INFO: Comparing ${builds}"

    opts="-o ${package}.log ${builds}"
    run_rpminspect "${opts}"
    ret=$?
    [ ${ret} -lt 2 ] || exit 1

    echo "INFO: Comparison complete: ${package}.log"
done

exit 0
