#
# Copyright 2020 Red Hat, Inc.
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

import datetime

from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji


# changelog tests only apply to before & after build comparisons.
# Perform the following each as a separate test:
#
# In the source RPM:
# 1) Remove the %changelog section in the after build.  The test should
#    report that as VERIFY.
class LostChangeLogCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # create a simple changelog
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = clog
        self.after_rpm.section_changelog = None

        self.inspection = "changelog"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class LostChangeLogCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # create a simple changelog
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = clog
        self.after_rpm.section_changelog = None

        self.inspection = "changelog"
        # this is INFO because it's only comapring the binary RPMs,
        # the other checks are for SRPMs
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class LostChangeLogCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create a simple changelog
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = clog
        self.after_rpm.section_changelog = None

        self.inspection = "changelog"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# 2) Prevent a %changelog in the before build but add one in the after
#    build.  The test should report that as INFO.
class GainedChangeLogCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # create a simple changelog
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = None
        self.after_rpm.section_changelog = clog

        self.inspection = "changelog"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainedChangeLogCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # create a simple changelog
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = None
        self.after_rpm.section_changelog = clog

        self.inspection = "changelog"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainedChangeLogCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create a simple changelog
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = None
        self.after_rpm.section_changelog = clog

        self.inspection = "changelog"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# 3) If the first entries in the changelog as the same in the before and
#    after build, the test should report it as BAD.
class SameChangeLogCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # create a simple changelog and duplicate it
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = clog
        self.after_rpm.section_changelog = clog

        self.inspection = "changelog"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class SameChangeLogCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create a simple changelog and duplicate it
        today = datetime.date.today().strftime("%a %b %d %Y")
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = clog
        self.after_rpm.section_changelog = clog

        self.inspection = "changelog"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# In the binary RPM:
# 1) Change some lines in the after build's %changelog.  The test should
#    report that as INFO.
class BalancedChangeLogEditCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create a simple prefix that will result in a one line edit script
        today = datetime.date.today().strftime("%a %b %d %Y")
        before_prefix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n\n" % today
        )
        after_prefix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 7.47-1\n"
            "- Upgrade to the latest and greatest\n\n" % today
        )
        suffix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 1.0\n"
            "- Initial package\n" % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = before_prefix + suffix
        self.after_rpm.section_changelog = after_prefix + suffix

        self.inspection = "changelog"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class UnbalancedChangeLogEditCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create a simple prefix that will result in a multiline removal
        today = datetime.date.today().strftime("%a %b %d %Y")
        prefix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-2\n"
            "- Another upgrade\n\n" % today
        )
        clog = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n\n" % today
        )
        suffix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 1.0\n"
            "- Initial package\n" % today
        )

        # modify the changelog
        self.before_rpm.section_changelog = clog + suffix
        self.after_rpm.section_changelog = prefix + suffix

        self.inspection = "changelog"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# 2) Only add a new entry to the %changelog in the after build.  This
#    should report as INFO.
class AddChangeLogEntryCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # create a simple prefix that will result in a new entry
        today = datetime.date.today().strftime("%a %b %d %Y")
        after_prefix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n\n" % today
        )
        suffix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 1.0\n- Initial package\n"
            % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = suffix
        self.after_rpm.section_changelog = after_prefix + suffix

        # The result is OK here because this is just the SRPM
        # comparison.  The changelog text comparison does not report
        # new entries except when comparing built packages.  the
        # reason for that is to account for macro expansion failures.
        # We want to check here for OK anyway because any other return
        # indicates a failure in the SRPM only changelog inspection.
        self.inspection = "changelog"
        self.result = "OK"


class AddChangeLogEntryCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # create a simple prefix that will result in a new entry
        today = datetime.date.today().strftime("%a %b %d %Y")
        after_prefix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n\n" % today
        )
        suffix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 1.0\n- Initial package\n"
            % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = suffix
        self.after_rpm.section_changelog = after_prefix + suffix

        self.inspection = "changelog"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AddChangeLogEntryCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create a simple prefix that will result in a new entry
        today = datetime.date.today().strftime("%a %b %d %Y")
        after_prefix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and greatest\n\n" % today
        )
        suffix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 1.0\n- Initial package\n"
            % today
        )

        # modify the changelogs
        self.before_rpm.section_changelog = suffix
        self.after_rpm.section_changelog = after_prefix + suffix

        self.inspection = "changelog"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# 3) Add unprofessional language to the after build %changelog and make
#    sure that is reported as BAD.
class UnprofessinalChangeLogEntryCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create a change with a bad word
        today = datetime.date.today().strftime("%a %b %d %Y")
        after_prefix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7-1\n"
            "- Upgrade to the latest and reallybadword greatest\n\n" % today
        )
        suffix = (
            "* %s Packie McPackerson <packie@mcpackerson.io> - 1.0\n- Initial package\n"
            % today
        )

        # modify the changelog
        self.before_rpm.section_changelog = after_prefix + suffix
        self.after_rpm.section_changelog = after_prefix + suffix

        self.inspection = "changelog"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"
