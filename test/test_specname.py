#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import unittest

from baseclass import TestSRPM, TestRPMs, TestKoji


# Verify spec filename matches package name on SRPM (OK)
class SpecNameSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.inspection = "specname"
        self.result = "OK"


# Verify spec filename matches package name on Koji build (OK)
class SpecNameKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.inspection = "specname"
        self.result = "OK"


# Verify spec filename test on binary RPMs fails (BAD)
class SpecNameRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.inspection = "specname"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Verify spec filename not matching package name fails (BAD)
class BadSpecNameSRPM(TestSRPM):
    @unittest.skip("requires addSpecBasename() support in rpmfluff")
    def setUp(self):
        super().setUp()
        self.rpm.addSpecBasename("badspecname")
        self.inspection = "specname"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Verify spec filename not matching package name fails on Koji build (BAD)
class BadSpecNameKojiBuild(TestKoji):
    @unittest.skip("requires addSpecBasename() support in rpmfluff")
    def setUp(self):
        super().setUp()
        self.rpm.addSpecBasename("badspecname")
        self.inspection = "specname"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"
