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

import os
import platform
import shutil
import socket
import subprocess
import tempfile
import unittest
import json
import yaml
import rpmfluff

# NVRs to use for the fake test packages
BEFORE_NAME = "vaporware"
BEFORE_VER = "0.1"
BEFORE_REL = "1"

AFTER_NAME = BEFORE_NAME
AFTER_VER = "0.1"
AFTER_REBASE_VER = "47.0"
AFTER_REL = "2"

# Test suite Vendor name
VENDOR = "rpminspect Test Vendor Ltd."

# Run the test suite with KEEP=y (or set to anything) in the
# environment to instruct the test suite to keep intermediate files
# and results.
if os.getenv("KEEP") is None:
    KEEP_RESULTS = False
else:
    KEEP_RESULTS = True


# Exceptions used by the test suite
class MissingRpminspect(Exception):
    pass


class MissingRpminspectConf(Exception):
    pass


# This function is called to check the JSON results to make sure we
# have at least one result that matches the result and waiver_auth we
# were expecting.
def check_results(results, inspection, result, waiver_auth=None, message=None):
    if results == {} or results == [] or results is None:
        raise AssertionError("JSON test result is empty")

    if inspection not in results:
        raise AssertionError(
            f'The inspection "{inspection}" is missing from results "{results}"'
        )

    for r in results[inspection]:
        if "result" in r and ((waiver_auth is None) or ("waiver authorization" in r)):
            if r["result"] == result and (
                (waiver_auth is None) or (r["waiver authorization"] == waiver_auth)
            ):
                # handle systems with partially configured rpmbuild setups
                if message:
                    m = r.get("message")
                else:
                    m = None

                if m and m.find("%{_arch}") != -1:
                    m = m.replace("%{_arch}", platform.machine())

                if m != message:
                    raise AssertionError(
                        f'Expected result message "{m}" but it was "{m}"'
                    )
                break
    else:
        raise AssertionError(
            f"Expected result={result} with waiver authorization={waiver_auth} in "
            f"{json.dumps(results[inspection], sort_keys=True, indent=4)}"
        )


# This is a local replacement for ExternalSourceFile.  Rather than
# reading in the file and writing it out, it just uses shutil.copy to
# copy the file in place.
class ProvidedSourceFile:
    def __init__(self, sourceName, path):
        self.sourceName = sourceName
        self.path = path

    def _get_dst_file(self, sourcesDir):
        dstFileName = os.path.join(sourcesDir, self.sourceName)
        return dstFileName

    def write_file(self, sourcesDir):
        dstFile = self._get_dst_file(sourcesDir)
        shutil.copy(self.path, dstFile)


# This is a local extension to SimpleRpmBuild in rpmfluff.  For the
# SRPM only test classes, override the do_make() function so that
# rpmbuild only builds the SRPM and not all of the packages.
#
# The only significant difference here is that rpmbuild is called
# with the '-bs' option instead of '-ba'.  Enough functions are
# duplicated here in order to make it all work.
class SimpleSrpmBuild(rpmfluff.SimpleRpmBuild):
    def __write_log(self, log, arch):
        log_dir = self.get_build_log_dir(arch)
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
        filename = self.get_build_log_path(arch)
        f = open(filename, "wb")
        for line in log:
            f.write(line)
        f.close()

    def __create_directories(self):
        """Sets up the directory hierarchy for the build"""
        if hasattr(self, "tmpdir"):
            if self.tmpdir and not (
                self.tmpdir_location and os.path.isdir(self.tmpdir_location)
            ):
                self.tmpdir_location = tempfile.mkdtemp(prefix="rpmfluff-")
        os.mkdir(self.get_base_dir())

        # Make fake rpmbuild directories
        for subDir in ["BUILD", "SOURCES", "SRPMS", "RPMS"]:
            os.mkdir(os.path.join(self.get_base_dir(), subDir))

    def do_make(self):
        """
        Hook to actually perform the rpmbuild, gathering the necessary source files first
        """
        self.clean()

        self.__create_directories()

        specFileName = self.gather_spec_file(self.get_base_dir())

        sourcesDir = self.get_sources_dir()
        self.gather_sources(sourcesDir)

        absBaseDir = os.path.abspath(self.get_base_dir())

        buildArchs = ()
        if self.buildArchs:
            buildArchs = self.buildArchs
        else:
            if hasattr(rpmfluff, "utils"):
                buildArchs = (rpmfluff.utils.expectedArch,)
            else:
                buildArchs = (rpmfluff.expectedArch,)
        for arch in buildArchs:
            command = [
                "rpmbuild",
                "--nodeps",
                "--define",
                "_topdir %s" % absBaseDir,
                "--define",
                "_rpmfilename %%{ARCH}/%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm",
                "-bs",
                "--target",
                arch,
                specFileName,
            ]
            try:
                log = subprocess.check_output(
                    command, stderr=subprocess.STDOUT
                ).splitlines(True)
            except subprocess.CalledProcessError as e:
                raise RuntimeError(
                    "rpmbuild command failed with exit status %s: %s\n%s"
                    % (e.returncode, e.cmd, e.output)
                )
            self.__write_log(log, arch)


