#
# Copyright 2019 Red Hat, Inc.
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
