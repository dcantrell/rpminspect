#
# Copyright 2021 Red Hat, Inc.
# Author(s): David Cantrell <dcantrell@redhat.com>
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
#

import os

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]


#####################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "$ORIGIN" --> pass #
#####################################################################
class ValidRPATH1RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '$ORIGIN' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH1Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '$ORIGIN' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH1CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '$ORIGIN' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH1CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '$ORIGIN' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


######################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "$ORIGIN/" --> pass #
######################################################################
class ValidRPATH2RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '$ORIGIN/' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH2Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '$ORIGIN/' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH2CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '$ORIGIN/' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH2CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '$ORIGIN/' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


###############################################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "$ORIGIN/../lib/jli:$ORIGIN/../lib" --> FAIL #
###############################################################################################
class ValidRPATH3RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN/../lib/jli:$ORIGIN/../lib' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH3Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN/../lib/jli:$ORIGIN/../lib' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH3CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '$ORIGIN/../lib/jli:$ORIGIN/../lib' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH3CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '$ORIGIN/../lib/jli:$ORIGIN/../lib' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


##############################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "$ORIGIN/../lib64" --> pass #
##############################################################################
class ValidRPATH4RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '$ORIGIN/../lib64' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH4Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '$ORIGIN/../lib64' a.out\n"

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH4CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN/../lib64' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH4CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN/../lib64' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "OK"


####################################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "$ORIGIN:$ORIGIN/../lib" --> pass #
####################################################################################
class ValidRPATH5RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN:$ORIGIN/../lib' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH5Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN:$ORIGIN/../lib' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH5CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN:$ORIGIN/../lib' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "OK"


class ValidRPATH5CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '$ORIGIN:$ORIGIN/../lib' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "OK"


################################################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/builddir/build/BUILD/jq-1.6/.libs" --> FAIL #
################################################################################################
class InvalidRPATH1RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/jq-1.6/.libs' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH1Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/jq-1.6/.libs' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH1CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/jq-1.6/.libs' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH1CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/jq-1.6/.libs' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


#########################################################################################################################  # noqa: E266,E501
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/builddir/build/BUILD/texlive-base-20200327/source/inst/lib" --> FAIL #  # noqa: E501
#########################################################################################################################  # noqa: E266,E501
class InvalidRPATH2RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/texlive-base-20200327/source/inst/lib' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH2Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/texlive-base-20200327/source/inst/lib' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH2CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/texlive-base-20200327/source/inst/lib' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH2CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/builddir/build/BUILD/texlive-base-20200327/source/inst/lib' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


##############################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/usr/lib/systemd" --> FAIL #
##############################################################################
class ValidRPATH6RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib/systemd' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH6Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib/systemd' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH6CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '/usr/lib/systemd' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH6CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '/usr/lib/systemd' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


########################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/usr/lib64" --> FAIL #
########################################################################
class ValidRPATH7RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib64' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH7Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib64' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH7CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/usr/lib64' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH7CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/usr/lib64' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


#############################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/usr/lib64/sssd" --> FAIL #
#############################################################################
class ValidRPATH8RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib64/sssd' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH8Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib64/sssd' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH8CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/usr/lib64/sssd' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH8CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/usr/lib64/sssd' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


#################################################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/usr/lib64:/usr/lib64/firebird/intl" --> FAIL #
#################################################################################################
class ValidRPATH9RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib64:/usr/lib64/firebird/intl' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH9Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/lib64:/usr/lib64/firebird/intl' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH9CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/usr/lib64:/usr/lib64/firebird/intl' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH9CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/usr/lib64:/usr/lib64/firebird/intl' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


###############################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/usr/libexec/sudo" --> FAIL #
###############################################################################
class ValidRPATH10RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/libexec/sudo' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH10Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/libexec/sudo' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH10CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '/usr/libexec/sudo' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH10CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '/usr/libexec/sudo' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


##################################################################################################################################  # noqa: E266, E501
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/lib64:$ORIGIN/../etc:$ORIGIN/usr/lib64/:/builddir/build/BUILD/lib64" --> FAIL #  # noqa: E501
##################################################################################################################################  # noqa: E266, E501
class InvalidRPATH3RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/lib64:$ORIGIN/../etc:$ORIGIN/usr/lib64/:/builddir/build/BUILD/lib64' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH3Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/lib64:$ORIGIN/../etc:$ORIGIN/usr/lib64/:/builddir/build/BUILD/lib64' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH3CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/lib64:$ORIGIN/../etc:$ORIGIN/usr/lib64/:/builddir/build/BUILD/lib64' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidRPATH3CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += "patchelf --set-rpath '/lib64:$ORIGIN/../etc:$ORIGIN/usr/lib64/:/builddir/build/BUILD/lib64' a.out\n"  # noqa: E501

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


##############################################################################
# /usr/bin/program DT_RPATH or DT_RUNPATH set to "/usr/src/kernels/" --> pass #
##############################################################################
class ValidRPATH11RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/src/kernels/' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH11Koji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation()
        self.rpm.section_build += "patchelf --set-rpath '/usr/src/kernels/' a.out\n"

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH11CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '/usr/src/kernels/' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ValidRPATH11CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation()
        self.after_rpm.section_build += (
            "patchelf --set-rpath '/usr/src/kernels/' a.out\n"
        )

        self.inspection = "runpath"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