# Base test case class that ensures we have 'rpminspect'
# as an executable command.
class RequiresRpminspect(unittest.TestCase):
    def setUp(self):
        super().setUp()

        # make sure we have the program
        self.rpminspect = os.environ["RPMINSPECT"]

        if not os.path.isfile(self.rpminspect) or not os.access(
            self.rpminspect, os.X_OK
        ):
            raise MissingRpminspect

        # ensure we have the sample configuration file
        if not os.path.isfile(os.environ["RPMINSPECT_YAML"]) or not os.access(
            os.environ["RPMINSPECT_YAML"], os.R_OK
        ):
            raise MissingRpminspectConf

        # let test cases override the build type
        self.buildtype = "rpm"

        # set in configFile()
        self.conffile = None
        self.extra_cfg = None

        # use for rpminspect results
        (handle, self.outputfile) = tempfile.mkstemp()
        os.close(handle)

        # settings that the inheriting test can override
        self.buildhost_subdomain = None

        # Set this to the test's expected result message, if any. If it is
        # None, the value is not checked.
        self.message = None

        # Default expected test results
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = None

    def dumpResults(self):
        r = None

        if len(self.results) > 0:
            r = json.dumps(self.results, sort_keys=True, indent=4)

        print("\n\ninspection=%s\n" % self.inspection)

        if r is not None:
            print("results:\n%s\n" % r)

        if len(self.out) > 0:
            print("stdout:\n%s\n" % str(self.out, "utf-8"))

        if len(self.err) > 0:
            print("stderr:\n%s\n" % str(self.err, "utf-8"))

        print("\n")

    def configFile(self):
        # create a copy of the sample conf file for test purposes
        (handle, self.conffile) = tempfile.mkstemp()
        os.close(handle)

        shutil.copyfile(os.environ["RPMINSPECT_YAML"], self.conffile)

        instream = open(os.environ["RPMINSPECT_YAML"], "r")

        # modify settings for the test suite based on where it's running
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["metadata"]["vendor"] = VENDOR
        cfg["vendor"]["vendor_data_dir"] = os.environ["RPMINSPECT_TEST_DATA_PATH"]
        cfg["vendor"]["licensedb"] = ["test.json"]

        if self.buildhost_subdomain:
            cfg["metadata"]["buildhost_subdomain"] = [self.buildhost_subdomain]
        else:
            hn = socket.getfqdn()
            hnd = None

            if hn.find(".") != -1:
                hnd = "." + hn.split(".", 1)[1]

            if hn.startswith("localhost"):
                cfg["metadata"]["buildhost_subdomain"] = ["localhost", hn]
            else:
                cfg["metadata"]["buildhost_subdomain"] = [hn]

            if hnd:
                cfg["metadata"]["buildhost_subdomain"].append(hnd)

        # any additional config settings for the test case
        if self.extra_cfg is not None:
            cfg.update(self.extra_cfg)

        # write the temporary config file for the test suite
        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()

    def tearDown(self):
        if not KEEP_RESULTS:
            os.unlink(self.conffile)
            os.unlink(self.outputfile)


