#!/bin/sh
PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/pkg/sbin:/usr/pkg/bin:/usr/X11R7/bin:/usr/local/sbin:/usr/local/bin
CWD="$(pwd)"

# for pkgin use below
PKG_PATH="ftp://ftp.netbsd.org/pub/pkgsrc/packages/$(uname)/$(uname -m)/$(uname -r)/All/"
export PKG_PATH

# clamav is not available as a precompiled package
# Do not build from pkgsrc because it will try to update all of the
# packages we have installed already.
git clone -q https://github.com/Cisco-Talos/clamav.git
cd clamav || exit 1
TAG="$(git tag -l | grep -E "^clamav-[0-9\.]+$" | grep "\." | grep -v "\-rc" | gsort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
mkdir build
cd build || exit 1
cmake -D ENABLE_MILTER=OFF \
      -D ENABLE_JSON_SHARED=ON \
      -D ENABLE_TESTS=OFF \
      -D ENABLE_TESTS_DEFAULT=OFF \
      -D CMAKE_INSTALL_PREFIX=/usr/pkg \
      -D CMAKE_INSTALL_LIBDIR=lib \
      -D APP_CONFIG_DIRECTORY=/etc/clamav \
      -D DATABASE_DIRECTORY=/var/db/clamav \
      -G Ninja ..
ninja
ninja install
mv /etc/clamav/freshclam.conf.sample /etc/clamav/freshclam.conf
sed -i -e '/^Example/d' /etc/clamav/freshclam.conf
useradd clamav
mkdir -p /var/db/clamav
chown clamav /var/db/clamav
cd "${CWD}" || exit 1

# Update the clamav database
freshclam

# mandoc is available on NetBSD, but not the library which is what we need
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
{ echo 'PREFIX=/usr/pkg';
  echo 'BINDIR=/usr/pkg/bin';
  echo 'SBINDIR=/usr/pkg/sbin';
  echo 'MANDIR=/usr/pkg/man';
  echo 'INCLUDEDIR=/usr/pkg/include';
  echo 'LIBDIR=/usr/pkg/lib';
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

# cdson is not [yet] in NetBSD
# NOTE: install to /usr/pkg so pkgconf works with dependency() in
# meson.build and we don't have to do environment variable overrides
cd "${CWD}" || exit 1
git clone -q https://github.com/frozencemetery/cdson.git
cd cdson || exit 1
TAG="$(git tag -l | gsort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
meson setup build -Dprefix=/usr/pkg
ninja -C build
ninja -C build test
ninja -C build install
cd "${CWD}" || exit 1
rm -rf cdson

# Install elfutils to get usable libelf
cd "${CWD}" || exit 1
git clone -q https://sourceware.org/git/elfutils.git
cd elfutils || exit 1
TAG="$(git tag -l | grep ^elfutils- | cut -c10- | gsort -V | tail -n 1)"
git checkout -n "${TAG}" "${TAG}"
echo "#include <sys/endian.h>" > /usr/pkg/include/byteswap.h
autoreconf -fiv
env CPPFLAGS=-I/usr/pkg/include \
    LDFLAGS=-L/usr/pkg/lib \
    LIBS=-liberty \
    CFLAGS="-Dbswap_16=bswap16 -Dbswap_32=bswap32 -Dbswap_64=bswap64" \
./configure --prefix=/usr/pkg \
            --enable-maintainer-mode \
            --disable-debuginfo
find . -type f -name "*.c" | xargs sed -I -E 's|program_invocation_short_name|getprogname()|g'
gmake


# XXX: need to install libelf in a way that doesn't conflict with the system one

# Install latest Python and some modules
VERPART="$(pkgin search python | grep -E "^python[0-9]+\-" | awk '{ print $1; }' | sed -e 's/python//g' | cut -d '-' -f 1 | gsort -V | tail -n 1)"
pkgin -y install python"${VERPART}" py"${VERPART}"-gcovr py"${VERPART}"-coveralls py"${VERPART}"-pip py"${VERPART}"-timeout-decorator

# Figure out what the latest Python installed is
PYTHON="$(basename "$(find /usr/pkg/bin -type f -name "python*.*[0-9]" | gsort -V | tail -n 1)")"
PYTHON_VER="$(${PYTHON} -c 'import sys ; print("%d.%d" % (sys.version_info[0], sys.version_info[1]))')"

# XXX: install python rpm bindings

# Install modules with pip
pip"${PYTHON_VER}" install -q rpmfluff

# The ksh93 port does not install a 'ksh' executable, so create a symlink
ln -sf ksh93 /usr/pkg/bin/ksh

# https://github.com/rpm-software-management/rpm/pull/2459
RPMTAG_HDR="/usr/pkg/include/rpm/rpmtag.h"
if grep RPM_MASK_RETURN_TYPE ${RPMTAG_HDR} 2>/dev/null | grep -q 0xffff0000 >/dev/null 2>&1; then
    sed -I -E 's|^.*RPM_MASK_RETURN_TYPE.*=.*0xffff0000$|#define RPM_MASK_RETURN_TYPE 0xffff0000|g' ${RPMTAG_HDR}
    sed -I -E '/RPM_MAPPING_RETURN_TYPE/ s/\,$//' ${RPMTAG_HDR}
fi
