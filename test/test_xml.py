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

import rpmfluff

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

valid_xml = """<?xml version='1.0'?>
<!DOCTYPE greeting [
    <!ELEMENT greeting (#PCDATA)>
]>
<greeting>Hello, world!</greeting>
"""

invalid_xml = """<?xml version='1.0'?>
<!DOCTYPE greeting [
    <!ELEMENT greeting EMPTY>
]>
<greeting>Hello world</greeting>
<nonClosingElement variable="value">
"""


# XML file is well formed in RPM (OK)
class XMLWellFormedRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_installed_file(
            "/usr/share/data/valid.xml", rpmfluff.SourceFile("valid.xml", valid_xml)
        )
        self.inspection = "xml"
        self.result = "OK"


# XML file is well formed in Koji build (OK)
class XMLWellFormedKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_installed_file(
            "/usr/share/data/valid.xml", rpmfluff.SourceFile("valid.xml", valid_xml)
        )
        self.inspection = "xml"
        self.result = "OK"


# XML file is well formed in compare RPMs (OK)
class XMLWellFormedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid.xml", rpmfluff.SourceFile("valid.xml", valid_xml)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid.xml", rpmfluff.SourceFile("valid.xml", valid_xml)
        )
        self.inspection = "xml"
        self.result = "OK"


# XML file is well formed in compare Koji builds (OK)
class XMLWellFormedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/valid.xml", rpmfluff.SourceFile("valid.xml", valid_xml)
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/valid.xml", rpmfluff.SourceFile("valid.xml", valid_xml)
        )
        self.inspection = "xml"
        self.result = "OK"


# XML file is malformed in RPM (VERIFY)
class XMLMalformedRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_installed_file(
            "/usr/share/data/invalid.xml",
            rpmfluff.SourceFile("invalid.xml", invalid_xml),
        )
        self.inspection = "xml"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# XML file is malformed in Koji build (VERIFY)
class XMLMalformedKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_installed_file(
            "/usr/share/data/invalid.xml",
            rpmfluff.SourceFile("invalid.xml", invalid_xml),
        )
        self.inspection = "xml"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# XML file is malformed in compare RPMs (VERIFY)
class XMLMalformedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid.xml",
            rpmfluff.SourceFile("invalid.xml", invalid_xml),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid.xml",
            rpmfluff.SourceFile("invalid.xml", invalid_xml),
        )
        self.inspection = "xml"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# XML file is malformed in compare Koji builds (VERIFY)
class XMLMalformedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/share/data/invalid.xml",
            rpmfluff.SourceFile("invalid.xml", invalid_xml),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/data/invalid.xml",
            rpmfluff.SourceFile("invalid.xml", invalid_xml),
        )
        self.inspection = "xml"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
