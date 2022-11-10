#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import rpmfluff

from baseclass import TestSRPM, TestRPMs, TestKoji
from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji


class ExecutableWithSetUIDSRPM(TestSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class ExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ExecutableWithSetUIDCompareSRPM(TestCompareSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class ExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class UnapprovedExecutableWithSetUIDSRPM(TestSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class UnapprovedExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnapprovedExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMUnapprovedExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnapprovedExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMUnapprovedExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedExecutableWithSetUIDCompareSRPM(TestCompareSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class UnapprovedExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnapprovedExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMUnapprovedExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnapprovedExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMUnapprovedExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable file is in a package,
    OK result occurs when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    OK result occurs when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/skip", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMFileIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/inform", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/verify", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/fail", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable dir is in a package,
    OK result occures when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "0757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable dir is in a package,
    OK result occures when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "0757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    OK result occures when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "1757"
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    OK result occures when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/dumping-ground"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPDirIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/skip"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "OK"


class SecurityINFORMDirIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/inform"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYDirIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/verify"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILDirIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        d = "usr/share/fail"
        m = "1757"
        self.after_rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/%s\n" % d
        self.after_rpm.section_install += "chmod %s $RPM_BUILD_ROOT/%s\n" % (m, d)
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(%s,-,-) /%s\n" % (m, d)
        self.after_rpm.add_payload_check(d, None)

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"
