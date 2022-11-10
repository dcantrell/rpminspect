#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import platform
import yaml
import rpmfluff

from baseclass import TestCompareRPMs


class FileSizeGrowsAtThreshold(TestCompareRPMs):
    """Assert when a file grows by exactly the configured threshold, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        # Tests have threshold configuration of 20%
        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 5)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 6)
        )

        self.inspection = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = f"/some/file grew by 20% on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class FileSizeGrowsAboveThreshold(TestCompareRPMs):
    """Assert when a file grows by more than the configured threshold, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        # Tests have threshold configuration of 20%
        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 5)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 10)
        )

        self.inspection = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = f"/some/file grew by 100% on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class FileSizeGrowsBelowThreshold(TestCompareRPMs):
    """Assert when a file grows by less than the configured threshold, an INFO result occurs."""

    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 10)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 11)
        )

        self.inspection = "filesize"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
        self.message = f"/some/file grew by 10% on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class EmptyFileSizeGrows(TestCompareRPMs):
    """Assert when an empty file grows by, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "")
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a")
        )

        self.inspection = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = f"/some/file became a non-empty file on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class FileSizeShrinksAtThreshold(TestCompareRPMs):
    """Assert when a file shrinks by exactly the configured threshold, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 5)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 4)
        )

        self.inspection = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = f"/some/file shrank by 20% on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class FileSizeShrinksAboveThreshold(TestCompareRPMs):
    """Assert when a file shrinks by more than the configured threshold, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 10)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 2)
        )

        self.inspection = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = f"/some/file shrank by 80% on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class FileSizeShrinksBelowThreshold(TestCompareRPMs):
    """Assert when a file shrinks by less than the configured threshold, an INFO result occurs."""

    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 10)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 9)
        )

        self.inspection = "filesize"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
        self.message = f"/some/file shrank by 10% on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class FileSizeShrinksToEmpty(TestCompareRPMs):
    """Assert when a file shrinks to an empty file, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a")
        )
        self.after_rpm.add_installed_file("/some/file", rpmfluff.SourceFile("file", ""))

        self.inspection = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
        self.message = f"/some/file became an empty file on {platform.machine()}"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["filesize"]["size_threshold"] = "20"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()
