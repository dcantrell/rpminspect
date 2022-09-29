#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin

# libmandoc is missing on FreeBSD
curl -O https://mandoc.bsd.lv/snapshots/mandoc.tar.gz
SUBDIR="$(tar -tvf mandoc.tar.gz | head -n 1 | rev | cut -d ' ' -f 1 | rev)"
tar -xvf mandoc.tar.gz
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

# The 'rc' shell in FreeBSD is not the rc shell rpminspect expects, so
# just build it manually.
git clone https://github.com/rakitzis/rc.git
cd rc || exit 1
autoreconf -f -i -v
./configure --prefix=/usr/local
gmake
gmake install
cd "${CWD}" || exit 1
rm -rf rc

# Update the clamav database
freshclam
