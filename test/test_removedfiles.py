#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import platform
import rpmfluff

from baseclass import TestCompareRPMs, TestCompareKoji

machine = platform.machine().replace("amd64", "x86_64")


class FileNoRemovedRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )
        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", after_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "OK"


class FileNoRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )
        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", after_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "OK"


# File removed from an after build but it's not a security-related file (OK)
class FileRemovedRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# File removed from an after build from a security path (BAD)
class SecurityFileRemovedRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # create the test packages
        self.before_rpm.add_installed_file(
            "etc/sudoers.d/s3kr1t",
            rpmfluff.SourceFile("s3kr1t.sudo", "# This is a security related file!"),
            mode="0644",
        )

        self.inspection = "removedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileRemovedRebaseRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class FileRemovedSecurityRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount.so",
            rpmfluff.SourceFile("mount.so", before_src),
        )

        self.inspection = "removedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class SecurityFileRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create the test packages
        self.before_rpm.add_installed_file(
            "etc/sudoers.d/s3kr1t",
            rpmfluff.SourceFile("s3kr1t.sudo", "# This is a security related file!"),
            mode="0644",
        )

        self.inspection = "removedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileRemovedRebaseKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Removed file matches a security rule without a securitypath action (INFO)
class FileRemovedOtherSecruleKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # /usr/sbin/verify has caps and execstack rules in the test data
        self.before_rpm.add_installed_file(
            "usr/sbin/verify",
            rpmfluff.SourceFile("verify.bin", before_src),
            mode="0755",
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
        self.message = f"/usr/sbin/verify removed from package vaporware on {machine}"


# Removed PIE executable is not reported as a library (INFO)
class ExecutableRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_compilation(
            compileFlags="-fPIE -pie", installPath="usr/bin/hello-world"
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
        self.message = (
            f"/usr/bin/hello-world removed from package vaporware on {machine}"
        )


# Removed shared library is reported as an ABI break (INFO)
class LibraryRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so.1",
            installPath="usr/lib/libvaporware.so.1",
            compileFlags="-Wl,-soname,libvaporware.so.1",
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
        self.message = (
            "ABI break: Library /usr/lib/libvaporware.so.1 with SONAME "
            f"'libvaporware.so.1' removed from package vaporware on {machine}"
        )
