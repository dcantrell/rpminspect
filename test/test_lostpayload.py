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

from baseclass import TestCompareKoji


# New package has empty payload across Koji builds (VERIFY)
class NewPkgHasEmptyPayload(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.after_rpm.add_subpackage(self.after_rpm.name + "-newthing")
        self.inspection = "lostpayload"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Packages continue to be empty (INFO)
class PkgStillHasEmptyPayload(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.inspection = "lostpayload"
        self.result = "INFO"


# Package lost payload across Koji builds (VERIFY)
class PkgLostPayload(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.inspection = "lostpayload"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Existing package is now missing across Koji builds (VERIFY)
class ExistingPkgMissing(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.before_rpm.add_subpackage(self.before_rpm.name + "-newthing")
        self.inspection = "lostpayload"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