# Base test case class that tests on the SRPM package only
class TestSRPM(RequiresRpminspect):
    def setUp(self):
        super().setUp()
        self.rpm = SimpleSrpmBuild(AFTER_NAME, AFTER_VER, AFTER_REL)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.rpm.header += "\n%global __os_install_post %{nil}\n"

        # the inheriting class needs to override these
        self.inspection = None
        self.result_inspection = None

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        if not self.result_inspection:
            self.result_inspection = self.inspection

        self.rpm.do_make()

        args = [
            self.rpminspect,
            "-D",
            "-d",
            "-c",
            self.conffile,
            "-b",
            self.buildtype,
            "-F",
            "json",
            "-r",
            "GENERIC",
            "-o",
            self.outputfile,
        ]

        if self.inspection:
            args.append("-T")
            args.append(self.inspection)

        if KEEP_RESULTS:
            args.append("-k")

        args.append(self.rpm.get_built_srpm())

        self.p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()

        self.results = None

        try:
            with open(self.outputfile) as f:
                self.results = json.loads(f.read().encode("utf-8"))
        except json.decoder.JSONDecodeError:
            self.dumpResults()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO", "DIAGNOSTICS"] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            self.dumpResults()

        self.assertEqual(self.p.returncode, self.exitcode)
        check_results(
            self.results, self.result_inspection, self.result, self.waiver_auth
        )

    def tearDown(self):
        if not KEEP_RESULTS:
            super().tearDown()
            self.rpm.clean()


# Base test case class that compares a before and after SRPM
class TestCompareSRPM(RequiresRpminspect):
    def setUp(self, rebase=False, same=False):
        super().setUp()

        if rebase:
            after_ver = AFTER_REBASE_VER
        else:
            after_ver = AFTER_VER

        if same:
            after_name = BEFORE_NAME
            after_ver = BEFORE_VER
        else:
            after_name = AFTER_NAME

        self.before_rpm = SimpleSrpmBuild(BEFORE_NAME, BEFORE_VER, BEFORE_REL)
        self.after_rpm = SimpleSrpmBuild(after_name, after_ver, AFTER_REL)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        # the inheriting class needs to override these
        self.inspection = None
        self.result_inspection = None

        # defaults
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = None

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        if not self.result_inspection:
            self.result_inspection = self.inspection

        self.before_rpm.do_make()
        self.after_rpm.do_make()

        args = [
            self.rpminspect,
            "-d",
            "-c",
            self.conffile,
            "-b",
            self.buildtype,
            "-F",
            "json",
            "-r",
            "GENERIC",
            "-o",
            self.outputfile,
        ]

        if self.inspection:
            args.append("-T")
            args.append(self.inspection)

        if KEEP_RESULTS:
            args.append("-k")

        args.append(self.before_rpm.get_built_srpm())
        args.append(self.after_rpm.get_built_srpm())

        self.p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()

        try:
            with open(self.outputfile) as f:
                self.results = json.loads(f.read().encode("utf-8"))
        except json.decoder.JSONDecodeError:
            self.dumpResults()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO", "DIAGNOSTICS"] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            self.dumpResults()

        self.assertEqual(self.p.returncode, self.exitcode)
        check_results(
            self.results,
            self.result_inspection,
            self.result,
            self.waiver_auth,
            message=self.message,
        )

    def tearDown(self):
        if not KEEP_RESULTS:
            super().tearDown()
            self.before_rpm.clean()
            self.after_rpm.clean()


