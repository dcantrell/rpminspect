#!/bin/sh
#
# Create a text file suitable for a GitHub release announcement or
# blog post.  Groups git log summaries by category and only includes
# those log entries that have a category marking.
#
# Copyright David Cantrell
# SPDX-License-Identifier: GPL-3.0-or-later
#

PATH=/bin:/usr/bin
TMPDIR="$(mktemp -d)"
CWD="$(pwd)"

trap 'rm -rf "${TMPDIR}"' EXIT

# need the correct tags
if [ "$1" = "--stable" ]; then
    PREV_TAG="$(git tag -l | sort -V | tail -n 2 | head -n 1)"
    LATEST_TAG="$(git tag -l | sort -V | tail -n 1)"
else
    PREV_TAG="$(git tag -l | sort -V | tail -n 1)"
    LATEST_TAG="HEAD"
fi

if [ -z "${PREV_TAG}" ]; then
    echo "*** unable to find the previous git tag" >&2
    exit 1
fi

if [ -z "${LATEST_TAG}" ]; then
    echo "*** unable to find the latest git tag" >&2
    exit 1
fi

# gather log entries since the latest tag in reverse order and only
# those with category markers
git log --reverse --pretty=format:%s "${PREV_TAG}".."${LATEST_TAG}" 2>/dev/null | grep -E "^\[" | while read -r logline ; do
    category="$(echo "${logline}" | cut -d ']' -f 1 | cut -d '[' -f 2)"
    [ -z "${category}" ] && continue
    echo "*$(echo "${logline}" | cut -d ']' -f 2 | xargs -0)" >> "${TMPDIR}"/"${category}"
done

# now create a draft announcement grouped by category
for category in "${TMPDIR}"/* ; do
    if [ -f "${CWD}"/CONTRIBUTING.md ]; then
        desc="$(grep "\[$(basename "${category}")\]" "${CWD}"/CONTRIBUTING.md | cut -d '|' -f 3 | awk '{$1=$1};1')"
    fi

    if [ -z "${desc}" ]; then
        echo "$(basename "${category}"):"
    else
        echo "${desc}:"
    fi

    cat "${category}"
    echo
done
