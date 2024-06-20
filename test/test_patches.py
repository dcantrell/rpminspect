#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import rpm
import subprocess
import rpmfluff
import unittest
from baseclass import TestSRPM, TestCompareSRPM

# rpm < v4.18 does not support '%patch N' syntax
rpmver = list(map(lambda x: int(x), rpm.__version__.strip().split("-")[0].split(".")))
patch_N_supported = True

# Starting with Python 3.10, distutils emits DeprecationWarnings and
# more recent Python releases have simply removed distutils entirely.
# Try the modern replacement before falling back on distutils.  This
# should allow the test suite to work across a range of Python 3.x
# releases.
try:
    from packaging.version import Version, parse

    if parse("%d.%d" % (rpmver[0], rpmver[1])) <= Version("4.18"):
        patch_N_supported = False
except ImportError:
    from distutils.version import LooseVersion

    if LooseVersion("%d.%d" % (rpmver[0], rpmver[1])) < LooseVersion("4.18"):
        patch_N_supported = False

# Check to see if %autopatch works (requires lua)
proc = subprocess.Popen(
    ["rpmbuild", "-E", "%autopatch"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
(out, err) = proc.communicate()

if proc.returncode == 0:
    have_autopatch = True
else:
    have_autopatch = False

# Check to see if %autosetup works (requires lua)
proc = subprocess.Popen(
    ["rpmbuild", "-E", "%autosetup"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
(out, err) = proc.communicate()

if proc.returncode == 0:
    have_autosetup = True
else:
    have_autosetup = False

# test data
source_file = """#include <stdio.h>
int main(void)
{
    printf("Hello, world!\n");
    return 0;
}
"""

patch_file = """--- fortify.c.orig      2020-09-22 14:45:41.592625821 -0400
+++ fortify.c   2020-09-29 10:43:21.829954365 -0400
@@ -1,13 +1,20 @@
 #include <stdio.h>
+#include <stdlib.h>
 #include <string.h>
+#include <errno.h>
+#include <err.h>
+
 int main (int argc, char **argv)
 {
     /* https://access.redhat.com/blogs/766093/posts/1976213 */
     char buffer[5];
+
     printf("Buffer Contains: %s , Size Of Buffer is %lu\n",
            buffer, sizeof(buffer));
     strcpy(buffer, argv[1]);
+    assert(buffer);
     printf("Buffer Contains: %s , Size of Buffer is %lu\n",
            buffer, sizeof(buffer));
-    return 0;
+
+    return EXIT_SUCCESS;
 }
"""

patch_file_changed = """--- fortify.c.orig      2020-09-22 14:45:41.592625821 -0400
+++ fortify.c   2020-09-29 10:51:05.922483632 -0400
@@ -1,13 +1,18 @@
 #include <stdio.h>
+#include <stdlib.h>
 #include <string.h>
+
 int main (int argc, char **argv)
 {
     /* https://access.redhat.com/blogs/766093/posts/1976213 */
     char buffer[5];
+
     printf("Buffer Contains: %s , Size Of Buffer is %lu\n",
            buffer, sizeof(buffer));
     strcpy(buffer, argv[1]);
+    assert(buffer);
     printf("Buffer Contains: %s , Size of Buffer is %lu\n",
            buffer, sizeof(buffer));
-    return 0;
+
+    return EXIT_SUCCESS;
 }
"""

patch_file_threshold = """diff --git a/test/data/forbidden-ipv6.c b/test/data/forbidden-ipv6.c
index 12295de..dd2a9e9 100644
--- a/test/data/forbidden-ipv6.c
+++ b/test/data/forbidden-ipv6.c
@@ -1,11 +1,18 @@
 #include <stdio.h>
+#include <stdlib.h>
+#include <assert.h>
+#include <errno.h>
+#include <err.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
+
 int main (int argc, char **argv)
 {
     const char *cp = "forbidden";
     struct in_addr in;
+
     int ret = inet_aton(cp, &in);
-    return 0;
+
+    return EXIT_SUCCESS;
 }
diff --git a/test/data/fortify.c b/test/data/fortify.c
index d81ab48..d6f5c88 100644
--- a/test/data/fortify.c
+++ b/test/data/fortify.c
@@ -1,13 +1,18 @@
 #include <stdio.h>
+#include <stdlib.h>
 #include <string.h>
+
 int main (int argc, char **argv)
 {
     /* https://access.redhat.com/blogs/766093/posts/1976213 */
     char buffer[5];
+
     printf("Buffer Contains: %s , Size Of Buffer is %lu\n",
            buffer, sizeof(buffer));
     strcpy(buffer, argv[1]);
+    assert(buffer);
     printf("Buffer Contains: %s , Size of Buffer is %lu\n",
            buffer, sizeof(buffer));
-    return 0;
+
+    return EXIT_SUCCESS;
 }
diff --git a/test/data/mathlib.c b/test/data/mathlib.c
index ffc4206..12e7808 100644
--- a/test/data/mathlib.c
+++ b/test/data/mathlib.c
@@ -1,19 +1,19 @@
 double add(double x, double y)
 {
-    return x + y;
+    return (x + y);
 }
 
 double subtract(double x, double y)
 {
-    return x - y;
+    return (x - y);
 }
 
 double multiply(double x, double y)
 {
-    return x * y;
+    return (x * y);
 }
 
 double divide(double x, double y)
 {
-    return x / y;
+    return (x / y);
 }
"""  # noqa: W293


# patch under 4 bytes (BAD)
class PatchUnderFourBytes(TestSRPM):
    def setUp(self):
        super().setUp()

        # rpmbuild will usually catch malformed patches, but only if
        # they are applied in the spec file.  rpminspect will catch
        # malformed patches that are defined but not used.  The second
        # parameter of add_patch() is a boolean indicating whether or
        # not to apply the patch in the spec file, so disable that for
        # this test to see if rpminspect works.
        self.rpm.add_patch(rpmfluff.SourceFile("some.patch", "Qx"), True)

        self.inspection = "patches"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class PatchUnderFourBytesCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # rpmbuild will usually catch malformed patches, but only if
        # they are applied in the spec file.  rpminspect will catch
        # malformed patches that are defined but not used.  The second
        # parameter of add_patch() is a boolean indicating whether or
        # not to apply the patch in the spec file, so disable that for
        # this test to see if rpminspect works.
        self.before_rpm.add_patch(rpmfluff.SourceFile("some.patch", "Qx"), True)
        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", "Qx"), True)

        self.inspection = "patches"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# patch changed (INFO)
class PatchChangedNotRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), True)
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_changed), True
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchChangedInRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), True)
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_changed), True
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch added not a rebase (INFO)
class PatchAddedNotRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), True)

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch added in a rebase (INFO)
class PatchAddedInRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), True)

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch touches too many files (INFO)
class PatchTouchesTooManyFiles(TestSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), True
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchTouchesTooManyFilesCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), True
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch touches too many lines (INFO)
class PatchTouchesTooManyLines(TestSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), True
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchTouchesTooManyLinesCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), True
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch defined and %autosetup used
class PatchDefinedAutoSetupSRPM(TestSRPM):
    @unittest.skipUnless(
        have_autosetup, "rpmbuild lacks %autosetup support (maybe missing liblua)"
    )
    def setUp(self):
        super().setUp()

        # add a patch and use %autosetup
        self.rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)
        self.rpm.section_prep += "%autosetup\n"

        # the %autosetup macro requires at least one SourceN definition
        self.rpm.add_source(rpmfluff.SourceFile("hello.c", source_file))

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchDefinedAutoSetupCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(
        have_autosetup, "rpmbuild lacks %autosetup support (maybe missing liblua)"
    )
    def setUp(self):
        super().setUp()

        # add a patch and use %autosetup
        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)
        self.after_rpm.section_prep += "%autosetup\n"

        # the %autosetup macro requires at least one SourceN definition
        self.after_rpm.add_source(rpmfluff.SourceFile("hello.c", source_file))

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch defined and %autopatch used
class PatchDefinedAutoPatchSRPM(TestSRPM):
    @unittest.skipUnless(
        have_autopatch, "rpmbuild lacks %autopatch support (maybe missing liblua)"
    )
    def setUp(self):
        super().setUp()

        # add a patch and use %autopatch
        self.rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)
        self.rpm.section_prep += "%autopatch\n"

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchDefinedAutoPatchCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(
        have_autopatch, "rpmbuild lacks %autopatch support (maybe missing liblua)"
    )
    def setUp(self):
        super().setUp()

        # add a patch and use %autopatch
        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)
        self.after_rpm.section_prep += "%autopatch\n"

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch defined but not applied
class PatchDefinedButNotAppliedSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # define the patch
        self.rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class PatchDefinedButNotAppliedCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # define the patch
        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# patches defined but some not applied
class PatchesDefinedButSomeNotAppliedSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # define the patch
        self.rpm.add_patch(rpmfluff.SourceFile("some1.patch", patch_file), True)
        self.rpm.add_patch(rpmfluff.SourceFile("some2.patch", patch_file), True)
        self.rpm.add_patch(rpmfluff.SourceFile("some3.patch", patch_file), False)
        self.rpm.add_patch(rpmfluff.SourceFile("some4.patch", patch_file), True)
        self.rpm.add_patch(rpmfluff.SourceFile("some5.patch", patch_file), False)

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class PatchesDefinedButSomeNotAppliedCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # define the patch
        self.after_rpm.add_patch(rpmfluff.SourceFile("some1.patch", patch_file), True)
        self.after_rpm.add_patch(rpmfluff.SourceFile("some2.patch", patch_file), True)
        self.after_rpm.add_patch(rpmfluff.SourceFile("some3.patch", patch_file), False)
        self.after_rpm.add_patch(rpmfluff.SourceFile("some4.patch", patch_file), True)
        self.after_rpm.add_patch(rpmfluff.SourceFile("some5.patch", patch_file), False)

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# patch defined without a space after ':'
class PatchDefinedWithoutFieldSpaceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # add a patch without a space after the ':'
        patch = rpmfluff.SourceFile("some.patch", patch_file)
        patchIndex = len(self.rpm.patches)
        self.rpm.patches[patchIndex] = patch
        self.rpm.section_patches += "Patch%s:%s\n" % (patchIndex, patch.sourceName)
        self.rpm.add_check(rpmfluff.CheckSourceFile(patch.sourceName))
        self.rpm.section_prep += "%%patch -P %i\n" % patchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchDefinedWithoutFieldSpaceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add a patch without a space after the ':'
        patch = rpmfluff.SourceFile("some.patch", patch_file)
        patchIndex = len(self.after_rpm.patches)
        self.after_rpm.patches[patchIndex] = patch
        self.after_rpm.section_patches += "Patch%s:%s\n" % (
            patchIndex,
            patch.sourceName,
        )
        self.after_rpm.add_check(rpmfluff.CheckSourceFile(patch.sourceName))
        self.after_rpm.section_prep += "%%patch -P %i\n" % patchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch filename has a spec file macro in it
class PatchFilenameWithMacroSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # define a macro
        self.rpm.header += "\n%define super_patch_number 47\n"

        # add a patch using a macro in a filename
        self.rpm.add_patch(
            rpmfluff.SourceFile("some-47.patch", patch_file),
            True,
            patchUrl="some-%{super_patch_number}.patch",
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchFilenameWithMacroCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # define a macro
        self.before_rpm.header += "\n%define super_patch_number 47\n"
        self.after_rpm.header += "\n%define super_patch_number 47\n"

        # add a patch using a macro in a filename
        self.before_rpm.add_patch(
            rpmfluff.SourceFile("some-47.patch", patch_file),
            True,
            patchUrl="some-%{super_patch_number}.patch",
        )
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some-47.patch", patch_file),
            True,
            patchUrl="some-%{super_patch_number}.patch",
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# '%patch N' syntax used to apply patch
class PatchNMacroSRPM(TestSRPM):
    @unittest.skipUnless(
        patch_N_supported, "rpmbuild v4.14 does not support '%patch N' syntax"
    )
    def setUp(self):
        super().setUp()

        # add the patch with '%patch N' syntax
        patch = rpmfluff.SourceFile("some.patch", patch_file)
        patchIndex = len(self.rpm.patches)
        self.rpm.patches[patchIndex] = patch

        self.rpm.section_patches += "Patch%i: %s\n" % (patchIndex, patch.sourceName)
        self.rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))

        self.rpm.section_prep += "%%patch %i\n" % patchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchNMacroCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(
        patch_N_supported, "rpmbuild v4.14 does not support '%patch N' syntax"
    )
    def setUp(self):
        super().setUp()

        # add the patch with '%patch N' syntax
        patch = rpmfluff.SourceFile("some.patch", patch_file)

        beforePatchIndex = len(self.before_rpm.patches)
        self.before_rpm.patches[beforePatchIndex] = patch
        self.before_rpm.section_patches += "Patch%i: %s\n" % (
            beforePatchIndex,
            patch.sourceName,
        )
        self.before_rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))
        self.before_rpm.section_prep += "%%patch %i\n" % beforePatchIndex

        afterPatchIndex = len(self.after_rpm.patches)
        self.after_rpm.patches[afterPatchIndex] = patch
        self.after_rpm.section_patches += "Patch%i: %s\n" % (
            afterPatchIndex,
            patch.sourceName,
        )
        self.after_rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))
        self.after_rpm.section_prep += "%%patch %i\n" % afterPatchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# '%patch -P N' syntax used to apply patch