# Base test case class that tests the binary RPMs
class TestRPMs(RequiresRpminspect):
    def setUp(self):
        super().setUp()
        self.rpm = rpmfluff.SimpleRpmBuild(AFTER_NAME, AFTER_VER, AFTER_REL)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.rpm.header += "\n%global __os_install_post %{nil}\n"

        # the inheriting class needs to override these
        self.inspection = None
        self.result_inspection = None

        # defaults
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = None

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        if not self.result_inspection:
            self.result_inspection = self.inspection

        self.rpm.do_make()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO", "DIAGNOSTICS"] and self.exitcode == 0:
            self.exitcode = 1

        for a in self.rpm.get_build_archs():
            args = [
                self.rpminspect,
                "-d",
                "-c",
                self.conffile,
                "-b",
                self.buildtype,
                "-F",
                "json",
                "-r",
                "GENERIC",
                "-o",
                self.outputfile,
            ]

            if self.inspection:
                args.append("-T")
                args.append(self.inspection)

            if KEEP_RESULTS:
                args.append("-k")

            args.append(self.rpm.get_built_rpm(a))

            self.p = subprocess.Popen(
                args, stdout=subprocess.PIPE, stderr=subprocess.PIPE
            )
            (self.out, self.err) = self.p.communicate()

            try:
                with open(self.outputfile) as f:
                    self.results = json.loads(f.read().encode("utf-8"))
            except json.decoder.JSONDecodeError:
                self.dumpResults()

            # dump stdout and stderr if these do not match
            if self.p.returncode != self.exitcode:
                self.dumpResults()

            self.assertEqual(self.p.returncode, self.exitcode)
            self.assertEqual(
                self.results[self.result_inspection][0]["result"], self.result
            )
            if "waiver authorization" in self.results[self.result_inspection][0]:
                self.assertEqual(
                    self.results[self.result_inspection][0]["waiver authorization"],
                    self.waiver_auth,
                )

    def tearDown(self):
        if not KEEP_RESULTS:
            super().tearDown()
            self.rpm.clean()


# Base test case class that compares before and after built RPMs
class TestCompareRPMs(RequiresRpminspect):
    def setUp(self, rebase=False, same=False):
        super().setUp()

        if rebase:
            after_ver = AFTER_REBASE_VER
        else:
            after_ver = AFTER_VER

        if same:
            after_name = BEFORE_NAME
            after_ver = BEFORE_VER
        else:
            after_name = AFTER_NAME

        self.before_rpm = rpmfluff.SimpleRpmBuild(BEFORE_NAME, BEFORE_VER, BEFORE_REL)
        self.after_rpm = rpmfluff.SimpleRpmBuild(after_name, after_ver, AFTER_REL)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        # the inheriting class needs to override these
        self.inspection = None
        self.result_inspection = None

        # defaults
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = None

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        if not self.result_inspection:
            self.result_inspection = self.inspection

        self.before_rpm.do_make()
        self.after_rpm.do_make()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO", "DIAGNOSTICS"] and self.exitcode == 0:
            self.exitcode = 1

        for a in self.before_rpm.get_build_archs():
            args = [
                self.rpminspect,
                "-d",
                "-c",
                self.conffile,
                "-b",
                self.buildtype,
                "-F",
                "json",
                "-r",
                "GENERIC",
                "-o",
                self.outputfile,
            ]

            if self.inspection:
                args.append("-T")
                args.append(self.inspection)

            if KEEP_RESULTS:
                args.append("-k")

            args.append(self.before_rpm.get_built_rpm(a))
            args.append(self.after_rpm.get_built_rpm(a))

            self.p = subprocess.Popen(
                args, stdout=subprocess.PIPE, stderr=subprocess.PIPE
            )
            (self.out, self.err) = self.p.communicate()

            try:
                with open(self.outputfile) as f:
                    self.results = json.loads(f.read().encode("utf-8"))
            except json.decoder.JSONDecodeError:
                self.dumpResults()

            # dump stdout and stderr if these do not match
            if self.p.returncode != self.exitcode:
                self.dumpResults()

            self.assertEqual(self.p.returncode, self.exitcode)
            check_results(
                self.results,
                self.result_inspection,
                self.result,
                self.waiver_auth,
                message=self.message,
            )

    def tearDown(self):
        if not KEEP_RESULTS:
            super().tearDown()
            self.before_rpm.clean()
            self.after_rpm.clean()


