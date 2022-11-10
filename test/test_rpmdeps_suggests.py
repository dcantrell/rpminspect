#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import rpm
import unittest

from rpmfluff.sourcefile import SourceFile
from rpmfluff.utils import CC

from baseclass import TestKoji, TestRPMs, TestSRPM
from baseclass import TestCompareKoji, TestCompareRPMs, TestCompareSRPM
from baseclass import BEFORE_NAME, BEFORE_VER, BEFORE_REL
from baseclass import AFTER_NAME, AFTER_VER, AFTER_REL

# determine if we have /usr/lib/rpm/elfdeps
# (missing on Alpine Linux as of 14-Jan-2022)
elfdeps = "/usr/lib/rpm/elfdeps"
have_elfdeps = False

if os.path.isfile(elfdeps) and os.access(elfdeps, os.X_OK):
    have_elfdeps = True

# need to know if we are on ALT Linux or not because rpmbuild
# on that platform prohibits unexpanded macros, so we can skip
# those test cases
on_alt_linux = False

if os.path.isfile("/etc/altlinux-release") or os.path.isfile("/etc/alt-release"):
    on_alt_linux = True

# determine if the Suggests tag type is present in librpm
# version must be >= 4.12.0
rpmver = list(map(lambda x: int(x), rpm.__version__.strip().split("-")[0].split(".")))
rpm_major = rpmver[0]
rpm_minor = rpmver[1]
rpm_update = rpmver[2]

if (rpm_major < 4 or (rpm_major == 4 and rpm_minor < 12)) or on_alt_linux:
    have_suggests = False
else:
    have_suggests = True

before_suggests = "important-package >= 2.0.2-47"
after_suggests = "important-package >= 4.7.0-1"
unexpanded_suggests = "important-package >= 4.7.1-1%{_macro}"

# library source code for use in these package builds
library_source = """#include <stdio.h>

void greet(const char *message)
{
    printf ("%s\\n", message);
}
"""

# source to use as /usr/bin example program in package builds
hello_lib_world = """#include <stdio.h>

void greet(const char *message);

int
main (int argc, char **argv)
{
    greet ("Hello world\\n");

    return 0;
}
"""


