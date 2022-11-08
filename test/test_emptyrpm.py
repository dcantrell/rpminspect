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

from baseclass import TestKoji, TestRPMs, TestSRPM


# Regular SRPM has payload (OK)
class HasPayloadSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.inspection = "emptyrpm"
        self.result = "OK"


# Regular RPMs have payload (OK)
class HasPayloadRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.inspection = "emptyrpm"
        self.result = "OK"


# Regular Koji build has payload (OK)
class HasPayloadKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.inspection = "emptyrpm"
        self.result = "OK"


# Regular package has empty payload (VERIFY)
class PkgHasEmptyPayload(TestRPMs):
    def setUp(self):
        super().setUp()
        self.inspection = "emptyrpm"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Packages in Koji build have empty payloads (VERIFY)
class KojiBuildHaveEmptyPayloads(TestKoji):
    def setUp(self):
        super().setUp()
        self.inspection = "emptyrpm"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
