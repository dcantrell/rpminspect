#!/bin/sh
PATH=/usr/bin

if [ "$(uname -m)" = "x86_64" ]; then
    dpkg --add-architecture i386
    apt-get -y install lib32gcc-s1
fi