# Suggests dependency is correct (OK) - control case
class SuggestsCorrectRPMs(TestRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "OK"


class SuggestsCorrectKoji(TestKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "OK"


# Retaining Suggests dependency in rebase comparison (INFO)
class RetainingSuggestsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_suggests(after_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RetainingSuggestsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_suggests(after_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Retaining Suggests dependency in maint comparison (OK)
class RetainingSuggestsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(after_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "OK"


class RetainingSuggestsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(after_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "OK"


# Gaining a new Suggests in a rebase comparison (INFO)
class GainingSuggestsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainingSuggestsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Gaining a new Suggests in a maint comparison
class GainingSuggestsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainingSuggestsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Changing a Suggests in a rebase comparison (INFO)
class ChangingSuggestsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_suggests(before_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingSuggestsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_suggests(before_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Changing a Suggests in a maint comparison
class ChangingSuggestsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(before_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingSuggestsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(before_suggests)
        self.after_rpm.add_suggests(after_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Changing a Suggests in a maint comparison due to NVR (INFO)
class ChangingSuggestsExpectedCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # ok result because this is a source RPM
        self.inspection = "rpmdeps"
        self.result = "OK"


class ChangingSuggestsExpectedCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingSuggestsExpectedCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Losing a Suggests in a rebase comparison (INFO)
class LosingSuggestsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_suggests(before_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class LosingSuggestsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_suggests(before_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Losing a Suggests in a maint comparison
class LosingSuggestsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(before_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class LosingSuggestsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(before_suggests)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Missing Epoch prefix on maint compare (BAD for Koji compares, OK
# otherwise)
class MissingEpochSuggestsSRPM(TestSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.rpm.epoch = 47

        # add a subpackage
        sub = self.rpm.add_subpackage("devel")
        sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # the result is OK here because the Epoch check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingEpochSuggestsRPMs(TestRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.rpm.epoch = 47

        # add a subpackage
        sub = self.rpm.add_subpackage("devel")
        sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # the result is OK here because the rpmdeps inspection when
        # run against just a pair of RPMs can't check for missing
        # Epoch values in manual dependencies, so the inspection
        # doesn't fail
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingEpochSuggestsKoji(TestKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.rpm.epoch = 47

        # add a subpackage
        sub = self.rpm.add_subpackage("devel")
        sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # result is OK for Suggests weak dep; Epoch check is for Provides/Requires pairs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingEpochSuggestsCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        before_sub.add_suggests("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # this is 'OK' because the missing Epoch check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingEpochSuggestsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        before_sub.add_suggests("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # this is 'OK' because the Compare RPMs tests don't work with the
        # entire collection of RPMs, it does it iteratively
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingEpochSuggestsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        before_sub.add_suggests("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # result is OK for Suggests weak dep; Epoch check is for Provides/Requires pairs
        self.inspection = "rpmdeps"
        self.result = "OK"


# Missing Epoch prefix on rebase compare (INFO)
class MissingEpochSuggestsRebaseCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        before_sub.add_suggests("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # ok result because this is a source RPM
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingEpochSuggestsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        before_sub.add_suggests("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class MissingEpochSuggestsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp(rebase=True)

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Suggests
        before_sub.add_suggests("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_suggests("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Unexpanded macro in Suggests (BAD)
class UnexpandedMacroSuggestsSRPM(TestSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.rpm.add_suggests(unexpanded_suggests)

        # this is OK because it's the SRPM
        self.inspection = "rpmdeps"
        self.result = "OK"


class UnexpandedMacroSuggestsRPMs(TestRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.rpm.add_suggests(unexpanded_suggests)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroSuggestsKoji(TestKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.rpm.add_suggests(unexpanded_suggests)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroSuggestsCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(unexpanded_suggests)
        self.after_rpm.add_suggests(unexpanded_suggests)

        # this is OK because it's the SRPM
        self.inspection = "rpmdeps"
        self.result = "OK"


class UnexpandedMacroSuggestsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(unexpanded_suggests)
        self.after_rpm.add_suggests(unexpanded_suggests)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroSuggestsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_suggests(unexpanded_suggests)
        self.after_rpm.add_suggests(unexpanded_suggests)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Missing explicit Suggests
class MissingExplicitSuggestsSRPM(TestSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"
        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps on SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingExplicitSuggestsRPMs(TestRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"
        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps when just
        # looking at a single RPM
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingExplicitSuggestsKoji(TestKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"
        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class MissingExplicitSuggestsCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps on SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingExplicitSuggestsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps on single
        # RPM comparisons
        self.inspection = "rpmdeps"
        self.result = "OK"


class MissingExplicitSuggestsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Multiple providers of the same thing (VERIFY)
class MultipleProvidersSRPM(TestSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.rpm.add_subpackage("libs")
        self.rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # add an explicit suggests on the libs package
        self.rpm.add_suggests("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MultipleProvidersRPMs(TestRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.rpm.add_subpackage("libs")
        self.rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # add an explicit suggests on the libs package
        self.rpm.add_suggests("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for single RPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MultipleProvidersKoji(TestKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.rpm.add_subpackage("libs")
        self.rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # add an explicit suggests on the libs package
        self.rpm.add_suggests("vaporware-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "OK"


class MultipleProvidersCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.before_rpm.add_subpackage("libs")
        self.before_rpm.add_subpackage("morelibs")
        self.after_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # add an explicit suggests on the libs package
        self.before_rpm.add_suggests("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_suggests("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MultipleProvidersCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.before_rpm.add_subpackage("libs")
        self.before_rpm.add_subpackage("morelibs")
        self.after_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # add an explicit suggests on the libs package
        self.before_rpm.add_suggests("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_suggests("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op when comparing single RPMs
        self.inspection = "rpmdeps"
        self.result = "OK"


class MultipleProvidersCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_suggests, "librpm too old to support Suggests")
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.before_rpm.add_subpackage("libs")
        self.before_rpm.add_subpackage("morelibs")
        self.after_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # add an explicit suggests on the libs package
        self.before_rpm.add_suggests("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_suggests("vaporware-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "OK"
