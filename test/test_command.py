#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import subprocess

from baseclass import RequiresRpminspect


# Verify --help gives help output
class RpminspectHelp(RequiresRpminspect):
    def runTest(self):
        RequiresRpminspect.configFile(self)
        p = subprocess.Popen(
            [self.rpminspect, "--help"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        (out, err) = p.communicate()
        self.assertEqual(p.returncode, 0)

        out = out.splitlines()
        t1 = list(filter(lambda x: x.startswith(b"Usage:"), out))
        t2 = list(filter(lambda x: x.startswith(b"Options:"), out))

        self.assertEqual(len(t1), 1)
        self.assertEqual(len(t2), 1)


# Verify rpminspect doesn't segfault on release-less args
class RpminspectSegv(RequiresRpminspect):
    def runTest(self):
        RequiresRpminspect.configFile(self)
        p = subprocess.Popen(
            [self.rpminspect, "42"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        p.communicate()
        self.assertNotEqual(p.returncode, 139)
