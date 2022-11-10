#!/bin/sh
#
# Build new releases in Koji
#
# Copyright David Cantrell
# SPDX-License-Identifier: GPL-3.0-or-later
#

# Arguments:
#     $1    Path to the release tarball to build
#     $2    Path to the detached signature for the release tarball
#     $3    The name of the project in dist-git

PATH=/usr/bin
CWD="$(pwd)"
WRKDIR="$(mktemp -d)"

# The list of tools that may or may not be installed locally but
# that the script needs.  Extend this list if needed.
TOOLS="klist"

# What dist-git interaction tool we are using, e.g. Fedora is 'fedpkg'
VENDORPKG="fedpkg"
VENDORKOJI="koji"

cleanup() {
    rm -rf "${WRKDIR}"
}

trap cleanup EXIT

# Verify specific tools are available
for tool in ${TOOLS} ${VENDORPKG} ${VENDORKOJI} ; do
    ${tool} >/dev/null 2>&1
    if [ $? -eq 127 ]; then
        echo "*** Missing '${tool}', perhaps 'yum install -y /usr/bin/${tool}'" >&2
        exit 1
    fi
done

# Need a tarball
if [ $# -eq 0 ]; then
    echo "*** Missing tarball of new release" >&2
    exit 1
fi

TARBALL="$(realpath "$1")"

if [ ! -f "${TARBALL}" ]; then
    echo "*** $(basename "${TARBALL}") does not exist" >&2
    exit 1
fi

if ! tar tf "${TARBALL}" >/dev/null 2>&1 ; then
    echo "*** $(basename "${TARBALL}") is not a tar archive" >&2
    exit 1
fi

shift

# Need tarball signature
if [ $# -eq 0 ]; then
    echo "*** Missing detached signature of release tarball" >&2
    exit 1
fi

TARBALL_ASC="$(realpath "$1")"

if [ ! -f "${TARBALL_ASC}" ]; then
    echo "*** $(basename "${TARBALL_ASC}") does not exist" >&2
    exit 1
fi

if [ ! "$(file -b --mime-type "${TARBALL_ASC}")" = "application/pgp-signature" ]; then
    echo "*** $(basename "${TARBALL_ASC}") is not a gpg signature" >&2
    exit 1
fi

shift

# Need project name
if [ $# -eq 0 ]; then
    echo "*** Missing project name" >&2
    exit 1
fi

PROJECT="$1"
shift

# Need a krb5 ticket
klist >/dev/null 2>&1
if [ $? -eq 1 ]; then
    echo "*** You lack an active Kerberos ticket" >&2
    exit 1
fi

klist | grep -q "krbtgt/FEDORAPROJECT.ORG@FEDORAPROJECT.ORG" >/dev/null 2>&1
if [ $? -eq 1 ]; then
    echo "*** You need a FEDORAPROJECT.ORG Kerberos ticket" >&2
    exit 1
fi

GIT_USERNAME="$(git config user.name)"
GIT_USEREMAIL="$(git config user.email)"

cd "${CWD}" || exit

cd "${WRKDIR}" || exit
${VENDORPKG} co "${PROJECT}"
cd "${PROJECT}" || exit

# Allow the calling environment to override the list of dist-git branches
if [ -z "${BRANCHES}" ]; then
    BRANCHES="$(git branch -r | grep -vE "(HEAD|playground|main)" | cut -d '/' -f 2 | sort | xargs)"
fi

for branch in ${BRANCHES} ; do
    git clean -dxf
    git config user.name "${GIT_USERNAME}"
    git config user.email "${GIT_USEREMAIL}"

    # skip this branch if we lack build targets
    if [ ! "${branch}" = "rawhide" ]; then
        if ! ${VENDORKOJI} list-targets --name="${branch}-candidate" >/dev/null 2>&1 ; then
            echo "*** Skipping ${branch} because there is no longer a ${VENDORKOJI} target"
            continue
        fi
    fi

    # make sure we are on the right branch
    ${VENDORPKG} switch-branch "${branch}"
    git pull

    # add the new source archive
    ${VENDORPKG} new-sources "${TARBALL}" "${TARBALL_ASC}"

    # save current changelog
    pos=$(grep -n '^%changelog' "${PROJECT}".spec | cut -d ':' -f 1)
    len=$(wc -l "${PROJECT}".spec | cut -d ' ' -f 1)
    tail -n $((len - pos)) "${PROJECT}".spec > "${CWD}"/cl

    # new changelog entry
    VER="$(grep ^Version "${CWD}"/"${PROJECT}".spec | awk '{ print $2; }')"
    REL="$(grep ^Release: "${CWD}"/"${PROJECT}".spec | awk '{ print $2; }' | cut -d '%' -f 1)"
    echo "* $(date +"%a %b %d %Y") ${GIT_USERNAME} <${GIT_USEREMAIL}> - ${VER}-${REL}" > "${CWD}"/newcl
    echo "- Upgrade to ${PROJECT}-${VER}" >> "${CWD}"/newcl

    # new spec file
    cat "${CWD}"/"${PROJECT}".spec "${CWD}"/newcl > "${PROJECT}".spec

    if [ ! "$(stat -c %s "${CWD}"/cl)" = "0" ]; then
        echo >> "${PROJECT}".spec
        cat "${CWD}"/cl >> "${PROJECT}".spec
    fi

    rm -f "${CWD}"/newcl
    rm -f "${CWD}"/cl

    # copy in gpgkey
    cp "${CWD}"/*.gpg .

    # commit changes
    git add sources ./*.gpg "${PROJECT}".spec
    ${VENDORPKG} ci -cps
    git clean -dxf

    # build
    ${VENDORPKG} build --nowait
done
