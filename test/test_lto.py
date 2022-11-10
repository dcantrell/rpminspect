#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]

# Source code used for the -flto tests
lto_src = open(datadir + "/mathlib.c").read()

# NOTE: The add_simple_compilation() calls to rpmfluff use '-o a.out' in
# the compileFlags due to a limitation in rpmfluff.  It always tries to
# copy a compiled file by the name of 'a.out' so if you are invoking gcc
# to compile a .o, you need to tell gcc to name the output a.out otherwise
# rpmfluff will fail.  This does not impact ELF executables which is the
# more common use.


# No LTO symbols in .o files (OK)
class NoLTOSymbolsRelocRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "OK"


class NoLTOSymbolsRelocKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "OK"


class NoLTOSymbolsRelocCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "OK"


class NoLTOSymbolsRelocCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "OK"


# No LTO symbols in .a files (OK)
class NoLTOSymbolsStaticLibRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "OK"


class NoLTOSymbolsStaticLibKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "OK"


class NoLTOSymbolsStaticLibCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "OK"


class NoLTOSymbolsStaticLibCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "OK"


# LTO symbols present in .o files (BAD)
class LTOSymbolsRelocRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsRelocKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsRelocCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsRelocCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# LTO symbols present in .a files (BAD)
class LTOSymbolsStaticLibRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsStaticLibKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsStaticLibCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsStaticLibCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"
