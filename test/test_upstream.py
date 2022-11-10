#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os

import rpmfluff

from baseclass import (
    TestSRPM,
    TestRPMs,
    TestKoji,
    TestCompareSRPM,
    TestCompareKoji,
)

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ["RPMINSPECT"], mode="rb") as f:
    ri_bytes = f.read()

###################################
# Single SRPM, RPM, or Koji build #
###################################


class SkipUpstreamSRPM(TestSRPM):
    """
    Test that single source RPM runs skip the upstream inspection.
    """

    def setUp(self):
        super().setUp()

        # tell it to pass '-T upstream' to rpminspect
        self.inspection = "upstream"

        # since it should skip, only look for our diagnostic output
        self.result_inspection = "diagnostics"
        self.result = "DIAGNOSTICS"

    def runTest(self):
        super().runTest()

        # make sure 'upstream' is not in the results
        self.assertFalse(self.inspection in self.results.keys())


class SkipUpstreamRPMs(TestRPMs):
    """
    Test that single binary RPM runs skip the upstream inspection.
    """

    def setUp(self):
        super().setUp()

        # tell it to pass '-T upstream' to rpminspect
        self.inspection = "upstream"

        # since it should skip, only look for our diagnostic output
        self.result_inspection = "diagnostics"
        self.result = "DIAGNOSTICS"

    def runTest(self):
        super().runTest()

        # make sure 'upstream' is not in the results
        self.assertFalse(self.inspection in self.results.keys())


class SkipUpstreamKoji(TestKoji):
    """
    Test that single Koji build runs skip the upstream inspection.
    """

    def setUp(self):
        super().setUp()

        # tell it to pass '-T upstream' to rpminspect
        self.inspection = "upstream"

        # since it should skip, only look for our diagnostic output
        self.result_inspection = "diagnostics"
        self.result = "DIAGNOSTICS"

    def runTest(self):
        super().runTest()

        # make sure 'upstream' is not in the results
        self.assertFalse(self.inspection in self.results.keys())


##########################
# Source RPM comparisons #
##########################


class DiffVerUpstreamCompareSRPM(TestCompareSRPM):
    """
    Test that comparing two SRPMs with different versions gives an OK
    result in the upstream inspection.
    """

    def setUp(self):
        super().setUp()
        self.inspection = "upstream"
        self.result = "OK"


class SameVerChangeUpstreamCompareSRPM(TestCompareSRPM):
    """
    Test that comparing two SRPMs with the same version but changing
    Source files in the package gives a VERIFY result in the upstream
    inspection.
    """

    def setUp(self):
        # recreate the after RPM with the same version as the before
        super().setUp(same=True)

        # add the same installed target file with different sources
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_simple_compilation(installPath="bin/rpminspect")
        self.after_rpm.section_install += "chmod 0755 $RPM_BUILD_ROOT/bin/rpminspect\n"

        # what we are checking
        self.inspection = "upstream"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class SameVerRemoveUpstreamCompareSRPM(TestCompareSRPM):
    """
    Test that comparing two SRPMs with the same version but removing
    a Source file in the package gives a VERIFY result in the upstream
    inspection.
    """

    def setUp(self):
        # recreate the after RPM with the same version as the before
        super().setUp(same=True)

        # simulate a removed source file by not including it in the after rpm
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )

        # what we are checking
        self.inspection = "upstream"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


##########################
# Koji build comparisons #
##########################


class DiffVerUpstreamCompareKoji(TestCompareKoji):
    """
    Test that comparing two Koji builds with different versions gives
    an OK result in the upstream inspection.

    """

    def setUp(self):
        super().setUp()
        self.inspection = "upstream"
        self.result = "OK"


class SameVerChangeUpstreamCompareKoji(TestCompareKoji):
    """
    Test that comparing two Koji builds with the same version but changing
    Source files in the package gives a VERIFY result in the upstream
    inspection.
    """

    def setUp(self):
        # recreate the after RPM with the same version as the before
        super().setUp(same=True)

        # add the same installed target file with different sources
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )
        self.after_rpm.add_simple_compilation(installPath="bin/rpminspect")
        self.after_rpm.section_install += "chmod 0755 $RPM_BUILD_ROOT/bin/rpminspect\n"

        # what we are checking
        self.inspection = "upstream"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class SameVerRemoveUpstreamCompareKoji(TestCompareKoji):
    """
    Test that comparing two Koji builds with the same version but removing
    a Source file in the package gives a VERIFY result in the upstream
    inspection.
    """

    def setUp(self):
        # recreate the after RPM with the same version as the before
        super().setUp(same=True)

        # simulate a removed source file by not including it in the after rpm
        self.before_rpm.add_installed_file(
            installPath="bin/rpminspect",
            sourceFile=rpmfluff.SourceFile("rpminspect", ri_bytes),
            mode="0755",
        )

        # what we are checking
        self.inspection = "upstream"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