class PatchP_NMacroSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # add the patch with '%patch -P N' syntax
        patch = rpmfluff.SourceFile("some.patch", patch_file)
        patchIndex = len(self.rpm.patches)
        self.rpm.patches[patchIndex] = patch

        self.rpm.section_patches += "Patch%i: %s\n" % (patchIndex, patch.sourceName)
        self.rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))

        self.rpm.section_prep += "%%patch -P %i\n" % patchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchP_NMacroCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add the patch with '%patch -P N' syntax
        patch = rpmfluff.SourceFile("some.patch", patch_file)

        beforePatchIndex = len(self.before_rpm.patches)
        self.before_rpm.patches[beforePatchIndex] = patch
        self.before_rpm.section_patches += "Patch%i: %s\n" % (
            beforePatchIndex,
            patch.sourceName,
        )
        self.before_rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))
        self.before_rpm.section_prep += "%%patch -P %i\n" % beforePatchIndex

        afterPatchIndex = len(self.after_rpm.patches)
        self.after_rpm.patches[afterPatchIndex] = patch
        self.after_rpm.section_patches += "Patch%i: %s\n" % (
            afterPatchIndex,
            patch.sourceName,
        )
        self.after_rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))
        self.after_rpm.section_prep += "%%patch -P %i\n" % afterPatchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# '%patch -PN' syntax used to apply patch
class PatchPNMacroSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # add the patch with '%patch -PN' syntax
        patch = rpmfluff.SourceFile("some.patch", patch_file)
        patchIndex = len(self.rpm.patches)
        self.rpm.patches[patchIndex] = patch

        self.rpm.section_patches += "Patch%i: %s\n" % (patchIndex, patch.sourceName)
        self.rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))

        self.rpm.section_prep += "%%patch -P%i\n" % patchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchPNMacroCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add the patch with '%patch -PN' syntax
        patch = rpmfluff.SourceFile("some.patch", patch_file)

        beforePatchIndex = len(self.before_rpm.patches)
        self.before_rpm.patches[beforePatchIndex] = patch
        self.before_rpm.section_patches += "Patch%i: %s\n" % (
            beforePatchIndex,
            patch.sourceName,
        )
        self.before_rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))
        self.before_rpm.section_prep += "%%patch -P%i\n" % beforePatchIndex

        afterPatchIndex = len(self.after_rpm.patches)
        self.after_rpm.patches[afterPatchIndex] = patch
        self.after_rpm.section_patches += "Patch%i: %s\n" % (
            afterPatchIndex,
            patch.sourceName,
        )
        self.after_rpm.add_check(rpmfluff.check.CheckSourceFile(patch.sourceName))
        self.after_rpm.section_prep += "%%patch -P%i\n" % afterPatchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
