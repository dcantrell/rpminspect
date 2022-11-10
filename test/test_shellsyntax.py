#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

from baseclass import TestRPMs, TestKoji
from baseclass import TestCompareRPMs, TestCompareKoji
import rpmfluff

# The 'shellsyntax' inspection looks at shell scripts in the
# packages and reports of syntax validation errors.  Use the rpmfluff
# Python module to add valid and invalid shell scripts to the test
# RPMs for each test.  rpmfluff isn't documented, but it's a single
# Python file installed by the 'python3-rpmfluff' package.  See other
# tests for examples on how to use it as well.

# NOTE: We are only going to test the list of default shells as
# specified in generic.yaml.  See data/rpminspect.yaml for more info.

###################################################################
# sh - Bourne/POSIX shell syntax only                             #
# Fedora and derivatives use 'dash', which is a Debian maintained #
# port of the Almquist shell:                                     #
#     https://en.wikipedia.org/wiki/Almquist_shell                #
# Scripts using /bin/sh should adhere to POSIX shell syntax.      #
###################################################################


valid_sh = """#!/bin/sh
valid=true
count=1
while [ $valid ]
do
echo $count
if [ $count -eq 5 ];
then
break
fi
((count++))
done
"""


invalid_sh = """#!/bin/sh
valid=true
count=1
while [ $valid ]
do
echo $count
if count -eq 5 ]
then
"""


class ShWellFormedRPM(TestRPMs):
    """
    Valid /bin/sh script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ShMalformedRPM(TestRPMs):
    """
    Invalid /bin/sh script is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ShWellFormedKoji(TestKoji):
    """
    Valid /bin/sh script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ShMalformedKoji(TestKoji):
    """
    Invalid /bin/sh script is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ShWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/sh script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ShMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/sh script is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ShWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/sh script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ShMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/sh script is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ShWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/sh script in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ShWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/sh script in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_sh.sh", rpmfluff.SourceFile("valid_sh.sh", valid_sh)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_sh.sh", invalid_sh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


##################################################################
# ksh - Korn shell                                               #
# There are two main varieties of ksh:  ksh88 and ksh93          #
# Both are from David Korn at AT&T and have been released in     #
# various forms for a long time.  The most widely used version   #
# in commercial Unix settings was ksh88.  ksh88 inspired shells  #
# like bash, zsh, and others with advanced editing capabilities. #
#     https://en.wikipedia.org/wiki/KornShell                    #
#                                                                #
# There is also the Public Domain Korn shell (pdksh) and mksh    #
# from MirOS BSD.  Neither of these are 100% compatible, but     #
# close.  Red Hat used to ship pdksh to provide something close  #
# to ksh for users.  Fedora and derivatives now ship ksh93 which #
# was open sourced in 2000 and is still maintained.  The tests   #
# here should assume /bin/ksh is AT&T ksh93.                     #
##################################################################


valid_ksh = """#!/bin/ksh
function date.get
{
        .sh.value=$(date)
}
print -r -- "$date"

for  i in 0 1
do   if   ((i==0))
     then alias print=date
     fi
     x='"'  y=""
     print "'hello'" ${x}there${x}
done
print +%H:%M:%S
"""


invalid_ksh = """#!/bin/ksh
function date.get
{
        .sh.value=$(date)
}
print -r -- "$date"

for  i in "0" "1"
do   if   ((i==0))
     then alias print = date
     fi
     x='"  y=""
     print "'hello'" ${x}there${x}
done
print +%H:%M:%S
"""


