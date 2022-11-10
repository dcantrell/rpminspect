#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import glob
import os
import shutil
import subprocess
import unittest
import rpmfluff
from baseclass import TestCompareRPMs, TestCompareKoji

# We try to use the running kernel, but failing that we look at what version we have
# installed with build files and go with that.  The kernel version is necessary for
# kmod tests below.
have_kernel_devel = False
kernel_build_dir = None

# Try the running kernel
kver = os.uname()[2]
kmakefile = os.path.join("/lib/modules", kver, "build/Makefile")
if not have_kernel_devel and os.path.isfile(kmakefile):
    have_kernel_devel = True
    kernel_build_dir = os.path.dirname(kmakefile)

# Try the installed RPMs
if not have_kernel_devel:
    args = [
        "rpm",
        "-q",
        "--queryformat='%{version}-%{release}.%{arch}\n'",
        "kernel-devel",
    ]
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (out, err) = proc.communicate()
    if proc.returncode == 0:
        for kver in out.splitlines():
            kmakefile = os.path.join("/lib/modules", str(kver), "build/Makefile")
            if os.path.isfile(kmakefile):
                have_kernel_devel = True
                kernel_build_dir = os.path.dirname(kmakefile)
                break

# Lastly, try just finding the first file in /lib/modules
if not have_kernel_devel:
    makefiles = glob.glob("/lib/modules/*/build/Makefile")
    if len(makefiles) > 0:
        kver = makefiles[0].split("/")[3]
        have_kernel_devel = True
        kernel_build_dir = os.path.join("/lib", "modules", kver, "build")


# Support functions to build the kernel modules we need
def build_module(rpminspect, build_ext=None, extra_cflags=None):
    build = os.path.dirname(rpminspect)
    srcdir = os.path.realpath(
        os.path.join(os.environ["RPMINSPECT_TEST_DATA_PATH"], "derp-kmod")
    )

    if build_ext is None:
        moddir = os.path.join(build, "derp-kmod")
    else:
        moddir = os.path.join(build, "derp-kmod" + build_ext)

    kmod = os.path.join(moddir, "derp.ko")

    if os.path.isdir(moddir) and os.path.isfile(kmod):
        f = open(kmod, "rb")
        kmodbuf = f.read()
        f.close()
        return kmodbuf

    shutil.rmtree(moddir, ignore_errors=True)
    shutil.copytree(srcdir, moddir)

    cwd = os.getcwd()
    os.chdir(moddir)

    if extra_cflags is None:
        cmd = "make -s KERNEL_BUILD_DIR=" + kernel_build_dir
    else:
        cmd = (
            "make -s KERNEL_BUILD_DIR="
            + kernel_build_dir
            + " EXTRA_CFLAGS="
            + extra_cflags
        )

    os.system(cmd)
    os.chdir(cwd)

    # read in the module and return it
    f = open(kmod, "rb")
    kmodbuf = f.read()
    f.close()
    return kmodbuf


def get_derp_kmod(rpminspect):
    return build_module(rpminspect)


def get_derp_kmod_params(rpminspect):
    return build_module(
        rpminspect, build_ext="-params", extra_cflags="-D_USE_MODULE_PARAMETERS"
    )


def get_derp_kmod_depends(rpminspect):
    return build_module(
        rpminspect, build_ext="-depends", extra_cflags="-D_USE_MODULE_DEPENDS"
    )


def get_derp_kmod_aliases(rpminspect):
    return build_module(
        rpminspect, build_ext="-aliases", extra_cflags="-D_USE_MODULE_ALIASES"
    )


############################
# kernel module parameters #
############################

# Verify kmod params are good between before and after RPMs
class GoodKmodParamsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_params(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_params(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Report loss of kmod params between before and after RPMs
class LostKmodParmsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_params(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Verify kmod params are good between before and after Koji builds
class GoodKmodParamsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_params(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_params(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Report loss of kmod params between before and after Koji builds
class LostKmodParamsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_params(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


#########################
# kernel module depends #
#########################

# Verify kmod depends are good between before and after RPMs
class GoodKmodDependsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Report loss of kmod depends between before and after RPMs
class LostKmodDependsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Verify kmod depends are good between before and after Koji builds
class GoodKmodDependsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Report loss of kmod depends between before and after Koji builds
class LostKmodDependsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


#########################
# kernel module aliases #
#########################

# Verify kmod aliases are good between before and after RPMs
class GoodKmodAliasesRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_aliases(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_aliases(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Report loss of kmod aliases between before and after RPMs
class LostKmodAliasesRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_aliases(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Verify kmod aliases are good between before and after Koji builds
class GoodKmodAliasesKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_aliases(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_aliases(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Report loss of kmod aliases between before and after Koji builds
class LostKmodAliasesKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_aliases(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Kernel module changing paths
class KmodChangingPathCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derpy-stuff/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class KmodChangingPathCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/modules/" + kver + "/extra/drivers/derpy-stuff/derp.ko",
            rpmfluff.SourceFile("derp.ko", get_derp_kmod_depends(self.rpminspect)),
        )

        self.inspection = "kmod"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"
