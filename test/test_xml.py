#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
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
