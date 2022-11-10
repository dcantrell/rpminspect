#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import rpmfluff
from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji

rpminspect_sh = """#!/bin/sh
echo nope
"""


class SameTypeCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")
        self.after_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")

        self.inspection = "types"
        self.result = "OK"


class SameTypeCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")
        self.after_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")

        self.inspection = "types"
        self.result = "OK"


class SameTypeCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")
        self.after_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")

        self.inspection = "types"
        self.result = "OK"


class ChangedTypeCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.SourceFile("rpminspect.source", rpminspect_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.GeneratedSourceFile("rpminspect.source", rpmfluff.make_png()),
        )

        self.inspection = "types"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangedTypeCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.SourceFile("rpminspect.sh", rpminspect_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.GeneratedSourceFile("rpminspect.png", rpmfluff.make_png()),
        )

        self.inspection = "types"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangedTypeCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.SourceFile("rpminspect.sh", rpminspect_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.GeneratedSourceFile("rpminspect.png", rpmfluff.make_png()),
        )

        self.inspection = "types"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
