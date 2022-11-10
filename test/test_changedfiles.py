#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import rpmfluff

from baseclass import TestCompareRPMs, TestCompareKoji

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"] + "/compression/"


# gzip file does not change between builds despite having different
# compression ratios
class GzipFileNoChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.gz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test1-high.gz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


class GzipFileNoChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.gz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test1-high.gz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


# gzip file changes between builds and has different compression ratios
class GzipFileChangesRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.gz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test2-low.gz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


class GzipFileChangesKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.gz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test2-low.gz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.gz", rpmfluff.SourceFile("firmware.gz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


# bzip2 file does not change between builds despite having different
# compression ratios
class Bzip2FileNoChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.bz2", "rb") as f:
            before_src = f.read()

        with open(datadir + "test1-high.bz2", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


class Bzip2FileNoChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.bz2", "rb") as f:
            before_src = f.read()

        with open(datadir + "test1-high.bz2", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


# bzip2 file changes between builds and has different compression ratios
class Bzip2FileChangesRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.bz2", "rb") as f:
            before_src = f.read()

        with open(datadir + "test2-low.bz2", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


class Bzip2FileChangesKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.bz2", "rb") as f:
            before_src = f.read()

        with open(datadir + "test2-low.bz2", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.bz2", rpmfluff.SourceFile("firmware.bz2", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


# xz file does not change between builds despite having different
# compression ratios
class XzFileNoChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test2-low.xz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test2-high.xz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


class XzFileNoChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.xz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test1-high.xz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


# xz file changes between builds and has different compression ratios
class XzFileChangesRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.xz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test2-low.xz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"


class XzFileChangesKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # two files with the same data, but compressed differently
        with open(datadir + "test1-low.xz", "rb") as f:
            before_src = f.read()

        with open(datadir + "test2-low.xz", "rb") as f:
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", before_src)
        )
        self.after_rpm.add_installed_file(
            "/usr/etc/firmware.xz", rpmfluff.SourceFile("firmware.xz", after_src)
        )

        self.inspection = "changedfiles"
        self.result = "OK"
