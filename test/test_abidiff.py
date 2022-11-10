#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
from baseclass import TestCompareRPMs, TestCompareKoji

# Simple source files for library ABI tests
old_library_source = """#include <math.h>

double exponent(double x, double y)
{
    return pow(x, y);
}
"""

new_library_source = """#include <stdio.h>

void exponent(void)
{
    return;
}
"""


# Test two builds that are not a rebase and do not change the ABI in
# an incompatible way (OK)
class AbidiffNoRebaseNoABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        self.inspection = "abidiff"
        self.result = "OK"


class AbidiffNoRebaseNoABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        self.inspection = "abidiff"
        self.result = "OK"


# Test two builds that are a rebase and do not break the ABI (OK)
class AbidiffRebaseNoABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"


class AbidiffRebaseNoABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"


# Test two builds that are a rebase and break the ABI (OK)
class AbidiffRebaseWithABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # work around a limitation or difference on musl-based toolchains
        if os.path.isfile("/etc/alpine-release"):
            self.extra_cfg = {}
            self.extra_cfg["abidiff"] = {}
            self.extra_cfg["abidiff"]["extra_args"] = "--no-unreferenced-symbols"

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.before_rpm.add_simple_library(sourceContent=old_library_source)
        self.after_rpm.add_simple_library(sourceContent=new_library_source)

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"


class AbidiffRebaseWithABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # work around a limitation or difference on musl-based toolchains
        if os.path.isfile("/etc/alpine-release"):
            self.extra_cfg = {}
            self.extra_cfg["abidiff"] = {}
            self.extra_cfg["abidiff"]["extra_args"] = "--no-unreferenced-symbols"

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.before_rpm.add_simple_library(sourceContent=old_library_source)
        self.after_rpm.add_simple_library(sourceContent=new_library_source)

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"


# Test two builds that are not a rebase and break the ABI (VERIFY)
class AbidiffNoRebaseWithABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            sourceContent=old_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )
        self.after_rpm.add_simple_library(
            sourceContent=new_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )

        self.result = "VERIFY"
        self.inspection = "abidiff"
        self.waiver_auth = "Anyone"


class AbidiffNoRebaseWithABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            sourceContent=old_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )
        self.after_rpm.add_simple_library(
            sourceContent=new_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )

        self.inspection = "abidiff"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
