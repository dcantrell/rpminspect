#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import platform

from baseclass import TestCompareRPMs, TestCompareSRPM, TestCompareKoji

machine = platform.machine().replace("amd64", "x86_64")

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ["RPMINSPECT"], mode="rb") as f:
    ri_bytes = f.read()


# Test rebase reporting level set to VERIFY for external package dependencies
class RebaseReportingLevelVerifyExternalSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_build_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_build_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


class RebaseReportingLevelVerifyExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


class RebaseReportingLevelVerifyExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


# Test rebase reporting level set to BAD for external package dependencies
class RebaseReportingLevelBadExternalSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_build_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_build_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to BAD
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


class RebaseReportingLevelBadExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to BAD
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


class RebaseReportingLevelBadExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to BAD
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


# Test that internal subpackage dependencies are always reported at INFO level
# even when rebase_reporting_level is set to VERIFY or BAD


class InternalSubpackageDepWithVerifyLevelRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # Create a libs subpackage
        self.after_rpm.add_subpackage("libs")

        # internal subpackage dependency - should always be INFO
        # regardless of rebase_reporting_level setting
        self.before_rpm.add_requires("%{name}-libs")
        self.after_rpm.add_requires("%{name}-libs")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY, but internal deps stay INFO
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


class InternalSubpackageDepWithVerifyLevelKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # Create a libs subpackage
        self.after_rpm.add_subpackage("libs")

        # internal subpackage dependency - should always be INFO
        # regardless of rebase_reporting_level setting
        self.before_rpm.add_requires("%{name}-libs")
        self.after_rpm.add_requires("%{name}-libs")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY, but internal deps stay INFO
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


# Test rebase reporting level defaults to INFO for external deps
class RebaseReportingLevelDefaultExternalSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should default to INFO
        self.before_rpm.add_build_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_build_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RebaseReportingLevelDefaultExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should default to INFO
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RebaseReportingLevelDefaultExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should default to INFO
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Maintenance builds report external dependency changes at VERIFY by default
class MaintReportingLevelDefaultExternalSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # source package changes are expected and stay INFO
        self.before_rpm.add_build_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_build_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class MaintReportingLevelDefaultExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class MaintReportingLevelDefaultExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Maintenance builds lose a dependency (VERIFY)
class MaintLostExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires("external-somepkg = 1.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class MaintLostExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires("external-somepkg = 1.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# rebase_reporting_level does not apply to maintenance builds
class MaintReportingLevelIgnoresSettingRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to BAD, maint builds stay VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


class MaintReportingLevelIgnoresSettingKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to BAD, maint builds stay VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


# Rebase builds lose a dependency (follows rebase_reporting_level)
class RebaseLostExternalWithVerifyLevelRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires("external-somepkg = 1.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

        # Set rebase_reporting_level to VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


# Retained dependencies are INFO even with rebase_reporting_level set
class RebaseRetainedExternalWithVerifyLevelRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 1.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


# The package's own dependency changes are INFO in a rebase regardless
# of rebase_reporting_level
class RebaseOwnDepsWithBadLevelSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # BuildRequires that follow the package version, older rpm
        # versions do not add a self Provides to source packages
        self.before_rpm.add_build_requires("%{name}-devel = %{version}-%{release}")
        self.after_rpm.add_build_requires("%{name}-devel = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to BAD
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


class RebaseOwnDepsWithBadLevelRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # subpackage Provides and versioned Requires follow the package version
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")
        self.before_rpm.add_requires("%{name}-libs = %{version}-%{release}")
        self.after_rpm.add_requires("%{name}-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to BAD
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


class RebaseOwnDepsWithBadLevelKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # subpackage Provides and versioned Requires follow the package version
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")
        self.before_rpm.add_requires("%{name}-libs = %{version}-%{release}")
        self.after_rpm.add_requires("%{name}-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to BAD
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


# Only INFO results carry the "this is expected" message suffix
class RebaseReportingLevelDefaultMessageRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
        self.message = (
            "Changed 'Requires: external-somepkg = 1.0-1' to "
            "'Requires: external-somepkg = 2.0-1' in subpackage "
            f"vaporware on {machine}; this is expected"
        )


class RebaseReportingLevelVerifyMessageRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = (
            "Changed 'Requires: external-somepkg = 1.0-1' to "
            "'Requires: external-somepkg = 2.0-1' in subpackage "
            f"vaporware on {machine}"
        )

        # Set rebase_reporting_level to VERIFY
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "VERIFY"}}


class RebaseReportingLevelBadMessageSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_build_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_build_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"
        self.message = (
            "Changed 'Requires: external-somepkg = 1.0-1' to "
            "'Requires: external-somepkg = 2.0-1' in source package vaporware"
        )

        # Set rebase_reporting_level to BAD
        self.extra_cfg = {"rpmdeps": {"rebase_reporting_level": "BAD"}}


class MaintReportingLevelDefaultMessageRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = (
            "Changed 'Requires: external-somepkg = 1.0-1' to "
            "'Requires: external-somepkg = 2.0-1' in subpackage "
            f"vaporware on {machine}"
        )
