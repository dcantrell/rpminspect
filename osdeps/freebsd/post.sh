#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin
CWD="$(pwd)"

# Sync ports tree from git
rm -rf /usr/ports || :
git clone https://git.freebsd.org/ports.git /usr/ports

# https://github.com/rpm-software-management/rpm/pull/2459
RPMTAG_HDR="/usr/local/include/rpm/rpmtag.h"
if grep RPM_MASK_RETURN_TYPE ${RPMTAG_HDR} 2>/dev/null | grep -q 0xffff0000 >/dev/null 2>&1 ; then
    sed -I -E 's|^.*RPM_MASK_RETURN_TYPE.*=.*0xffff0000$|#define RPM_MASK_RETURN_TYPE 0xffff0000|g' ${RPMTAG_HDR}
    sed -I -E '/RPM_MAPPING_RETURN_TYPE/ s/\,$//' ${RPMTAG_HDR}
fi

# Hostname to make sure rpmbuild works (this is gross)
echo "$(ifconfig | grep "inet " | grep -v "inet 127" | awk '{ print $2; }') $(hostname)" >> /etc/hosts

# Install Python modules from ports, but we have to determine the
# package prefix based on the version of Python installed.
PKG_PREFIX="py$(python3 --version | cut -d ' ' -f 2 | cut -d '.' -f 1,2 | sed -e 's|\.||g')"
pkg install -y -q "${PKG_PREFIX}-pip" "${PKG_PREFIX}-pyaml" "${PKG_PREFIX}-timeout-decorator"

# Now install modules with pip
pip install -q cpp-coveralls gcovr rpmfluff

# libmandoc is missing on FreeBSD
cd "${CWD}" || exit 1
if curl -s http://mandoc.bsd.lv/ >/dev/null 2>&1 ; then
    curl -O http://mandoc.bsd.lv/snapshots/mandoc.tar.gz
else
    # failed to connect to upstream host; take Debian's source
    DEBIAN_URL=http://ftp.debian.org/debian/pool/main/m/mdocml/
    # figure out which one is the latest and get that
    SRCFILE="$(curl -s ${DEBIAN_URL} 2>/dev/null | sed -r 's/<[^>]*>//g' | sed -r 's/<[^>]*>$//g' | tr -s ' ' | grep -vE '^[ \t]*$' | grep ".orig.tar" | sed -r 's/[0-9]{4}-[0-9]{2}-[0-9]{2}.*$//g' | sort -n | tail -n 1)"
    curl -o mandoc.tar.gz ${DEBIAN_URL}/"${SRCFILE}"
fi
SUBDIR="$(tar -tvf mandoc.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xf mandoc.tar.gz
{ echo 'PREFIX=/usr/local';
  echo 'BINDIR=/usr/local/bin';
  echo 'SBINDIR=/usr/local/sbin';
  echo 'MANDIR=/usr/local/share/man';
  echo 'INCLUDEDIR=/usr/local/include';
  echo 'LIBDIR=/usr/local/lib';
  echo 'LN="ln -sf"';
  echo 'MANM_MANCONF=mandoc.conf';
  echo 'INSTALL_PROGRAM="install -m 0755"';
  echo 'INSTALL_LIB="install -m 0644"';
  echo 'INSTALL_HDR="install -m 0644"';
  echo 'INSTALL_MAN="install -m 0644"';
  echo 'INSTALL_DATA="install -m 0644"';
  echo 'INSTALL_LIBMANDOC=1';
  echo 'CFLAGS="-g -fPIC"';
} > "${SUBDIR}"/configure.local

( cd "${SUBDIR}" && ./configure && gmake && gmake lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# cdson is not [yet] in FreeBSD
cd "${CWD}" || exit 1
git clone https://github.com/frozencemetery/cdson.git
cd cdson || exit 1
TAG="$(git tag -l | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
meson setup build
ninja -C build
ninja -C build test
ninja -C build install
cd "${CWD}" || exit 1
rm -rf cdson

# The ksh93 package does not install a 'ksh' executable, so create a symlink
ln -sf ksh93 /usr/local/bin/ksh

# Update the clamav database
freshclam
