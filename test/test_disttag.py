#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import json
import os
import shutil
import subprocess
import tempfile
import unittest
from distutils.version import LooseVersion
from baseclass import TestSRPM, TestRPMs, TestKoji
from baseclass import RequiresRpminspect, check_results

# Older versions of rpm require a Group tag
proc = subprocess.Popen(
    ["rpmbuild", "--version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
(out, err) = proc.communicate()
if LooseVersion(out.split()[2].decode("utf-8")) <= LooseVersion("4.0.4"):
    have_old_rpm = True
else:
    have_old_rpm = False

specdir = os.path.realpath(
    os.path.join(os.environ["RPMINSPECT_TEST_DATA_PATH"], "SPECS")
)

good_spec_file = open(os.path.join(specdir, "good.spec"), "r").read()
good_tabbed_spec_file = open(os.path.join(specdir, "good-tabbed.spec"), "r").read()
good_group_spec_file = open(os.path.join(specdir, "good-group.spec"), "r").read()
good_tabbed_group_spec_file = open(
    os.path.join(specdir, "good-tabbed-group.spec"), "r"
).read()
bad_spec_file = open(os.path.join(specdir, "bad.spec"), "r").read()

# The %autorelease macro is in Fedora under a specific filename, so
# only run those tests if we have it present on the system.
missing_rpmautospec = True

if os.path.isfile("/usr/lib/rpm/macros.d/macros.rpmautospec"):
    f = open("/usr/lib/rpm/macros.d/macros.rpmautospec", "r")
    lines = f.readlines()
    f.close()

    for line in lines:
        if line.index("%autorelease") != -1:
            missing_rpmautospec = False
            break


# Verify missing %{?dist} in Release fails on SRPM (BAD)
class MissingDistTagSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.inspection = "disttag"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Verify missing %{?dist} in Release fails on Koji build (BAD)
class MissingDistTagKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.inspection = "disttag"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Verify running on not an SRPM fails
class DistTagOnNonSRPM(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.release = "1%{?dist}"
        self.inspection = "disttag"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Verify malformed %{?dist} tag in Release fails on SRPM (BAD)
class MalformedDistTagSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.release = "1dist"
        self.inspection = "disttag"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Verify malformed %{?dist} tag in Release fails on Koji build (BAD)
class MalformedDistTagKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.release = "1dist"
        self.inspection = "disttag"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# Verify correct %{?dist} usage passes on SRPM (OK)
class DistTagSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.release = "1%{?dist}"
        self.inspection = "disttag"


# Verify correct %{?dist} usage passes on Koji build (OK)
class DistTagKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.release = "1%{?dist}"
        self.inspection = "disttag"


# Verify system wide macros in the dist tag work in SRPM (OK)
class AutoReleaseDistTagSRPM(TestSRPM):
    @unittest.skipIf(missing_rpmautospec, "test requires the rpmautospec macros")
    def setUp(self):
        super().setUp()
        self.rpm.release = "%autorelease"
        self.inspection = "disttag"


# Verify system wide macros in the dist tag work in Koji build (OK)
class AutoReleaseDistTagKoji(TestKoji):
    @unittest.skipIf(missing_rpmautospec, "test requires the rpmautospec macros")
    def setUp(self):
        super().setUp()
        self.rpm.release = "%autorelease"
        self.inspection = "disttag"


########################################################################
# The test cases below do not use rpmfluff due to limitations in that  #
# Python module.  Instead these test cases inherit RequiresRpminspect  #
# and then invoke rpmbuild manually to construct SRPM files for use to #
# test rpminspect.  At some point rpmfluff may support the macro       #
# expansion style we need to use here, but until then we can just test #
# things this way.                                                     #
########################################################################


# Macros in the Release value with macros expanding to %{?dist} (OK)
class DistTagInMacroSRPM(RequiresRpminspect):
    def setUp(self):
        super().setUp()

        # create a temporary directory to build an SRPM
        self.tmpdir = tempfile.TemporaryDirectory()

        # copy things in to the temporary directory
        shutil.copyfile(self.rpminspect, os.path.join(self.tmpdir.name, "rpminspect"))
        self.specfile = os.path.join(self.tmpdir.name, "rpminspect.spec")
        sf = open(self.specfile, "w")

        if have_old_rpm:
            sf.write(good_group_spec_file)
        else:
            sf.write(good_spec_file)

        sf.close()

        # create the test SRPM (undefine dist to make a predictable
        # SRPM filename)
        args = [
            "rpmbuild",
            "--define",
            "_topdir %s" % self.tmpdir.name,
            "--define",
            "_builddir %s" % self.tmpdir.name,
            "--define",
            "_rpmdir %s" % self.tmpdir.name,
            "--define",
            "_sourcedir %s" % self.tmpdir.name,
            "--define",
            "_specdir %s" % self.tmpdir.name,
            "--define",
            "_srcrpmdir %s" % self.tmpdir.name,
            "--define",
            "_buildrootdir %s" % self.tmpdir.name,
            "-bs",
            os.path.join(self.tmpdir.name, "rpminspect.spec"),
        ]

        if not have_old_rpm:
            args.insert(1, "dist")
            args.insert(1, "--undefine")

        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (out, err) = p.communicate()
        self.srpm = os.path.join(self.tmpdir.name, "example-0.1-4.7.src.rpm")

        # the inspection we are checking
        self.exitcode = 0
        self.inspection = "disttag"
        self.result = "OK"

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        args = [
            self.rpminspect,
            "-d",
            "-c",
            self.conffile,
            "-F",
            "json",
            "-r",
            "GENERIC",
        ]
        args += ["-T", self.inspection]
        args += [self.srpm]

        self.p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()

        try:
            self.results = json.loads(self.out)
        except json.decoder.JSONDecodeError:
            self.dumpResults()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO"] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            self.dumpResults()

        self.assertEqual(self.p.returncode, self.exitcode)
        check_results(self.results, self.inspection, self.result, self.waiver_auth)

    def tearDown(self):
        self.tmpdir.cleanup()
        super().tearDown()


# Macros in the Release value with macros expanding to %{?dist}, tab field separator (OK)
class TabbedDistTagInMacroSRPM(RequiresRpminspect):
    def setUp(self):
        super().setUp()

        # create a temporary directory to build an SRPM
        self.tmpdir = tempfile.TemporaryDirectory()

        # copy things in to the temporary directory
        shutil.copyfile(self.rpminspect, os.path.join(self.tmpdir.name, "rpminspect"))
        self.specfile = os.path.join(self.tmpdir.name, "rpminspect.spec")
        sf = open(self.specfile, "w")

        if have_old_rpm:
            sf.write(good_tabbed_group_spec_file)
        else:
            sf.write(good_tabbed_spec_file)

        sf.close()

        # create the test SRPM (undefine dist to make a predictable
        # SRPM filename)
        args = [
            "rpmbuild",
            "--define",
            "_topdir %s" % self.tmpdir.name,
            "--define",
            "_builddir %s" % self.tmpdir.name,
            "--define",
            "_rpmdir %s" % self.tmpdir.name,
            "--define",
            "_sourcedir %s" % self.tmpdir.name,
            "--define",
            "_specdir %s" % self.tmpdir.name,
            "--define",
            "_srcrpmdir %s" % self.tmpdir.name,
            "--define",
            "_buildrootdir %s" % self.tmpdir.name,
            "-bs",
            os.path.join(self.tmpdir.name, "rpminspect.spec"),
        ]

        if not have_old_rpm:
            args.insert(1, "dist")
            args.insert(1, "--undefine")

        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (out, err) = p.communicate()
        self.srpm = os.path.join(self.tmpdir.name, "example-0.1-4.7.src.rpm")

        # the inspection we are checking
        self.exitcode = 0
        self.inspection = "disttag"
        self.result = "OK"

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        args = [
            self.rpminspect,
            "-d",
            "-c",
            self.conffile,
            "-F",
            "json",
            "-r",
            "GENERIC",
        ]
        args += ["-T", self.inspection]
        args += [self.srpm]

        self.p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()

        try:
            self.results = json.loads(self.out)
        except json.decoder.JSONDecodeError:
            self.dumpResults()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO"] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            self.dumpResults()

        self.assertEqual(self.p.returncode, self.exitcode)
        check_results(self.results, self.inspection, self.result, self.waiver_auth)

    def tearDown(self):
        self.tmpdir.cleanup()
        super().tearDown()
