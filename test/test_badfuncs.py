#
# Copyright 2021 Red Hat, Inc.
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

import os
from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]

# Source code used for the forbidden IPv6 function tests
forbidden_ipv6_src = open(datadir + "/forbidden-ipv6.c").read()


# Program uses forbidden IPv6 function
class ForbiddenIPv6FunctionRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.inspection = "badfuncs"
        self.waiver_auth = "Anyone"
        self.result = "VERIFY"


class ForbiddenIPv6FunctionKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.inspection = "badfuncs"
        self.waiver_auth = "Anyone"
        self.result = "VERIFY"


class ForbiddenIPv6FunctionCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.after_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.inspection = "badfuncs"
        self.waiver_auth = "Anyone"
        self.result = "VERIFY"


class ForbiddenIPv6FunctionCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.after_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.inspection = "badfuncs"
        self.waiver_auth = "Anyone"
        self.result = "VERIFY"


# Program uses forbidden IPv6 function, but is explicitly allowed
class AllowedForbiddenIPv6FunctionRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        self.extra_cfg = {}
        self.extra_cfg["badfuncs"] = {}
        self.extra_cfg["badfuncs"]["allowed"] = {}
        self.extra_cfg["badfuncs"]["allowed"]["/usr/bin/hello-world"] = ["inet_aton"]

        self.rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)

        self.inspection = "badfuncs"
        self.result = "OK"


class AllowedForbiddenIPv6FunctionKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.extra_cfg = {}
        self.extra_cfg["badfuncs"] = {}
        self.extra_cfg["badfuncs"]["allowed"] = {}
        self.extra_cfg["badfuncs"]["allowed"]["/usr/bin/hello-world"] = ["inet_aton"]

        self.rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)

        self.inspection = "badfuncs"
        self.result = "OK"


class AllowedForbiddenIPv6FunctionCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.extra_cfg = {}
        self.extra_cfg["badfuncs"] = {}
        self.extra_cfg["badfuncs"]["allowed"] = {}
        self.extra_cfg["badfuncs"]["allowed"]["/usr/bin/hello-world"] = ["inet_aton"]

        self.before_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.after_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)

        self.inspection = "badfuncs"
        self.result = "OK"


class AllowedForbiddenIPv6FunctionCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.extra_cfg = {}
        self.extra_cfg["badfuncs"] = {}
        self.extra_cfg["badfuncs"]["allowed"] = {}
        self.extra_cfg["badfuncs"]["allowed"]["/usr/bin/hello-world"] = ["inet_aton"]

        self.before_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)
        self.after_rpm.add_simple_compilation(sourceContent=forbidden_ipv6_src)

        self.inspection = "badfuncs"
        self.result = "OK"
