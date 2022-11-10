#!/bin/sh
#
# Copyright David Cantrell
# SPDX-License-Identifier: GPL-3.0-or-later
#

PATH=/usr/bin
CWD="$(pwd)"

if [ ! -f "${CWD}"/.copr/Makefile ]; then
    echo "*** Missing .copr/Makefile, exiting." >&2
    exit 1
fi

if [ "$1" = "-c" ]; then
    make -f "${CWD}"/.copr/Makefile srpm outdir="${CWD}" BUILDTYPE=copr
else
    make -f "${CWD}"/.copr/Makefile srpm outdir="${CWD}" BUILDTYPE=release
fi
