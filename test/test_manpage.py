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

try:
    from rpmfluff import sample_man_page
except ImportError:
    from rpmfluff.samples import sample_man_page

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji


# Man page in the correct section subdirectory in RPM (OK)
class ManPageCorrectSectionRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # force a gzip here since we have disabled brp scripts
        manpage = "usr/share/man/man1/foo.1"
        self.rpm.add_manpage(installPath=manpage)
        self.rpm.section_install += (
            "gzip -9 $RPM_BUILD_ROOT/%s\n" % self.rpm.escape_path(manpage)
        )

        self.inspection = "manpage"
        self.result = "OK"


# Man page in the correct section subdirectory in Koji build (OK)
class ManPageCorrectSectionKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # force a gzip here since we have disabled brp scripts
        manpage = "usr/share/man/man1/foo.1"
        self.rpm.add_manpage(installPath=manpage)
        self.rpm.section_install += (
            "gzip -9 $RPM_BUILD_ROOT/%s\n" % self.rpm.escape_path(manpage)
        )

        self.inspection = "manpage"
        self.result = "OK"


# Man page in the correct section subdirectory in compare RPMs (OK)
class ManPageCorrectSectionCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # force a gzip here since we have disabled brp scripts
        manpage = "usr/share/man/man1/foo.1"
        self.before_rpm.add_manpage(installPath=manpage)
        self.before_rpm.section_install += (
            "gzip -9 $RPM_BUILD_ROOT/%s\n" % self.before_rpm.escape_path(manpage)
        )
        self.after_rpm.add_manpage()
        self.after_rpm.section_install += (
            "gzip -9 $RPM_BUILD_ROOT/%s\n" % self.before_rpm.escape_path(manpage)
        )

        self.inspection = "manpage"
        self.result = "OK"


# Man page in the correct section subdirectory in compare Koji (OK)
class ManPageCorrectSectionCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # force a gzip here since we have disabled brp scripts
        manpage = "usr/share/man/man1/foo.1"
        self.before_rpm.add_manpage(installPath=manpage)
        self.before_rpm.section_install += (
            "gzip -9 $RPM_BUILD_ROOT/%s\n" % self.before_rpm.escape_path(manpage)
        )
        self.after_rpm.add_manpage()
        self.after_rpm.section_install += (
            "gzip -9 $RPM_BUILD_ROOT/%s\n" % self.before_rpm.escape_path(manpage)
        )

        self.inspection = "manpage"
        self.result = "OK"


