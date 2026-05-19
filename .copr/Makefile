# Copyright (C) 2019 David Cantrell <david.l.cantrell@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Set the top level source directory
topdir := $(shell realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Figure out if we need to run dnf or yum
_HAVE_DNF := $(shell ls -1 /usr/bin/dnf 2>/dev/null)
ifeq ($(_HAVE_DNF),)
PKG_CMD = yum
else
PKG_CMD = dnf
endif

# Install packages before anything else
_install := $(shell $(PKG_CMD) install -y git)
_safedir := $(shell git config --global --add safe.directory $(topdir))

# Spec file and template
SPEC_TEMPLATE = $(shell ls -1 $(topdir)/*.spec.in)
SPEC = $(topdir)/$(shell basename $(SPEC_TEMPLATE) .in)

# Replace placeholders in the spec file template
RPMDATE = $(shell date +'%a %b %d %Y')
RPMAUTHOR = $(shell git log | tac | grep ^Author: | head -n 1 | cut -d ' ' -f 2,3,4)

# Various things we need to generate a tarball
PKG = $(shell grep ^Name: "$(SPEC_TEMPLATE)" | awk '{ print $$2; }')
VER = $(shell grep 'version :' $(topdir)/meson.build | grep -E "'[0-9]+\.[0-9]+'" | cut -d "'" -f 2)

TAG              = $(shell git tag -l | sort -V | tail -n 1)

TARBALL_BASENAME = $(PKG)-$(VER)

srpm:
	sed -e 's|%%VERSION%%|$(VER)|g' < "$(SPEC_TEMPLATE)" > "$(SPEC)"
	sed -i -e 's|%%GPGKEYRING%%|gpgkey-$(GPGKEY).gpg|g' "$(SPEC)"
	rm -f "$(TARBALL_BASENAME)".tar.gz.asc
	git archive \
		--format=tar \
		--output='$(topdir)/$(TARBALL_BASENAME).tar' \
		--prefix='$(TARBALL_BASENAME)/' $(TAG) $(topdir)
	gzip -9f $(topdir)/$(TARBALL_BASENAME).tar
	gpg --detach-sign --armor "$(TARBALL_BASENAME)".tar.gz
	gpg --export -a $(GPGKEY) > gpgkey-$(GPGKEY).gpg
	rpmbuild \
		-bs --nodeps \
		--define "_sourcedir $(topdir)" \
		--define "_srcrpmdir $(outdir)" \
		--define "_rpmdir $(outdir)" "$(SPEC)"
