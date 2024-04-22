#!/bin/sh -x
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# Some RPM macros on suse try to override RPMTAG_VENDOR
sed -i -e '/^%vendor/d' /usr/lib/rpm/macros.d/*

# The mandoc package on OpenSUSE lacks libmandoc.a and
# header files, which we need to build rpminspect
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
( cd "${SUBDIR}" && ./configure && make && make lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# The 'rc' shell is not available in OpenSUSE Leap, build manually
git clone -q https://github.com/rakitzis/rc.git
cd rc || exit 1
make
make install
cd "${CWD}" || exit 1
rm -rf rc

# cdson is not [yet] in OpenSUSE Leap
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

# Update the clamav database
freshclam
