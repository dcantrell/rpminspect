#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
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
