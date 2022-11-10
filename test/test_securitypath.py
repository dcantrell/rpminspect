#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import rpmfluff

from baseclass import TestCompareRPMs, TestCompareKoji

contents = """
# Open things up
%wheel   ALL=(ALL) NOPASSWD: ALL
"""


################################################
# New file present or added to a security path #
################################################


class FileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"


class SecurityINFORMFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"


class SecurityINFORMFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


##################################################
# File removed from securith path in after build #
##################################################


class FileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"


class SecurityINFORMFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"


class SecurityINFORMFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"
