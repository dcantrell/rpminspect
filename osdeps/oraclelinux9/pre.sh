#!/bin/sh
PATH=/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/sbin:/usr/sbin

# Update pip and setuptools
dnf install -y python3-pip
pip3 install --upgrade pip setuptools
