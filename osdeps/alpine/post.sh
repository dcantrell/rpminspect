#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# The mandoc package in Alpine Linux lacks the library
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
  echo 'INSTALL_PROGRAM="install -D -m 0755"';
  echo 'INSTALL_LIB="install -D -m 0644"';
  echo 'INSTALL_HDR="install -D -m 0644"';
  echo 'INSTALL_MAN="install -D -m 0644"';
  echo 'INSTALL_DATA="install -D -m 0644"';
  echo 'INSTALL_LIBMANDOC=1';
  echo 'CFLAGS="-g -fPIC"';
} > "${SUBDIR}"/configure.local

# unusual workarounds for executable on Alpine Linux
sed -i -e 's|@echo|@/bin/echo|g' "${SUBDIR}"/configure

( cd "${SUBDIR}" && ./configure && make && make lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# The 'rc' shell is not available in Arch Linux, build manually
git clone -q https://github.com/rakitzis/rc.git
cd rc || exit 1
TAG="$(git tag -l | sort -V | tail -n 1)"
git checkout -b ${TAG} ${TAG}
autoreconf --force --install
CFLAGS=-std=gnu99 ./configure --prefix=/usr
make
make install
cd "${CWD}" || exit 1
rm -rf rc

# AT&T Korn Shell is not packaged for Alpine Linux, so cheat here and
# just use 'mksh' for the ksh syntax tests.
ln -s mksh /bin/ksh

# Install libabigail from git
cd "${CWD}" || exit 1
git clone -q git://sourceware.org/git/libabigail.git
cd libabigail || exit 1
TAG="$(git tag -l | grep ^libabigail- | grep -v '\.rc' | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
autoreconf -f -i -v
if [ ! -r /usr/lib/pkgconfig/fts-standalone.pc ]; then
    ln -sf musl-fts.pc /usr/lib/pkgconfig/fts-standalone.pc
fi
./configure --prefix=/usr/local
make V=1
make install

# Install debugedit (/usr/bin/find-debuginfo) from git
cd "${CWD}" || exit 1
git clone -q git://sourceware.org/git/debugedit.git
cd debugedit || exit 1
TAG="$(git tag -l | grep ^debugedit- | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
autoreconf -f -i -v
env CFLAGS=-D_LARGEFILE64_SOURCE ./configure --prefix=/usr
make -k V=1
make install

# cdson is not [yet] in Alpine
cd "${CWD}" || exit 1
git clone -q https://github.com/frozencemetery/cdson.git
cd cdson || exit 1
TAG="$(git tag -l | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
meson setup build
ninja -C build -v
ninja -C build test
ninja -C build install
cd "${CWD}" || exit 1
rm -rf cdson

# Avoid getting %{_arch} in filenames from rpmbuild
echo "%_arch %(/bin/arch)" > ~/.rpmmacros

# Alpine Linux does not define an RPM dist tag
echo '%dist .ri47' >> "${HOME}"/.rpmmacros

# Alpine Linux gets find-debuginfo via this script
echo '%__find_debuginfo /usr/bin/find-debuginfo' >> "${HOME}"/.rpmmacros

# This change to /usr/lib/rpm/macros was introduced on 24-Aug-2024 in
# commit 1a9803d0f8daf15bb706dc17783ab19589906487 to rpm, but it
# causes problems for the rpminspect test suite.  Undo the change.
sed -i -e '/^%%global\ __debug_package\ 1\\/d' /usr/lib/rpm/macros

# Update the clamav database
freshclam
