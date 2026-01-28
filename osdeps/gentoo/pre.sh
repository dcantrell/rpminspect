#!/bin/sh

PATH=/bin:/sbin:/usr/bin:/usr/sbin

# Enable Python support in things like rpm
echo 'USE="${USE} caps python lzma"' >> /etc/portage/make.conf
[ -d /etc/portage/package.use ] || mkdir -p /etc/portage/package.use
echo 'app-antivirus/clamav clamapp' > /etc/portage/package.use/clamav

# Disable things
echo 'USE="${USE} -milter -experimental -clamapp -bashlogger -mem-scramble -pgo -plugins -abyss -debug -gnome -qt5 -qt6"' >> /etc/portage/make.conf

# Allow installation of "masked" packages
[ -d /etc/portage ] || mkdir -p /etc/portage
rm -rf /etc/portage/package.accept_keywords
{ echo "app-shells/ksh ~amd64";
  echo "dev-util/gcovr ~amd64";
  echo "dev-util/libabigail ~amd64";
} >> /etc/portage/package.accept_keywords
