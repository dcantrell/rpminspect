#!/bin/sh
#
# Automate making a new release of meson-built projects that also
# use Copr for automated builds and that carry template RPM spec
# files.  See README for more information.
#
# Copyright David Cantrell
# SPDX-License-Identifier: GPL-3.0-or-later
#

PATH=/usr/bin
CWD="$(pwd)"
PROG="$(basename "$0")"
PROJECT="$(basename "$(realpath "${CWD}")")"

# Command line options
OPT_BUMPVER=
OPT_TAG=
OPT_PUSH=

usage() {
    echo "Create a release for ${PROJECT}"
    echo "${PROG} [options]"
    echo "Options:"
    echo "    -b, --bumpver      Increment the minor version number"
    echo "    -t, --tag          Tag the release in git"
    echo "    -p, --push         Push the release and release tag"
    echo "    -A, --all          Same as '-b -t -p'"
    echo "    -h, --help         Display usage"
    echo "The options control behavior on the repo and upstream."
    echo
    echo "The bumpver option increments the minor version number and commits"
    echo "that to the repo (e.g., version 0.5 becomes 0.6). Only the"
    echo "development team should explicitly increment the major version"
    echo "number."
    echo
    echo "The tag option will tag the released version in git locally. So,"
    echo "the 0.6 release becomes tag v0.6 in the local git repo."
    echo
    echo "The push option pushes the version increment commit and release"
    echo "tag to the upstream git repo."
}

# Handle command line options
OPTS=$(getopt -o 'btpAh' --long 'bumpver,tag,push,all,help' -n "${PROG}" -- "$@")

# shellcheck disable=SC2181
if [ $? -ne 0 ]; then
    echo "Terminating..." >&2
    exit 1
fi

eval set -- "${OPTS}"
unset OPTS

while true ; do
    case "$1" in
        '-b'|'--bumpver')
            OPT_BUMPVER=y
            shift
            ;;
        '-t'|'--tag')
            OPT_TAG=y
            shift
            ;;
        '-p'|'--push')
            OPT_PUSH=y
            shift
            ;;
        '-A'|'--all')
            OPT_BUMPVER=y
            OPT_TAG=y
            OPT_PUSH=y
            shift
            ;;
        '-h'|'--help')
            usage
            exit 0
            ;;
        '--')
            shift
            break
            ;;
        *)
            echo "Internal error" >&2
            exit 1
            ;;
    esac
done

# This must be a git project
if [ ! -d "${CWD}"/.git ] && [ ! -f "${CWD}"/.git/config ]; then
    echo "*** Missing .git subdirectory." >&2
    exit 1
fi

# Make sure we are in the right place
if [ ! -f "${CWD}/meson.build" ] && [ ! -f "${CWD}/${PROJECT}.spec.in" ]; then
    echo "*** You must run this script from the top level ${PROJECT} directory" >&2
    exit 1
fi

# Fail if the git index is unclean
if [ -n "$(git status --porcelain)" ]; then
    echo "*** git index is not clean:" >&2
    git status --porcelain | sed -e 's|^|*** |g' >&2
    exit 1
fi

# Increment the version number and commit that change
VERSION="$(grep 'version :' meson.build | grep -E "'[0-9]+\.[0-9]+'" | cut -d "'" -f 2)"
CURMAJ="$(echo "${VERSION}" | cut -d '.' -f 1)"
CURMIN="$(echo "${VERSION}" | cut -d '.' -f 2)"

if [ "${OPT_BUMPVER}" = "y" ]; then
    OLDVERSION="${VERSION}"
    NEWMIN="$(("${CURMIN}" + 1))"
    VERSION="${CURMAJ}.${NEWMIN}"
    sed -i -e "s|'${OLDVERSION}'|'${VERSION}'|g" meson.build
    git add meson.build
    git commit -m "New release (${VERSION})"
else
    OLDVERSION="${VERSION}"
fi

# Now tag the release
if [ "${OPT_TAG}" = "y" ]; then
    git tag -s -a -m "Tag release v${VERSION}" v"${VERSION}"
fi

# Generate the dist artifact and sign it
meson setup build
ninja -v -C build dist
cd build/meson-dist || exit
gpg --detach-sign --armor "${PROJECT}"-"${VERSION}".tar.xz
cd "${CWD}" || exit

# Push the changes
if [ "${OPT_PUSH}" = "y" ]; then
    git push
    git push --tags
fi

echo
echo "Log entries since the last release can be obtained with:"
echo "    git log v${OLDVERSION}.."
echo
