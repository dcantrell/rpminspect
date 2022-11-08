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

from baseclass import TestKoji
from baseclass import TestRPMs
from baseclass import TestSRPM
from baseclass import TestCompareKoji
from baseclass import TestCompareRPMs
from baseclass import TestCompareSRPM


class DefaultSRPM(TestSRPM):
    """
    Verify rpminspect runs against an SRPM
    """

    def setUp(self):
        super().setUp()
        self.result = "OK"


class DefaultRPMs(TestRPMs):
    """
    Verify rpminspect runs against an RPM
    """

    def setUp(self):
        super().setUp()
        self.result = "OK"


class DefaultKoji(TestKoji):
    """
    Verify rpminspect runs against a Koji build
    """

    def setUp(self):
        super().setUp()
        self.result = "OK"


class DefaultCompareSRPM(TestCompareSRPM):
    """
    Verify rpminspect runs against two SRPMs
    """

    def setUp(self):
        super().setUp()
        self.result = "OK"


class DefaultCompareRPMs(TestCompareRPMs):
    """
    Verify rpminspect runs against two RPMs
    """

    def setUp(self):
        super().setUp()
        self.result = "OK"


class DefaultCompareKoji(TestCompareKoji):
    """
    Verify rpminspect runs against two Koji builds
    """

    def setUp(self):
        super().setUp()
        self.result = "OK"
