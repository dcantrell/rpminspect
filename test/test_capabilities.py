#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

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


# package contains a file with capabilities(7) but it is not on the
# list (BAD)
class UnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


# package contains a file with approved capabilities(7) (OK)
class ApprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ApprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class ApprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class ApprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecurityFAILUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecurityINFORMUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecurityINFORMCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecuritySKIPUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"


class SecurityINFORMUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_caps_support, lack_caps_msg)
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"
