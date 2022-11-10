#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
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
