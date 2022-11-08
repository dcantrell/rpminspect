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

import unittest

from baseclass import TestSRPM, TestRPMs, TestKoji


# Empty License tag fails on SRPM (BAD)
class EmptyLicenseTagSRPM(TestSRPM):
    @unittest.skip(
        "rpmbuild actually prevents this, but leave the test in case we need it in the future"
    )
    def setUp(self):
        super().setUp()
        self.rpm.license = ""
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Empty License tag fails on RPMs (BAD)
class EmptyLicenseTagRPMs(TestRPMs):
    @unittest.skip(
        "rpmbuild actually prevents this, but leave the test in case we need it in the future"
    )
    def setUp(self):
        super().setUp()
        self.rpm.license = ""
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Empty License tag fails on Koji build (BAD)
class EmptyLicenseTagKoji(TestKoji):
    @unittest.skip(
        "rpmbuild actually prevents this, but leave the test in case we need it in the future"
    )
    def setUp(self):
        super().setUp()
        self.rpm.license = ""
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Forbidden License tag fails on SRPM (BAD)
class ForbiddenLicenseTagSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("APSL-1.2")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Forbidden License tag fails on RPMs (BAD)
class ForbiddenLicenseTagRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("APSL-1.2")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Forbidden License tag fails on Koji build (BAD)
class ForbiddenLicenseTagKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("APSL-1.2")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# License tag with unprofessional language fails on SRPM (BAD)
class BadWordLicenseTagSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# License tag with unprofessional language fails on RPMs (BAD)
class BadWordLicenseTagRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# License tag with unprofessional language fails on Koji build (BAD)
class BadWordLicenseTagKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Valid License tag passes on SRPM (OK)
class ValidLicenseTagSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv3+")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag passes on RPMs (OK)
class ValidLicenseTagRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv3+")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag passes on Koji build (OK)
class ValidLicenseTagKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv3+")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces passes on SRPM (OK)
class ValidLicenseTagWithSpacesSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("ASL 2.0")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces passes on RPMs (OK)
class ValidLicenseTagWithSpacesRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("ASL 2.0")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces passes on Koji build (OK)
class ValidLicenseTagWithSpacesKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("ASL 2.0")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces passes on SRPM (OK)
class ValidLicenseTagWithBooleanSpacesSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv3+ and ASL 2.0")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces passes on RPMs (OK)
class ValidLicenseTagWithBooleanSpacesRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("ASL 2.0 and GPLv3+")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces passes on Koji build (OK)
class ValidLicenseTagWithBooleanSpacesKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("GPLv3+ or ASL 2.0")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces and parens passes on SRPM (OK)
class ValidLicenseTagWithBooleanSpacesParensSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("Artistic 2.0 and (GPL+ or Artistic)")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces and parens passes on RPMs (OK)
class ValidLicenseTagWithBooleanSpacesParensRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("Artistic 2.0 and (GPL+ or Artistic)")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces and parens passes on Koji build (OK)
class ValidLicenseTagWithBooleanSpacesParensKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("Artistic 2.0 and (GPL+ or Artistic)")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces and parens passes on SRPM (OK)
class AnotherValidLicenseTagWithBooleanSpacesParensSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("MIT and (BSD or ASL 2.0)")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces and parens passes on RPMs (OK)
class AnotherValidLicenseTagWithBooleanSpacesParensRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("MIT and (BSD or ASL 2.0)")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License tag with spaces and parens passes on Koji build (OK)
class AnotherValidLicenseTagWithBooleanSpacesParensKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("MIT and (BSD or ASL 2.0)")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid glibc License tag on Koji build (OK)
class ValidGlibcLicenseTagKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense(
            "LGPLv2+ and LGPLv2+ with exceptions and GPLv2+ and GPLv2+ with exceptions "
            "and BSD and Inner-Net and ISC and Public Domain and GFDL"
        )
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid perl-HTTP-Message License tag on Koji build (OK)
class ValidPerlHTTPMessageLicenseTagKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("(GPL+ or Artistic) and CC0")
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Valid License string without any official abbreviations (OK)
class ValidLoremIpsonLicenseTagSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua"  # noqa: E501
        )
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ValidLoremIpsonLicenseTagRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua"  # noqa: E501
        )
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ValidLoremIpsonLicenseTagKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua"  # noqa: E501
        )
        self.inspection = "license"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Invalid use of fedora_name string when an abbreviation is available (BAD)
class InvalidUseOfLicenseStringSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("Apache Software License 2.0")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class InvalidUseOfLicenseStringRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("Apache Software License 2.0")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class InvalidUseOfLicenseStringKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.addLicense("Apache Software License 2.0")
        self.inspection = "license"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"