class KshWellFormedRPM(TestRPMs):
    """
    Valid /bin/ksh script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_ksh.sh", valid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class KshMalformedRPM(TestRPMs):
    """
    Invalid /bin/ksh script is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_ksh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class KshWellFormedKoji(TestKoji):
    """
    Valid /bin/ksh script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_ksh.sh", valid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class KshMalformedKoji(TestKoji):
    """
    Invalid /bin/ksh script is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_ksh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class KshWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/ksh script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_ksh.sh", valid_ksh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_sh.sh", valid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class KshMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/ksh script is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_ksh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_ksh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class KshWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/ksh script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_ksh.sh", valid_ksh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_ksh.sh", valid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class KshMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/ksh script is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_ksh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_ksh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class KshWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/ksh script in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_ksh.sh", valid_ksh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_ksh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class KshWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/ksh script in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_ksh.sh",
            rpmfluff.SourceFile("valid_ksh.sh", valid_ksh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_ksh.sh", invalid_ksh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


###################################################################
# zsh - Z shell                                                   #
# Z shell began life at Princeton University in 1990 and has lead #
# an exciting life as a powerful interactive shell and scripting  #
# language.  It shares some compatibility with sh, ksh, and bash  #
# but should be treated as distinctly separate.                   #
#     https://en.wikipedia.org/wiki/Z_shell                       #
###################################################################


valid_zsh = """#!/bin/zsh
ls > file1 > file2 > file3

citytext="New York
Rio
Tokyo"

cityarray=( ${(ps/\n/)citytext} )

a=5; b=32; c=24; (( a += (++a + b * c) - 2 ))

if [[ (($x -lt  8) && ($y -ge 32))
      || (($z -gt 32) && ($w -eq 16)) ]]
  then
     print "complex combinations"
     print "are not a problem."
fi
"""


invalid_zsh = """#!/bin/zsh
ls > file1 > file2 > file3

citytext="New York
Rio
Tokyo"

cityarray=( ${(ps/\n/)citytext}

a=5; b=32; c=24; (( a += (++a + b * c) - 2 )

if [[ (($x -lt  8) && ($y -ge 32))
      || (($z -gt 32) && ($w -eq 16)) ]]
  then
     print "complex combinations"
     print "are not a problem."
fi
"""


class ZshWellFormedRPM(TestRPMs):
    """
    Valid /bin/zsh script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ZshMalformedRPM(TestRPMs):
    """
    Invalid /bin/zsh script is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ZshWellFormedKoji(TestKoji):
    """
    Valid /bin/zsh script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ZshMalformedKoji(TestKoji):
    """
    Invalid /bin/zsh script is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ZshWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/zsh script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ZshMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/zsh script is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ZshWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/zsh script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class ZshMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/zsh script is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ZshWellMalformedCompareRPMs(TestCompareRPMs):
    """
    VValid /bin/zsh script in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ZshWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/zsh script in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_zsh.sh",
            rpmfluff.SourceFile("valid_zsh.sh", valid_zsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_zsh.sh",
            rpmfluff.SourceFile("invalid_zsh.sh", invalid_zsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


#################################################################
# csh - The C Shell                                             #
# Bill Joy gave us the C shell while at UCB.  The BSD operating #
# system included it as a result and it's why BSD-derivatives   #
# tend to favor csh varieties by default.  No one should write  #
# in csh, but that doesn't stop people from doing so.  Be sure  #
# to use csh syntax in these scripts.                           #
#     https://en.wikipedia.org/wiki/C_shell                     #
#################################################################


valid_csh = """#!/bin/csh
set ls1='some files: [a-z]*'
set ls2="some files: [a-z]*"

foreach i (*)
        if (-f $i) then
            echo "$i is a file."
            echo $ls1
        endif
        if (-d $i) then
             echo "$i is a directory."
             echo $ls2
        endif
end

set word = "anything"
     while ($word != "")
       echo -n "Enter a word to check (Return to exit): "
       set word = $<
       if ($word != "") grep $word /usr/share/dict/words
end

echo -n Input your value:
set input = `head -1`
echo You entered: $input
"""


invalid_csh = """#!/bin/csh
set ls1='some files: [a-z]*'
set ls2="some files: [a-z]*

foreach i (*)
        if (-f $i) then
            echo "$i is a file."
            echo $ls1
        endif
        if (-d $i) then
             echo "$i is a directory."
             echo $ls2
        end
end

set word = "anything"
     while ($word != "")
       echo -n "Enter a word to check (Return to exit): "
       set word = $<
       if ($word != "") grep $word /usr/share/dict/words
end

echo -n Input your value:
set input = `head -1
echo You entered: $
"""


class CshWellFormedRPM(TestRPMs):
    """
    Valid /bin/csh script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class CshMalformedRPM(TestRPMs):
    """
    Invalid /bin/csh script is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class CshWellFormedKoji(TestKoji):
    """
    Valid /bin/csh script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class CshMalformedKoji(TestKoji):
    """
    Invalid /bin/csh script is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class CshWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/csh script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class CshMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/csh script is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class CshWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/csh script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class CshMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/csh script is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class CshWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/csh script in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class CshWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/csh script in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_csh.sh",
            rpmfluff.SourceFile("valid_csh.sh", valid_csh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_csh.sh",
            rpmfluff.SourceFile("invalid_csh.sh", invalid_csh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


################################################################
# tcsh - TENEX C shell                                         #
# Inspired by TENEX, Ken Greer at Carnegie Mellon University   #
# wrote tcsh give TENEX-style completion to csh.               #
#     https://en.wikipedia.org/wiki/Tcsh                       #
################################################################

valid_tcsh = r"""#!/bin/tcsh
# test if we have two arguments, exit with warning if we don't
if ($# != 2) then
  echo "Usage $0 oldextension newextension"
  exit 1
endif

foreach file (*.$1)
   # this one echos filename, and replaces old extension
   # with the new one
   set newname=`echo $file | sed s/$1\$/$2/`
   echo "Renaming $file to $newname"
   mv "$file" "$newname"
end
"""


invalid_tcsh = r"""#!/bin/tcsh
# test if we have two arguments, exit with warning if we don't
if [[ $? != 0 ]] then
 echo "That didn't work!"
fi

foreach file (*.$1)
   # this one echos filename, and replaces old extension
   # with the new one
   set newname=`echo $file | sed s/$1\$/$2/
   echo "Renaming $file to $newname"
   mv "$file" "$newname
"""


class TcshWellFormedRPM(TestRPMs):
    """
    Valid /bin/tcsh script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class TcshMalformedRPM(TestRPMs):
    """
    Invalid /bin/tcsh script is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TcshWellFormedKoji(TestKoji):
    """
    Valid /bin/tcsh script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class TcshMalformedKoji(TestKoji):
    """
    Invalid /bin/tcsh script is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TcshWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/tcsh script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class TcshMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/tcsh script is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TcshWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/tcsh script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class TcshMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/tcsh script is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TcshWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/tcsh script in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TcshWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/tcsh script in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_tcsh.sh",
            rpmfluff.SourceFile("valid_tcsh.sh", valid_tcsh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_tcsh.sh",
            rpmfluff.SourceFile("invalid_tcsh.sh", invalid_tcsh),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


#################################################################
# rc - Plan 9 shell                                             #
# From the Plan 9 operating system at Bell Labs comes 'rc', the #
# shell inspired by Bourne shell syntax but a little different. #
# It is unlikely a Linux user would encounter an rc script in   #
# the wild, but the shell is provided so we should check for    #
# those scripts.                                                #
#     https://en.wikipedia.org/wiki/Rc                          #
#################################################################


valid_rc = """#!/bin/rc
who >user.names
who >>user.names
wc <file

echo [a-f]*.c
who | wc
who; date
vc *.c &

mk && v.out /*/bin/fb/*
echo hully^gully

{sleep 30;echo 'Time''s up!'}&

for(i in *.c) if(cpp $i >/tmp/$i) vc /tmp/$i

fn g {
    grep $1 *.[hcyl]
}

vc junk.c >[2=1] >junk.out
"""


invalid_rc = """#!/bin/rc
who >user.names
who >>user.names
wc <file

echo [a-f]*.c
who | wc
who; date
vc *.c &)

mk && v.out /*/bin/fb/*
echo hully^gully

{sleep 3600;echo 'Time''s up!'}&

for(i in *.c) if(cpp $i >/tmp/$i) vc /tmp/$i

fn g {
    grep $1 *.[hcyl]
}}

vc junk.c >2=1] >junk.out
"""


class RcWellFormedRPM(TestRPMs):
    """
    Valid /bin/rc script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class RcMalformedRPM(TestRPMs):
    """
    Invalid /bin/rc script is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class RcWellFormedKoji(TestKoji):
    """
    Valid /bin/rc script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class RcMalformedKoji(TestKoji):
    """
    Invalid /bin/rc script is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class RcWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/rc script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class RcMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/rc script is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class RcWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/rc script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class RcMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/rc script is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class RcWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/rc script in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class RcWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/rc script in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_rc.sh", rpmfluff.SourceFile("valid_rc.sh", valid_rc)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_rc.sh",
            rpmfluff.SourceFile("invalid_rc.sh", invalid_rc),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


#################################################################
# bash - Bourne again shell from GNU                            #
# Probably the most used shell these days, but also one of the  #
# most irritable with regards to version compatibility.  It is  #
# well known there exist different "bash-isms" that even vary   #
# by version.  When writing shell scripts, one should favor sh  #
# unless it is entirely unusable.  In those cases, maybe going  #
# with a different language is a better choice.                 #
#     https://en.wikipedia.org/wiki/Bash_(Unix_shell)
# NOTE: rpminspect checks syntax on bash scripts, but if it     #
# fails, it will try again with 'extglob' enabled.  If that     #
# passes it reports that the script needs 'extglob' enabled.    #
#################################################################


valid_bash = r"""#!/bin/bash
echo "A-$(echo B-$(echo C-$(echo D)))"
some_var=${DEFAULT_VAL}

log() {  # classic logger
   local prefix="[$(date +%Y/%m/%d\ %H:%M:%S)]: "
   echo "${prefix} $@" >&2
}


log "INFO" "a message"

for ((i=0; i<3; i++)); do
    echo "\t$i\n"
done
"""


invalid_bash = r"""#!/bin/bash
echo "A-$(echo B-$(echo C-$(echo D)))"
some_var=${DEFAULT_VAL}

log() {  # classic logger
   local prefix="[$(date +%Y/%m/%d\ %H:%M:%S)]: "
   echo "${prefix} $@" >&2


log "INFO" "a message"

for ((i=0; i<3; i++)); do
    echo !(*/)
