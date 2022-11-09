#
# Copyright 2022 Red Hat, Inc.
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

from baseclass import TestModule, TestCompareModules


# Module has %{modularitylabel} defined (OK)
class ModuleHasModularityLabel(TestModule):
    def setUp(self):
        super().setUp(modularitylabel=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveModularityLabel(TestCompareModules):
    def setUp(self):
        super().setUp(modularitylabel=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "OK"


# Module does not have %{modularitylabel} defined (BAD)
class ModuleLacksModularityLabel(TestModule):
    def setUp(self):
        super().setUp(modularitylabel=False)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "BAD"
        self.waiverauth = "Anyone"


class ModulesLacksModularityLabel(TestCompareModules):
    def setUp(self):
        super().setUp(modularitylabel=False)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "BAD"
        self.waiverauth = "Anyone"


# Module has static_context and rules require it (OK)
class ModuleHasStaticContext(TestModule):
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveStaticContext(TestCompareModules):
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"


# Module has static_context and rules recommend it (OK)
class ModuleHasStaticContextAndRecommended(TestModule):
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "recommend"

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveStaticContextAndRecommended(TestCompareModules):
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "recommend"

        self.inspection = "modularity"
        self.result = "OK"


# Module has static_context and rules forbid it (VERIFY)
class ModuleHasStaticContextAndForbidden(TestModule):
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "forbidden"

        self.inspection = "modularity"
        self.result = "VERIFY"
        self.waiverauth = "Anyone"


class ModulesHaveStaticContextAndForbidden(TestCompareModules):
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "forbidden"

        self.inspection = "modularity"
        self.result = "VERIFY"
        self.waiverauth = "Anyone"


# Module has static_context and rules do not exist (INFO)
class ModuleHasStaticContextWithNoRule(TestModule):
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveStaticContextWithNoRule(TestCompareModules):
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"
