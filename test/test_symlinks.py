#
# Copyright 2020 Red Hat, Inc.
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
import unittest
import rpm
import rpmfluff
from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

# more recent versions of RPM prevent dangling and too long symlinks
# b'4.15.90' -> [4, 15, 90]
# b'4.16.0-beta' -> [4, 16, 0]
# b'4.14.2.1' -> [4, 14, 2, 1]
# Ignore the fourth position and beyond on the version number.
rpmver = list(map(lambda x: int(x), rpm.__version__.strip().split("-")[0].split(".")))
rpm_major = rpmver[0]
rpm_minor = rpmver[1]
rpm_update = rpmver[2]

if (
    rpm_major < 4
    or (rpm_major == 4 and rpm_minor < 15)
    or (rpm_major == 4 and rpm_minor == 15 and rpm_update < 90)
):
    rpm_handles_symlinks = False
else:
    rpm_handles_symlinks = True

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ["RPMINSPECT"], mode="rb") as f:
    ri_bytes = f.read()


# Absolute symlink exists (OK)
class AbsoluteSymlinkExistsRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/testscript",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/sbin/testscript", "/usr/bin/testscript")

        self.inspection = "symlinks"
        self.result = "OK"


class AbsoluteSymlinkExistsKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/sbin/rpminspect", "/usr/bin/rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


class AbsoluteSymlinkExistsCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "/usr/bin/rpminspect"
        )

        self.inspection = "symlinks"
        self.result = "OK"


class AbsoluteSymlinkExistsCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "/usr/bin/rpminspect"
        )

        self.inspection = "symlinks"
        self.result = "OK"


# Relative symlink with ../ exists (OK)
class RelativeSymlinkExistsParentDirRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/sbin/rpminspect", "../bin/rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


class RelativeSymlinkExistsParentDirKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/sbin/rpminspect", "../bin/rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


class RelativeSymlinkExistsParentDirCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/sbin/rpminspect", "../bin/rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


class RelativeSymlinkExistsParentDirCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/sbin/rpminspect", "../bin/rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


# Relative symlink in current directory exists (OK)
class RelativeSymlinkExistsCurrentDirRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/bin/anotherrpminspect", "rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


class RelativeSymlinkExistsCurrentDirKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/bin/anotherrpminspect", "rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


class RelativeSymlinkExistsCurrentDirCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/bin/anotherrpminspect", "rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


class RelativeSymlinkExistsCurrentDirCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/bin/anotherrpminspect", "rpminspect")

        self.inspection = "symlinks"
        self.result = "OK"


# Symlink that exists spans subpackages (OK)
class SymlinkExistsMultiplePackagesRPMS(TestRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_subpackage("symlinks")
        self.rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "rpminspect", subpackageSuffix="symlinks"
        )

        self.inspection = "symlinks"
        self.result = "OK"


class SymlinkExistsMultiplePackagesKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_subpackage("symlinks")
        self.rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "rpminspect", subpackageSuffix="symlinks"
        )

        self.inspection = "symlinks"
        self.result = "OK"


class SymlinkExistsMultiplePackagesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_subpackage("symlinks")
        self.after_rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "rpminspect", subpackageSuffix="symlinks"
        )

        self.inspection = "symlinks"
        self.result = "OK"


class SymlinkExistsMultiplePackagesCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_subpackage("symlinks")
        self.after_rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "rpminspect", subpackageSuffix="symlinks"
        )

        self.inspection = "symlinks"
        self.result = "OK"


# Absolute symlink is dangling (VERIFY)
class AbsoluteSymlinkDanglingRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "/usr/bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AbsoluteSymlinkDanglingKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "/usr/bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AbsoluteSymlinkDanglingCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "/usr/bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AbsoluteSymlinkDanglingCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "/usr/bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Relative symlink with ../ is dangling (VERIFY)
class RelativeSymlinkDanglingParentDirRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "../bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RelativeSymlinkDanglingParentDirKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "../bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RelativeSymlinkDanglingParentDirAfterRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "../bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RelativeSymlinkDanglingParentDirAfterKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/sbin/rpminspect", "../bin/anotherrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Relative symlink in current directory is dangling (VERIFY)
class RelativeSymlinkDanglingCurrentDirRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "originalrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RelativeSymlinkDanglingCurrentDirKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "originalrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RelativeSymlinkDanglingCurrentDirCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "originalrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RelativeSymlinkDanglingCurrentDirCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect", "originalrpminspect"
        )

        self.inspection = "symlinks"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Too many symlink cycles (VERIFY)