"""


valid_bash_extglob = r"""#!/bin/bash
echo !(*/)
readonly DEFAULT_VAL=${DEFAULT_VAL:-7}

t="abc123"
[[ "$t" == abc* ]]    # true (globbing)

case "$input" in
    [Yy]|'') confirm=1;;
    [Nn]*) confirm=0;;
    *) echo "I don't understand.  Please try again.";;
esac
"""


class BashWellFormedRPM(TestRPMs):
    """
    Valid /bin/bash script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_bash.sh", valid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class BashMalformedRPM(TestRPMs):
    """
    Invalid /bin/bash script is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_bash.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class BashExtglobWellFormedRPM(TestRPMs):
    """
    Valid but requires 'extglob' script is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_bash_extglob.sh",
            rpmfluff.SourceFile("valid_bash_extglob.sh", valid_bash_extglob),
        )
        self.inspection = "shellsyntax"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class BashWellFormedKoji(TestKoji):
    """
    Valid /bin/bash script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_bash.sh", valid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class BashMalformedKoji(TestKoji):
    """
    Invalid /bin/bash script is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/invalid_bash.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class BashExtglobWellFormedKoji(TestKoji):
    """
    Valid but requires 'extglob' script is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/data/valid_bash_extglob.sh",
            rpmfluff.SourceFile("valid_bash_extglob.sh", valid_bash_extglob),
        )
        self.inspection = "shellsyntax"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class BashWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/bash script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_bash.sh", valid_bash),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_sh.sh", valid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class BashMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/bash script is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_bash.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_bash.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class BashExtglobWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid but requires 'extglob' script is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_bash_extglob.sh",
            rpmfluff.SourceFile("valid_bash_extglob.sh", valid_bash_extglob),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_bash_extglob.sh",
            rpmfluff.SourceFile("valid_bash_extglob.sh", valid_bash_extglob),
        )
        self.inspection = "shellsyntax"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class BashWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/bash script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_bash.sh", valid_bash),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_bash.sh", valid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "OK"


class BashMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/bash script is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid_bash.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_bash.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class BashExtglobWellFormedCompareKoji(TestCompareKoji):
    """
    Valid but requires 'extglob' script is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_bash_extglob.sh",
            rpmfluff.SourceFile("valid_bash_extglob.sh", valid_bash_extglob),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid_bash_extglob.sh",
            rpmfluff.SourceFile("valid_bash_extglob.sh", valid_bash_extglob),
        )
        self.inspection = "shellsyntax"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class BashWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/bash script in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_bash.sh", valid_bash),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_bash.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class BashWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/bash script in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid_bash.sh",
            rpmfluff.SourceFile("valid_bash.sh", valid_bash),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid_sh.sh",
            rpmfluff.SourceFile("invalid_bash.sh", invalid_bash),
        )
        self.inspection = "shellsyntax"
        self.result = "BAD"
        self.waiver_auth = "Anyone"
