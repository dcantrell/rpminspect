#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install 32-bit development files on x86_64 systems
if [ "$(uname -m)" = "x86_64" ]; then
    dnf install -y glibc-devel.i686 glibc.i686 libgcc.i686
fi

# Update clamav database
freshclam
