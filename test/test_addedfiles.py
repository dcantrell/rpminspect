#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import rpmfluff

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ["RPMINSPECT"], mode="rb") as f:
    ri_bytes = f.read()


# Forbidden path prefixes as defined in the config file -> BAD
class ForbiddenPathPrefixRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="var/tmp/whte_rbt.obj"
        )

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenPathPrefixKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="var/tmp/whte_rbt.obj"
        )

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenPathPrefixCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="var/tmp/whte_rbt.obj"
        )

        self.after_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="var/tmp/whte_rbt.obj"
        )

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenPathPrefixCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="var/tmp/whte_rbt.obj"
        )

        self.after_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="var/tmp/whte_rbt.obj"
        )

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Forbidden path suffixes as defined in the config file -> BAD
class ForbiddenPathSuffixRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/bin/simple.orig")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenPathSuffixKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/bin/simple.orig")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenPathSuffixCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_compilation(installPath="usr/bin/simple.orig")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/simple.orig")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenPathSuffixCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_compilation(installPath="usr/bin/simple.orig")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/simple.orig")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Forbidden directory as defined in the config file -> BAD
class ForbiddenDirectoryRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/bin/__MACOSX/simple")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenDirectoryKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/bin/__MACOSX/simple")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenDirectoryCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_compilation(installPath="usr/bin/__MACOSX/simple")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/__MACOSX/simple")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class ForbiddenDirectoryCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_compilation(installPath="usr/bin/__MACOSX/simple")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/__MACOSX/simple")

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Expected security mode as defined in fileinfo -> INFO
class ExpectedSecurityModeRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="4755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ExpectedSecurityModeKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="4755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ExpectedSecurityModeCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="4755"
        )
        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="4755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ExpectedSecurityModeCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="4755"
        )
        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="4755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Unexpected security file (not defined in fileinfo) -> BAD
# This check only happens for comparison runs because rpminspect needs a
# before build to know that something new was added.
class UnexpectedSecurityFileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # do not add the file in the before rpm, we want a new unexpected file
        self.after_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount",
            rpmfluff.SourceFile("mount.bin", ri_bytes),
            mode="0755",
        )

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnexpectedSecurityFileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # do not add the file in the before rpm, we want a new unexpected file
        self.after_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount",
            rpmfluff.SourceFile("mount.bin", ri_bytes),
            mode="0755",
        )

        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


# Existing security path file continues to exist -> OK
class SecurityFileContinuesToExistCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount",
            rpmfluff.SourceFile("mount.bin", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount",
            rpmfluff.SourceFile("mount.bin", ri_bytes),
            mode="0755",
        )

        self.inspection = "addedfiles"
        self.result = "OK"


class SecurityFileContinuesToExistCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount",
            rpmfluff.SourceFile("mount.bin", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount",
            rpmfluff.SourceFile("mount.bin", ri_bytes),
            mode="0755",
        )

        self.inspection = "addedfiles"
        self.result = "OK"


# New file shows up in after build for maintenance comparison -> VERIFY
class NewFileMaintBuildCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="0755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class NewFileMaintBuildCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="0755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# New file shows up in after build for rebase comparison -> INFO
class NewFileRebaseBuildCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="0755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class NewFileRebaseBuildCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", ri_bytes), mode="0755"
        )

        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