# Base test case class that tests a fake Koji build
class TestKoji(TestRPMs):
    def setUp(self):
        super().setUp()
        self.kojidir = tempfile.mkdtemp()

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        if not self.result_inspection:
            self.result_inspection = self.inspection

        # generate the build
        self.rpm.do_make()

        # copy everything in to place as if Koji built it
        # copy over the SRPM to the fake koji build
        srcdir = os.path.join(self.kojidir, "src")
        os.makedirs(srcdir, exist_ok=True)
        shutil.copy(self.rpm.get_built_srpm(), srcdir)

        # copy over the built RPMs to the fake koji build
        for a in self.rpm.get_build_archs():
            adir = os.path.join(self.kojidir, a)
            os.makedirs(adir, exist_ok=True)
            for sp in self.rpm.get_subpackage_names():
                src = self.rpm.get_built_rpm(a, sp)
                shutil.copy(src, adir)

        args = [
            self.rpminspect,
            "-d",
            "-c",
            self.conffile,
            "-b",
            self.buildtype,
            "-F",
            "json",
            "-r",
            "GENERIC",
            "-o",
            self.outputfile,
        ]
        if self.inspection:
            args.append("-T")
            args.append(self.inspection)

        if KEEP_RESULTS:
            args.append("-k")

        args.append(self.kojidir)

        self.p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()

        try:
            with open(self.outputfile) as f:
                self.results = json.loads(f.read().encode("utf-8"))
        except json.decoder.JSONDecodeError:
            self.dumpResults()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO", "DIAGNOSTICS"] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            self.dumpResults()

        self.assertEqual(self.p.returncode, self.exitcode)
        check_results(
            self.results,
            self.result_inspection,
            self.result,
            self.waiver_auth,
            message=self.message,
        )

        if KEEP_RESULTS:
            print("Test builds: %s" % self.kojidir)

    def tearDown(self):
        if not KEEP_RESULTS:
            super().tearDown()
            shutil.rmtree(self.kojidir, ignore_errors=True)


# Base test case class that compares before and after Koji builds
class TestCompareKoji(TestCompareRPMs):
    def setUp(self, rebase=False, same=False):
        super().setUp(rebase=rebase, same=same)
        self.kojidir = tempfile.mkdtemp()

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        if not self.result_inspection:
            self.result_inspection = self.inspection

        # generate the build
        self.before_rpm.do_make()
        self.after_rpm.do_make()

        # copy everything in to place as if Koji built it
        # copy over the SRPM to the fake koji build
        beforesrcdir = os.path.join(self.kojidir, "before", "src")
        aftersrcdir = os.path.join(self.kojidir, "after", "src")
        os.makedirs(beforesrcdir, exist_ok=True)
        os.makedirs(aftersrcdir, exist_ok=True)
        shutil.copy(self.before_rpm.get_built_srpm(), beforesrcdir)
        shutil.copy(self.after_rpm.get_built_srpm(), aftersrcdir)

        # copy over the built RPMs to the fake koji build
        for a in self.before_rpm.get_build_archs():
            adir = os.path.join(self.kojidir, "before", a)
            os.makedirs(adir, exist_ok=True)
            for sp in self.before_rpm.get_subpackage_names():
                shutil.copy(self.before_rpm.get_built_rpm(a, sp), adir)

        for a in self.after_rpm.get_build_archs():
            adir = os.path.join(self.kojidir, "after", a)
            os.makedirs(adir, exist_ok=True)
            for sp in self.after_rpm.get_subpackage_names():
                shutil.copy(self.after_rpm.get_built_rpm(a, sp), adir)

        args = [
            self.rpminspect,
            "-d",
            "-c",
            self.conffile,
            "-b",
            self.buildtype,
            "-F",
            "json",
            "-r",
            "GENERIC",
            "-o",
            self.outputfile,
        ]

        if self.inspection:
            args.append("-T")
            args.append(self.inspection)

        if KEEP_RESULTS:
            args.append("-k")

        args.append(os.path.join(self.kojidir, "before"))
        args.append(os.path.join(self.kojidir, "after"))

        self.p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()
        self.results = []

        try:
            with open(self.outputfile) as f:
                self.results = json.loads(f.read().encode("utf-8"))
        except json.decoder.JSONDecodeError:
            self.dumpResults()

        # anything not OK or INFO is a non-zero return
        if self.result not in ["OK", "INFO", "DIAGNOSTICS"] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            self.dumpResults()

        self.assertEqual(self.p.returncode, self.exitcode)
        try:
            check_results(
                self.results,
                self.result_inspection,
                self.result,
                self.waiver_auth,
                message=self.message,
            )
        except AssertionError:
            self.dumpResults()

        # show where results are
        if KEEP_RESULTS:
            print("Test builds: %s" % self.kojidir)

    def tearDown(self):
        if not KEEP_RESULTS:
            super().tearDown()
            shutil.rmtree(self.kojidir, ignore_errors=True)
