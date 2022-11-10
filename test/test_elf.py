#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import subprocess
import unittest

import rpmfluff

try:
    from rpmfluff import simple_library_source
except ImportError:
    from rpmfluff.samples import simple_library_source

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]

# Source code used for the -D_FORTIFY_SOURCE tests
fortify_src = open(datadir + "/fortify.c").read()

# Simple source file for library tests
test_library_source = """#include <math.h>

double exponent(double x, double y)
{
    return pow(x, y);
}
"""

# Figure out if the system is 32-bit capable or not
have_gcc_multilib = False
args = ["gcc", "-print-multi-lib"]
proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(out, err) = proc.communicate()

if (
    proc.returncode == 0
    and str(out).find("@m32") != -1
    and os.path.isfile("/usr/include/gnu/stubs-32.h")
    and not os.path.isfile("/etc/altlinux-release")
):
    have_gcc_multilib = True


# Simple way to figure out if we are musl or glibc
have_musl_libc = False
args = ["patchelf", "--print-interpreter", "/sbin/init"]
proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(out, err) = proc.communicate()

if proc.returncode == 0 and str(out).find("ld-musl") != -1:
    have_musl_libc = True


# Program built with noexecstack
class WithoutExecStackRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.result = "OK"


class WithoutExecStackKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.result = "OK"


class WithoutExecStackCompareRPM(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.result = "OK"


class WithoutExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.result = "OK"


# Program built with execstack
class WithExecStackRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/bin/execstack", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPWithExecStackRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMWithExecStackRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYWithExecStackRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILWithExecStackRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class WithExecStackKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/bin/execstack", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPWithExecStackKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMWithExecStackKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYWithExecStackKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILWithExecStackKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class WithExecStackCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/bin/execstack", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/bin/execstack", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPWithExecStackCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMWithExecStackCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYWithExecStackCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILWithExecStackCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class WithExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/bin/execstack", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/bin/execstack", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPWithExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMWithExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYWithExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILWithExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,execstack"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,execstack"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


# Program lost full RELRO
class LostFullRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,norelro")
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPLostFullRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMLostFullRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYLostFullRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILLostFullRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class LostFullRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,norelro")
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPLostFullRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMLostFullRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYLostFullRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILLostFullRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,norelro"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


# Program lost full RELRO but retained partial RELRO
class FulltoPartialRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,lazy")
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPFulltoPartialRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMFulltoPartialRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYFulltoPartialRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILFulltoPartialRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class FulltoPartialRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,lazy")
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPFulltoPartialRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/skip", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMFulltoPartialRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/inform", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYFulltoPartialRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/verify", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILFulltoPartialRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,relro,-z,now"
        )
        self.after_rpm.add_simple_compilation(
            installPath="usr/sbin/fail", compileFlags="-Wl,-z,relro,-z,lazy"
        )
        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


# Program lost -fPIC in after (BAD, WAIVABLE_BY_SECURITY)
class LostPICCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libsimple.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += (
            "gcc -m32 -shared -fno-PIC -o simple.o simple.c\n"
        )
        self.after_rpm.section_build += "chmod 0644 simple.o\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPLostPICCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libskip.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += (
            "gcc -m32 -shared -fno-PIC -o simple.o simple.c\n"
        )
        self.after_rpm.section_build += "chmod 0644 simple.o\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMLostPICCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libinform.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += (
            "gcc -m32 -shared -fno-PIC -o simple.o simple.c\n"
        )
        self.after_rpm.section_build += "chmod 0644 simple.o\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYLostPICCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libverify.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += (
            "gcc -m32 -shared -fno-PIC -o simple.o simple.c\n"
        )
        self.after_rpm.section_build += "chmod 0644 simple.o\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILLostPICCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libfail.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += (
            "gcc -m32 -shared -fno-PIC -o simple.o simple.c\n"
        )
        self.after_rpm.section_build += "chmod 0644 simple.o\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class LostPICCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libsimple.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += (
            "gcc -m32 -shared -fno-PIC -o simple.o simple.c\n"
        )
        self.after_rpm.section_build += "chmod 0644 simple.o\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


# Program has or gained TEXTREL relocations (32-bit arches only)
class HasTEXTRELRPMs(TestRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libfoo.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.rpm.add_source(rpmfluff.SourceFile("simple.c", simple_library_source))
        self.rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPHasTEXTRELRPMs(TestRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libskip.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.rpm.add_source(rpmfluff.SourceFile("simple.c", simple_library_source))
        self.rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMHasTEXTRELRPMs(TestRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libinform.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.rpm.add_source(rpmfluff.SourceFile("simple.c", simple_library_source))
        self.rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYHasTEXTRELRPMs(TestRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libverify.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.rpm.add_source(rpmfluff.SourceFile("simple.c", simple_library_source))
        self.rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILHasTEXTRELRPMs(TestRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libfail.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.rpm.add_source(rpmfluff.SourceFile("simple.c", simple_library_source))
        self.rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class HasTEXTRELCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libfoo.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPHasTEXTRELCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libskip.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMHasTEXTRELCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libinform.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYHasTEXTRELCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libverify.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILHasTEXTRELCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libfail.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class HasTEXTRELCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libfoo.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"


class SecuritySKIPHasTEXTRELCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libskip.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.result = "OK"


class SecurityINFORMHasTEXTRELCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libinform.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "INFO"


class SecurityVERIFYHasTEXTRELCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libverify.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class SecurityFAILHasTEXTRELCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        super().setUp()

        installPath = "usr/lib/libfail.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.waiver_auth = "Security"
        self.result = "BAD"
