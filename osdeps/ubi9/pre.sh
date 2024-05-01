#!/bin/sh
PATH=/usr/bin

# Update pip and setuptools
dnf install -y python3-pip
pip3 install --upgrade pip setuptools
