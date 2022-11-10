#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import subprocess
import rpmfluff
import unittest

from baseclass import TestSRPM, TestCompareSRPM

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
        self.rpm.section_prep += "%%patch%i\n" % patchIndex

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
        self.after_rpm.section_prep += "%%patch%i\n" % patchIndex

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
