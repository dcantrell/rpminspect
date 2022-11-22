#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import unittest
from baseclass import TestCompareRPMs, TestCompareKoji, TestRPMs, TestKoji

needed_flags = "-Wl,-z,now -fcf-protection=full -fplugin=annobin -O2 \
        -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS -fstack-protector-strong \
        -fstack-clash-protection -flto"

# This check requires the annocheck executable in the PATH
have_annocheck = True

if os.system("annocheck --help >/dev/null 2>&1") != 0:
    have_annocheck = False


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        if os.path.isfile("/etc/slackware-version"):
            self.extra_cfg = {}
            self.extra_cfg["annocheck"] = {}
            self.extra_cfg["annocheck"]["jobs"] = [
                {
                    "hardened": "--ignore-unknown --verbose --skip-cf-protection --skip-property-note"  # noqa: E501
                }
            ]

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        if os.path.isfile("/etc/slackware-version"):
            self.extra_cfg = {}
            self.extra_cfg["annocheck"] = {}
            self.extra_cfg["annocheck"]["jobs"] = [
                {
                    "hardened": "--ignore-unknown --verbose --skip-cf-protection --skip-property-note"  # noqa: E501
                }
            ]

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        if os.path.isfile("/etc/slackware-version"):
            self.extra_cfg = {}
            self.extra_cfg["annocheck"] = {}
            self.extra_cfg["annocheck"]["jobs"] = [
                {
                    "hardened": "--ignore-unknown --verbose --skip-cf-protection --skip-property-note"  # noqa: E501
                }
            ]

        self.rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedKoji(TestKoji):
    def setUp(self):
        super().setUp()

        if os.path.isfile("/etc/slackware-version"):
            self.extra_cfg = {}
            self.extra_cfg["annocheck"] = {}
            self.extra_cfg["annocheck"]["jobs"] = [
                {
                    "hardened": "--ignore-unknown --verbose --skip-cf-protection --skip-property-note"  # noqa: E501
                }
            ]

        self.rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