# To trigger an ELOOP on symlink resolution, you need to have more
# than 40 levels of symlink redirection, per path_resolution(7).  I use 47
# levels here just to make sure.
class TooManySymlinkLevelsRPMs(TestRPMs):
    @unittest.skipIf(
        rpm_handles_symlinks,
        "rpm %d.%d.%d detected, prevents ELOOP symlink errors"
        % (rpm_major, rpm_minor, rpm_update),
    )
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/bin/bin", ".")
        self.rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect",
            "bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin"
            "/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bi"
            "n/bin/bin/bin/bin/bin/bin/bin/rpminspect",
        )

        self.inspection = "symlinks"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TooManySymlinkLevelsKoji(TestKoji):
    @unittest.skipIf(
        rpm_handles_symlinks,
        "rpm %d.%d.%d detected, prevents ELOOP symlink errors"
        % (rpm_major, rpm_minor, rpm_update),
    )
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.rpm.add_installed_symlink("usr/bin/bin", ".")
        self.rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect",
            "bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin"
            "/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bi"
            "n/bin/bin/bin/bin/bin/bin/bin/rpminspect",
        )

        self.inspection = "symlinks"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TooManySymlinkLevelsCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(
        rpm_handles_symlinks,
        "rpm %d.%d.%d detected, prevents ELOOP symlink errors"
        % (rpm_major, rpm_minor, rpm_update),
    )
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/bin/bin", ".")
        self.after_rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect",
            "bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin"
            "/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bi"
            "n/bin/bin/bin/bin/bin/bin/bin/rpminspect",
        )

        # disable check-buildroot rpmbuild script to avoid stat errors
        # on our big symlink
        self.after_rpm.header += "\n%global __arch_install_post %{nil}\n"

        self.inspection = "symlinks"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class TooManySymlinkLevelsCompareKoji(TestCompareKoji):
    @unittest.skipIf(
        rpm_handles_symlinks,
        "rpm %d.%d.%d detected, prevents ELOOP symlink errors"
        % (rpm_major, rpm_minor, rpm_update),
    )
    def setUp(self):
        super().setUp()

        # add file and symlink
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/bin/bin", ".")
        self.after_rpm.add_installed_symlink(
            "usr/bin/anotherrpminspect",
            "bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin"
            "/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bi"
            "n/bin/bin/bin/bin/bin/bin/bin/rpminspect",
        )

        # disable check-buildroot rpmbuild script to avoid stat errors
        # on our big symlink
        self.after_rpm.header += "\n%global __arch_install_post %{nil}\n"

        self.inspection = "symlinks"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Directory becomes a symlink (VERIFY)
class DirectoryBecomesSymlinkCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add directories and symlinks
        self.before_rpm.add_installed_directory("usr/share/testdirectory")
        self.before_rpm.add_installed_directory("usr/share/actualdirectory")
        self.after_rpm.add_installed_symlink(
            "usr/share/testdirectory", "actualdirectory"
        )
        self.after_rpm.add_installed_directory("usr/share/actualdirectory")

        self.inspection = "symlinks"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class DirectoryBecomesSymlinkCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add directories and symlinks
        self.before_rpm.add_installed_directory("usr/share/testdirectory")
        self.before_rpm.add_installed_directory("usr/share/actualdirectory")
        self.after_rpm.add_installed_symlink(
            "usr/share/testdirectory", "actualdirectory"
        )
        self.after_rpm.add_installed_directory("usr/share/actualdirectory")

        self.inspection = "symlinks"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Non-directory becomes a symlink (VERIFY)
class NonDirectoryBecomesSymlinkCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add files and symlinks
        self.before_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.before_rpm.add_installed_file(
            installPath="usr/bin/anotherrpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/bin/anotherrpminspect", "rpminspect")

        self.inspection = "symlinks"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class NonDirectoryBecomesSymlinkCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add files and symlinks
        self.before_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.before_rpm.add_installed_file(
            installPath="usr/bin/anotherrpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_symlink("usr/bin/anotherrpminspect", "rpminspect")

        self.inspection = "symlinks"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
