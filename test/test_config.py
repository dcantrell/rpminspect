#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import rpmfluff

from baseclass import TestCompareRPMs, TestCompareKoji

kernel_conf = """#
# Select operating system kernel
#

#freebsd
#openbsd
#netbsd
linux
#irix6
#sunos4
#sunos5
#aix
#winnt
#darwin
#hurd
"""

kernel_conf_with_whitespace_changes = """#
# Select operating system kernel
#

#freebsd
#openbsd
#netbsd
linux                    
#irix6                   
#sunos4
#sunos5
#aix
#winnt
#darwin
#hurd
"""  # noqa: W291

kernel_conf_with_content_changes = """#
# Select operating system kernel
#

#freebsd
#openbsd
#netbsd
vmlinuz
#irix6
#sunos4
#sunos5
#aix
#winnt
#darwin
#hurd
"""


# %config becomes not a %config file in non-rebase comparison (VERIFY)
class ConfigBecomesNonConfigCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ConfigBecomesNonConfigCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# %config becomes not a %config file in rebase comparison (INFO)
class ConfigBecomesNonConfigRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ConfigBecomesNonConfigRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# not a %config becomes a %config file in non-rebase comparison (VERIFY)
class NonConfigBecomesConfigCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class NonConfigBecomesConfigCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# not a %config becomes a %config file in rebase comparison (INFO)
class NonConfigBecomesConfigRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class NonConfigBecomesConfigRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=False,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# whitespace %config differences (OK)
class ConfigChangesWhitespaceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_whitespace_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "OK"


class ConfigChangesWhitespaceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_whitespace_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "OK"


class ConfigChangesWhitespaceRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_whitespace_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "OK"


class ConfigChangesWhitespaceRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_whitespace_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "OK"


# %config content differences (VERIFY)
class ConfigChangesContentCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_content_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ConfigChangesContentCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_content_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ConfigChangesContentRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_content_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ConfigChangesContentRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf_with_content_changes),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# %config to/from file/symlink before and after (VERIFY)
class ConfigChangeFromFileToSymlinkCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ConfigChangeFromFileToSymlinkCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ConfigChangeFromSymlinkToFileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ConfigChangeFromSymlinkToFileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# %config to/from file/symlink before and after rebase (INFO)
class ConfigChangeFromFileToSymlinkRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ConfigChangeFromFileToSymlinkRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ConfigChangeFromSymlinkToFileRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ConfigChangeFromSymlinkToFileRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/kernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# different symlink destinations before and after (VERIFY)
class ConfigSymlinkChangedValueCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/realkernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/realkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ConfigSymlinkChangedValueCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/realkernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/realkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# different symlink destinations before and after rebase (INFO)
class ConfigSymlinkChangedValueRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/actualkernel.conf",
            isConfig=True,
        )

        self.after_rpm.add_installed_file(
            "/etc/realkernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/realkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ConfigSymlinkChangedValueRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/etc/actualkernel.conf",
            rpmfluff.SourceFile("actualkernel.conf", kernel_conf),
            isConfig=True,
        )
        self.before_rpm.add_installed_symlink(
            "/etc/kernel.conf", "/etc/actualkernel.conf", isConfig=True
        )

        self.after_rpm.add_installed_file(
            "/etc/realkernel.conf",
            rpmfluff.SourceFile("kernel.conf", kernel_conf),
            isConfig=True,
        )
        self.after_rpm.add_installed_symlink(
            "/etc/kernel.conf",
            "/etc/realkernel.conf",
            isConfig=True,
        )

        self.inspection = "config"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
