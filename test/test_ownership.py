#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import rpmfluff
import subprocess
import unittest

from baseclass import TestRPMs, TestKoji
from baseclass import TestCompareRPMs, TestCompareKoji

# Check that rpm is built with libcap
have_caps_support = False
lack_caps_msg = "rpm lacks %caps macro support"

proc = subprocess.Popen(
    ["ldd", "/usr/bin/rpmbuild"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
(out, err) = proc.communicate()

for line in out.split():
    if line.decode("utf-8").startswith("libcap"):
        have_caps_support = True
        break

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ["RPMINSPECT"], mode="rb") as f:
    ri_bytes = f.read()


#############
# bin owner #
#############


class SlashBinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /bin directory
    passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /sbin directory
    passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /usr/bin
    directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /usr/sbin
    directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /bin directory
    fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /sbin directory
    fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashBinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /usr/bin
    directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /usr/sbin
    directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by root in the /bin
    directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by root in the /sbin
    directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by root in the /usr/bin
    directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by root in the /usr/sbin
    directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by bin in the /bin
    directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by bin
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by bin in the /sbin
    directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by bin
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashBinOwnedByBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by bin in the /usr/bin
    directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by bin
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by bin in the /usr/sbin
    directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by bin
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by root
    in the /bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by root
    in the /sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by root
    in the /usr/bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by root
    in the /usr/sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by bin
    in the /bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by bin
    in the /sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashBinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by bin
    in the /usr/bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by bin
    in the /usr/sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    owned by root in the /bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    owned by root in the /sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    owned by root in the /usr.bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    owned by root in the /usr/sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    ]owned by bin in the /bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    owned by bin in the /sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashBinOwnedByBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    owned by bin in the /usr/bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    owned by bin in the /usr/sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


#############
# bin group #
#############


class SlashBinOwnedByGroupRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with root group permission
    in the /bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByGroupRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with root group permission
    in the /sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByGroupRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with root group permission
    in the /usr/bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByGroupRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with root group permission
    in the /usr/sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByGroupBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with bin group permission
    in the /bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByGroupBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with bin group permission
    in the /sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashiUsrSlashBinOwnedByGroupBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with bin group permission
    in the /usr/bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByGroupBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with bin group permission
    in the /usr/sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByGroupRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with root group permission
    in the /bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByGroupRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with root group permission
    in the /sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByGroupRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with root group permission
    in the /usr/bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByGroupRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with root group permission
    in the /usr/sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByGroupBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with bin group permission
    in the /bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByGroupBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with bin group permission
    in the /sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashBinOwnedByGroupBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with bin group permission
    in the /usr/bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByGroupBinKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with bin group permission
    in the /usr/sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with root group
    permissions in the /bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with root group
    permissions in the /sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with root group
    permissions in the /usr/bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with root group
    permissions in the /usr/sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with bin group
    permissions in the /bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with bin group
    permissions in the /sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashBinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with bin group
    permissions in the /usr/bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with bin group
    permissions in the /usr/sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with root group permissions in the /bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashSbinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with root group permissions in the /sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashBinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with root group permissions in the /usr/bin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashUsrSlashSbinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with root group permissions in the /usr/sbin directory passes.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "OK"


class SlashBinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with bin group permissions in the /bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashSbinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with bin group permissions in the /sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashBinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with bin group permissions in the /usr/bin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashUsrSlashSbinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file in the new build
    with bin group permissions in the /usr/sbin directory fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="usr/sbin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


###################
# Forbidden owner #
###################


class SlashBinOwnedByMockRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by mockbuild fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="mockbuild",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByMockKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by mockbuild fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="mockbuild",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByMockCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file owned by mockbuild
    in the new build fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="mockbuild",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByMockCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file owned by mockbuild
    in the new build fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="mockbuild",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


###################
# Forbidden group #
###################