# Man page not gzipped in RPM (VERIFY)
class ManPageNotGzippedRPM(TestRPMs):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        self.rpm.add_installed_file(
            "usr/local/share/man/man1/foo.1",
            rpmfluff.SourceFile("foo.1", sample_man_page),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Man page not gzipped in Koji build (VERIFY)
class ManPageNotGzippedKoji(TestKoji):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        self.rpm.add_installed_file(
            "usr/local/share/man/man1/foo.1",
            rpmfluff.SourceFile("foo.1", sample_man_page),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Man page not gzipped in compare RPMs (VERIFY)
class ManPageNotGzippedCompareRPMs(TestCompareRPMs):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        self.before_rpm.add_installed_file(
            "usr/local/share/man/man1/foo.1",
            rpmfluff.SourceFile("foo.1", sample_man_page),
        )
        self.after_rpm.add_installed_file(
            "usr/local/share/man/man1/foo.1",
            rpmfluff.SourceFile("foo.1", sample_man_page),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Man page not gzipped in compare Koji (VERIFY)
class ManPageNotGzippedCompareKoji(TestCompareKoji):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        self.before_rpm.add_installed_file(
            "usr/local/share/man/man1/foo.1",
            rpmfluff.SourceFile("foo.1", sample_man_page),
        )
        self.after_rpm.add_installed_file(
            "usr/local/share/man/man1/foo.1",
            rpmfluff.SourceFile("foo.1", sample_man_page),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Man page in wrong section subdirectory in RPM (VERIFY)
class ManPageWrongSectionRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_manpage(
            sourceFileName="foo.8", installPath="usr/share/man/man1/foo.8.gz"
        )
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Man page in wrong section subdirectory in Koji build (VERIFY)
class ManPageWrongSectionKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_manpage(
            sourceFileName="foo.8", installPath="usr/share/man/man1/foo.8.gz"
        )
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Man page in wrong section subdirectory in compare RPMs (VERIFY)
class ManPageWrongSectionCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_manpage(
            sourceFileName="foo.8", installPath="usr/share/man/man1/foo.8.gz"
        )
        self.after_rpm.add_manpage(
            sourceFileName="foo.8", installPath="usr/share/man/man1/foo.8.gz"
        )
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Man page in wrong section subdirectory in compare Koji (VERIFY)
class ManPageWrongSectionCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_manpage(
            sourceFileName="foo.8", installPath="usr/share/man/man1/foo.8.gz"
        )
        self.after_rpm.add_manpage(
            sourceFileName="foo.8", installPath="usr/share/man/man1/foo.8.gz"
        )
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Invalid man page syntax in RPM (VERIFY)
class InvalidManPageRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.rpm.add_installed_file(
            "/usr/share/man/man1/foo.1.gz",
            rpmfluff.GeneratedSourceFile("foo.1", rpmfluff.make_png()),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Invalid man page syntax in Koji build (VERIFY)
class InvalidManPageKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.rpm.add_installed_file(
            "/usr/share/man/man1/foo.1.gz",
            rpmfluff.GeneratedSourceFile("foo.1", rpmfluff.make_png()),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Invalid man page syntax in compare RPMs (VERIFY)
class InvalidManPageCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.before_rpm.add_installed_file(
            "/usr/share/man/man1/foo.1.gz",
            rpmfluff.GeneratedSourceFile("foo.1", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/man/man1/foo.1.gz",
            rpmfluff.GeneratedSourceFile("foo.1", rpmfluff.make_png()),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Invalid man page syntax in compare Koji (VERIFY)
class InvalidManPageCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.before_rpm.add_installed_file(
            "/usr/share/man/man1/foo.1.gz",
            rpmfluff.GeneratedSourceFile("foo.1", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/man/man1/foo.1.gz",
            rpmfluff.GeneratedSourceFile("foo.1", rpmfluff.make_png()),
        )

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Empty but compressed man page RPM build (VERIFY)
class EmptyManPageRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add an empty but gzipped man page
        self.rpm.section_build += "touch foo.1\n"
        self.rpm.section_build += "gzip -9 foo.1\n"
        self.rpm.section_install += (
            "install -D -m 0644 foo.1.gz %{buildroot}%{_mandir}/man1/foo.1.gz\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%{_mandir}/man1/foo.1.gz\n"

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Empty but compressed man page Koji build (VERIFY)
class EmptyManPageKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add an empty but gzipped man page
        self.rpm.section_build += "touch foo.1\n"
        self.rpm.section_build += "gzip -9 foo.1\n"
        self.rpm.section_install += (
            "install -D -m 0644 foo.1.gz %{buildroot}%{_mandir}/man1/foo.1.gz\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%{_mandir}/man1/foo.1.gz\n"

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Empty but compressed man page compare RPM builds (VERIFY)
class EmptyManPageCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add an empty but gzipped man page
        self.after_rpm.section_build += "touch foo.1\n"
        self.after_rpm.section_build += "gzip -9 foo.1\n"
        self.after_rpm.section_install += (
            "install -D -m 0644 foo.1.gz %{buildroot}%{_mandir}/man1/foo.1.gz\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%{_mandir}/man1/foo.1.gz\n"

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Empty but compressed man page compare Koji builds (VERIFY)
class EmptyManPageCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # disable automatic man page compression in rpmbuild
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add an empty but gzipped man page
        self.after_rpm.section_build += "touch foo.1\n"
        self.after_rpm.section_build += "gzip -9 foo.1\n"
        self.after_rpm.section_install += (
            "install -D -m 0644 foo.1.gz %{buildroot}%{_mandir}/man1/foo.1.gz\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%{_mandir}/man1/foo.1.gz\n"

        # the test
        self.inspection = "manpage"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
