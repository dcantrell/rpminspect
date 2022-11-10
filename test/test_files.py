#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import rpmfluff

try:
    from rpmfluff import simple_library_source
except ImportError:
    from rpmfluff.samples import simple_library_source

from baseclass import TestSRPM, TestKoji, TestCompareSRPM, TestCompareKoji


class ValidFilesSectionSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        self.rpm.add_source(rpmfluff.SourceFile(sourceFileName, simple_library_source))
        self.rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "files"
        self.result = "OK"


class ValidFilesSectionKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        self.rpm.add_source(rpmfluff.SourceFile(sourceFileName, simple_library_source))
        self.rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "files"
        self.result = "OK"


class ValidFilesSectionCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        self.after_rpm.add_source(
            rpmfluff.SourceFile(sourceFileName, simple_library_source)
        )
        self.after_rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.after_rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "files"
        self.result = "OK"


class ValidFilesSectionCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        self.after_rpm.add_source(
            rpmfluff.SourceFile(sourceFileName, simple_library_source)
        )
        self.after_rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.after_rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "files"
        self.result = "OK"


class InvalidFilesSectionSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_library()

        self.inspection = "files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidFilesSectionKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_library()

        self.inspection = "files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidFilesSectionCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_simple_library()

        self.inspection = "files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidFilesSectionCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_simple_library()

        self.inspection = "files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