class SlashBinOwnedByGroupMockRPMs(TestRPMs):
    """
    For binary RPM files, check that a file with
    mockbuild group permission fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="mockbuild",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByGroupMockKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with
    mockbuild group permission fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="mockbuild",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByGroupMockCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file with
    mockbuild group permission in the new build fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="mockbuild",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SlashBinOwnedByGroupMockCompareKoji(TestCompareKoji):
    """
    When comparing RPMs in Koji builds, check that a file with
    mockbuild group permission in the new build fails.
    """

    def setUp(self):
        super().setUp()

        # add file owned by root
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            owner="root",
            group="mockbuild",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


###################################
# CAP_SETUID and world executable #
###################################


class CapSETUIDWithOtherExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithOtherExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithOtherExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithOtherExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithOtherExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class CapSETUIDWithOtherExecKoji(TestKoji):
    """
    For RPMs from Koji builds, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithOtherExecKoji(TestKoji):
    """
    For RPMs from Koji builds, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithOtherExecKoji(TestKoji):
    """
    For RPMs from Koji builds, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithOtherExecKoji(TestKoji):
    """
    For RPMs from Koji builds, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithOtherExecKoji(TestKoji):
    """
    For RPMs from Koji builds, check that a file with CAP_SETUID set and
    world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class CapSETUIDWithOtherExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithOtherExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithOtherExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithOtherExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithOtherExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class CapSETUIDWithOtherExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithOtherExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithOtherExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithOtherExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithOtherExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


#################################
# CAP_SETUID and group writable #
#################################


class CapSETUIDWithGroupExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithGroupExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithGroupExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithGroupExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithGroupExecRPMs(TestRPMs):
    """
    For RPM binary files, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class CapSETUIDWithGroupExecKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithGroupExecKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithGroupExecKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithGroupExecKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithGroupExecKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file with CAP_SETUID set
    and group execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.rpm.add_source(rpmfluff.SourceFile("rpminspect", ri_bytes))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (
            sourceId,
            self.rpm.escape_path(installPath),
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class CapSETUIDWithGroupExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithGroupExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithGroupExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithGroupExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithGroupExecCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with CAP_SETUID set
    and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class CapSETUIDWithGroupExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/rpminspect"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapSETUIDWithGroupExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/skip"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "OK"


class SecurityINFORMCapSETUIDWithGroupExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/inform"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapSETUIDWithGroupExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/verify"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapSETUIDWithGroupExecCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that a file in the new build
    with CAP_SETUID set and world execution permissions fails.
    """

    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = "bin/fail"
        sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile("rpminspect", ri_bytes)
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n"
            % (sourceId, self.after_rpm.escape_path(installPath))
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += (
            "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Security"


#################
# Owner changed #
#################


class FileOwnerChangedCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that file ownership changing fails.
    """

    def setUp(self):
        super().setUp()

        # Start with root opwner
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
            owner="root",
            group="root",
        )

        # Switch to bin owner
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class FileOwnerChangedCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that
    file ownership changing fails.
    """

    def setUp(self):
        super().setUp()

        # Start with root opwner
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
            owner="root",
            group="root",
        )

        # Switch to bin owner
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
            owner="bin",
            group="root",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


#################
# Group changed #
#################


class FileGroupChangedCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that group ownership changing fails.
    """

    def setUp(self):
        super().setUp()

        # Start with root group
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
            owner="root",
            group="root",
        )

        # Switch to bin group
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class FileGroupChangedCompareKoji(TestCompareKoji):
    """
    When comparing RPMs from Koji builds, check that
    group ownership changing fails.
    """

    def setUp(self):
        super().setUp()

        # Start with root group
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", self.rpminspect),
            mode="0755",
            owner="root",
            group="root",
        )

        # Switch to bin group
        self.after_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", self.rpminspect),
            mode="0755",
            owner="root",
            group="bin",
        )

        self.inspection = "ownership"
        self.result = "BAD"
        self.waiver_auth = "Anyone"
