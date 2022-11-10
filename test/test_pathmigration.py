#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

from baseclass import TestRPMs, TestCompareRPMs, TestKoji, TestCompareKoji

# These tests rely on the [pathmigration] section in
# data/generic.yaml in the source tree.

########
# /bin #
########


# File in /bin in RPM (VERIFY)
class SlashBinFileRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        # Adds /bin/hello-world
        self.rpm.add_simple_compilation(installPath="bin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /bin in compare RPM (VERIFY)
class SlashBinFileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # Adds /bin/hello-world
        self.before_rpm.add_simple_compilation(installPath="bin/hello-world")
        self.after_rpm.add_simple_compilation(installPath="bin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /bin in Koji build (VERIFY)
class SlashBinFileKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # Adds /bin/hello-world
        self.rpm.add_simple_compilation(installPath="bin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /bin in compare Koji build (VERIFY)
class SlashBinFileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # Adds /bin/hello-world
        self.before_rpm.add_simple_compilation(installPath="bin/hello-world")
        self.after_rpm.add_simple_compilation(installPath="bin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


#########
# /sbin #
#########

# File in /sbin in RPM (VERIFY)
class SlashSbinFileRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        # Adds /sbin/hello-world
        self.rpm.add_simple_compilation(installPath="sbin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /sbin in compare RPM (VERIFY)
class SlashSbinFileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # Adds /sbin/hello-world
        self.before_rpm.add_simple_compilation(installPath="sbin/hello-world")
        self.after_rpm.add_simple_compilation(installPath="sbin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /sbin in Koji build (VERIFY)
class SlashSbinFileKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # Adds /sbin/hello-world
        self.rpm.add_simple_compilation(installPath="sbin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /sbin in compare Koji build (VERIFY)
class SlashSbinFileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # Adds /sbin/hello-world
        self.before_rpm.add_simple_compilation(installPath="sbin/hello-world")
        self.after_rpm.add_simple_compilation(installPath="sbin/hello-world")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


########
# /lib #
########

# File in /lib in RPM (VERIFY)
class SlashLibFileRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        # Adds /lib/libfoo.so
        self.rpm.add_simple_compilation(installPath="lib/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /lib in compare RPM (VERIFY)
class SlashLibFileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # Adds /lib/libfoo.so
        self.before_rpm.add_simple_compilation(installPath="lib/libfoo.so")
        self.after_rpm.add_simple_compilation(installPath="lib/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /lib in Koji build (VERIFY)
class SlashLibFileKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # Adds /lib/libfoo.so
        self.rpm.add_simple_compilation(installPath="lib/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /lib in compare Koji build (VERIFY)
class SlashLibFileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # Adds /lib/libfoo.so
        self.before_rpm.add_simple_compilation(installPath="lib/libfoo.so")
        self.after_rpm.add_simple_compilation(installPath="lib/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


##########
# /lib64 #
##########

# File in /lib64 in RPM (VERIFY)
class SlashLib64FileRPM(TestRPMs):
    def setUp(self):
        super().setUp()

        # Adds /lib64/libfoo.so
        self.rpm.add_simple_compilation(installPath="lib64/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /lib64 in compare RPM (VERIFY)
class SlashLib64FileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # Adds /lib64/libfoo.so
        self.before_rpm.add_simple_compilation(installPath="lib64/libfoo.so")
        self.after_rpm.add_simple_compilation(installPath="lib64/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /lib64 in Koji build (VERIFY)
class SlashLib64FileKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # Adds /lib64/libfoo.so
        self.rpm.add_simple_compilation(installPath="lib64/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# File in /lib64 in compare Koji build (VERIFY)
class SlashLib64FileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # Adds /lib64/libfoo.so
        self.before_rpm.add_simple_compilation(installPath="lib64/libfoo.so")
        self.after_rpm.add_simple_compilation(installPath="lib64/libfoo.so")

        self.inspection = "pathmigration"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
