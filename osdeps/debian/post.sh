#!/bin/sh
PATH=/usr/bin:/usr/sbin
CWD="$(pwd)"

# Debian Linux testing does not define an RPM dist tag
echo '%dist .ri47' > "${HOME}"/.rpmmacros

# Install 32-bit development files on 64-bit systems when available
case "$(uname -m)" in
    x86_64)
        apt install -y gcc-multilib libc6-i386
        ;;
    s390x)
        apt install -y gcc-multilib
        ;;
esac

# The mandoc package on Debian lacks libmandoc.a and
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

# unusual workarounds for executable on Debian
sed -i -e 's|@echo|@/bin/echo|g' "${SUBDIR}"/configure

( cd "${SUBDIR}" && ./configure && make && make lib-install )
rm -rf mandoc.tar.gz "${SUBDIR}"

# cdson is not [yet] in Debian
git clone https://github.com/frozencemetery/cdson.git
cd cdson || exit 1
TAG="$(git tag -l | sort -n | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
meson setup build
ninja -C build -v
ninja -C build test
ninja -C build install
cd "${CWD}" || exit 1
rm -rf cdson

# Install gcc plugin dev files needed for annobin
GCC_VER="$(gcc --version | head -n 1 | rev | cut -d ' ' -f 1 | rev | cut -d '.' -f 1)"
apt install -y gcc-${GCC_VER}-plugin-dev

# Build and install annocheck
git clone git://sourceware.org/git/annobin.git
cd annobin || exit 1
meson setup -D clang-plugin=false -D llvm-plugin=false -D docs=false -D debuginfod=disabled build
ninja -C build -v
ninja -C build install
install -D -m 0755 "${CWD}"/build/annocheck/annocheck /usr/local/bin/annocheck
cd "${CWD}" || exit 1
rm -rf annobin

# This change to /usr/lib/rpm/macros was introduced on 24-Aug-2024 in
# commit 1a9803d0f8daf15bb706dc17783ab19589906487 to rpm, but it
# causes problems for the rpminspect test suite.  Undo the change.
sed -i -e '/^%%global\ __debug_package\ 1\\/d' /usr/lib/rpm/macros

# Update clamav database
service clamav-freshclam stop
freshclam
