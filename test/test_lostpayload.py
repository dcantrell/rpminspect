#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
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
