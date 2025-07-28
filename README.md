rpminspect
==========

RPM build deviation analysis tools.  rpminspect looks at the output of
an RPM build (e.g., the output of a Koji build) and examines the
contents of the build artifacts to report:

* Policy compliance

* Changes from a previous build to the current build

* General correctness and best practices

rpminspect is the frontend tool, and librpminspect provides the
inspection engine and inspections.  The program is configured through
the a configuration file and runtime data is provided by a
vendor-specific rpminspect-data package.  The rpminspect-data-generic
package in this source tree provides a template for constructing the
vendor-specific data package.

Status
------

[![pre-commit](https://github.com/rpminspect/rpminspect/actions/workflows/pre-commit.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/pre-commit.yml)
[![ShellCheck](https://github.com/rpminspect/rpminspect/actions/workflows/shellcheck.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/shellcheck.yml)

For each merge and pull request, rpminspect is built and tested in the
following environments:

[x86_64](https://en.wikipedia.org/wiki/X86-64):

| Platform | Release | Status |
|:-------------------------------------------------------|:----|----:|
| [Fedora Linux](https://getfedora.org/)                 | rawhide<br />stable | [![Fedora Linux](https://github.com/rpminspect/rpminspect/actions/workflows/fedora.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/fedora.yml) |
| [CentOS Stream](https://www.centos.org/)               | 9<br />10           | [![CentOS](https://github.com/rpminspect/rpminspect/actions/workflows/centos.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/centos.yml) |
| [Ubuntu Linux](https://ubuntu.com/)                    | stable              | [![Ubuntu Linux](https://github.com/rpminspect/rpminspect/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/ubuntu.yml) |
| [Debian GNU/Linux](https://www.debian.org/)            | testing<br />stable | [![Debian GNU/Linux](https://github.com/rpminspect/rpminspect/actions/workflows/debian.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/debian.yml) |
| [OpenSUSE Linux](https://www.opensuse.org/)            | Leap                | [![OpenSUSE](https://github.com/rpminspect/rpminspect/actions/workflows/opensuse.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/opensuse.yml) |
| [Arch Linux](https://archlinux.org/)                   | stable              | [![Arch Linux](https://github.com/rpminspect/rpminspect/actions/workflows/archlinux.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/archlinux.yml) |
| [Gentoo Linux](https://www.gentoo.org/)                | stable              | [![Gentoo Linux](https://github.com/rpminspect/rpminspect/actions/workflows/gentoo.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/gentoo.yml) |
| [Alpine Linux](https://www.alpinelinux.org/)           | stable              | [![Alpine Linux](https://github.com/rpminspect/rpminspect/actions/workflows/alpinelinux.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/alpinelinux.yml) |
| [AlmaLinux](https://almalinux.org/)                    | 8<br />9<br />10    | [![AlmaLinux](https://github.com/rpminspect/rpminspect/actions/workflows/almalinux.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/almalinux.yml) |
| [Mageia Linux](https://www.mageia.org/)                | stable              | [![Mageia Linux](https://github.com/rpminspect/rpminspect/actions/workflows/mageia.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/mageia.yml) |
| [FreeBSD](https://www.freebsd.org/)                    | 13.x                | [![FreeBSD](https://github.com/rpminspect/rpminspect/actions/workflows/freebsd.yml/badge.svg)](https://github.com/rpminspect/rpminspect/actions/workflows/freebsd.yml) |


Documentation
-------------

The latest documentation is available at https://rpminspect.readthedocs.io/

The documentation is regenerated each time there is a push or pull
request merged in to the master branch.


Build Types Support
-------------------

rpminspect expects its input to either be local RPM packages or a Koji
build.  Koji produces certain types of builds that are not supported
by rpminspect.  Right now, the following input types are supported:

* Local RPM packages (source or binary)

* Koji builds (i.e., an SRPM run with 'rpmbuild -ba' on all specified architectures)

* Modules (https://docs.fedoraproject.org/en-US/modularity/)

See https://pagure.io/koji for more information on Koji.

If comparing local RPM packages, rpminspect assumes the before and
after specifications are peers whereas for a Koji build, rpminspect
matches peer packages (e.g., in the gcc package, gcc-gfortran in the
before build is peered with gcc-gfortran in the after build).


Intended Audience
-----------------

Developers, QE, release engineers, and system administrators who
regularly build RPMs for use in some environments or products.

From an individual user standpoint, rpminspect is a command-line tool
you can use as a linter of sorts.  rpminspect reports, and that's it.
rpminspect can output information in JSON format, which makes it
easier to integrate with automated workflows or web frontends.

The reporting uses certain classifications for different things found,
but it up to the end-user to determine what to do with that
information.  If used with an external build tool, the JSON data may
be more useful as you can construct decision making around those
results.


Build Requirements
------------------

A typical Linux system with a toolchain for building C software using
meson and ninja, plus the following libraries:

| Dependency | Required | Notes | License |
| :--- | :---: | :--- | :--- |
| [json-c](http://json-c.github.io/json-c/) | :heavy_check_mark: | | [MIT](https://spdx.org/licenses/MIT.html) |
| [xmlrpc-c](http://xmlrpc-c.sourceforge.net/) | :heavy_check_mark: | | [BSD-3-Clause](https://spdx.org/licenses/BSD-3-Clause.html) and [MIT](https://spdx.org/licenses/MIT.html) |
| [libxml-2.0](http://xmlsoft.org/) | :heavy_check_mark: | | [MIT](https://spdx.org/licenses/MIT.html) |
| [rpm](https://github.com/rpm-software-management/rpm) | :heavy_check_mark: | | [GPL-2.0-or-later](https://spdx.org/licenses/GPL-2.0-or-later.html) |
| [libarchive](https://www.libarchive.org/) | :heavy_check_mark: | | [BSD-2-Clause](https://spdx.org/licenses/BSD-2-Clause.html) and [BSD-3-Clause](https://spdx.org/licenses/BSD-3-Clause.html) |
| [elfutils](https://sourceware.org/elfutils/) | :heavy_check_mark: | libelf | [LGPL-3.0-or-later](https://spdx.org/licenses/LGPL-3.0-or-later.html) |
| [kmod](https://git.kernel.org/pub/scm/utils/kernel/kmod/kmod.git) |                  | Linux only | [GPL-2.0-or-later](https://spdx.org/licenses/GPL-2.0-or-later.html) |
| [zlib](https://www.zlib.net/) | :heavy_check_mark: | | [Zlib](https://spdx.org/licenses/Zlib.html) and [BSL-1.0](https://spdx.org/licenses/BSL-1.0.html) |
| [mandoc](https://mandoc.bsd.lv/) | :heavy_check_mark: | libmandoc.a | [ISC](https://spdx.org/licenses/ISC.html) |
| [libyaml](https://github.com/yaml/libyaml) | :heavy_check_mark: | | [MIT](https://spdx.org/licenses/MIT.html) |
| [file](http://www.darwinsys.com/file/) | :heavy_check_mark: | | [BSD-2-Clause](https://spdx.org/licenses/BSD-2-Clause.html) |
| [OpenSSL](https://www.openssl.org/) | :heavy_check_mark: | | [OpenSSL](https://spdx.org/licenses/OpenSSL.html) |
| [libcap](https://sites.google.com/site/fullycapable/) |                  | Linux only | [BSD-3-Clause](https://spdx.org/licenses/BSD-3-Clause.html) |
| [ICU](https://icu.unicode.org/) | :heavy_check_mark: | | [ICU](https://spdx.org/licenses/ICU.html) |
| [curl](https://curl.se/) | :heavy_check_mark: | | [MIT](https://spdx.org/licenses/MIT.html) |
| [ClamAV](https://www.clamav.net/) | :heavy_check_mark: | | [GPL-2.0-only](https://spdx.org/licenses/GPL-2.0-only.html) |
| [gettext](https://www.gnu.org/software/gettext/) | | .po to .mo | [GPL-3.0-or-later](https://spdx.org/licenses/GPL-3.0-or-later.html) |
| [libannocheck](https://sourceware.org/annobin/) | | | [GPL-3.0-or-later](https://spdx.org/licenses/GPL-3.0-or-later.html) |
| [cdson](https://github.com/frozencemetery/cdson) | :heavy_check_mark: | | [MPL-2.0](https://spdx.org/licenses/MPL-2.0.html) |
| [libtoml](https://github.com/ajwans/libtoml) | :heavy_check_mark: | | [BSD-3-Clause](https://spdx.org/licenses/BSD-3-Clause.html) |

Additionally, the unit test suite requires the following packages:

| Dependency | License |
| :--- | :--- |
| [CUnit](http://cunit.sourceforge.net/)| [LGPL-2.0-or-later](https://spdx.org/licenses/LGPL-2.0-or-later.html) |
| [Python 3](https://www.python.org/) | [Python-2.0](https://spdx.org/licenses/Python-2.0.html) |
| [rpmfluff](https://pagure.io/rpmfluff)| [GPL-2.0-only](https://spdx.org/licenses/GPL-2.0-only.html) |

Most distributions include the above projects in prebuilt and packaged
form.  If those are available, you should use those packages.


Runtime Requirements
--------------------

In addition to the libraries that will be linked in to librpminspect,
there are a number of userspace programs used:

    /usr/bin/desktop-file-validate [optional]
    /usr/bin/msgunfmt
    /usr/bin/abidiff [optional]
    /usr/bin/kmidiff [optional]
    /usr/bin/udevadm [optional]

The provided spec file template uses the Fedora locations for these
files, but in the program, they must be on the runtime system.
Different inspections use these tools at runtime. Most distributions
include the above tools. If they are available, you should use those
packages.

In Fedora, for example, you can run the following to install these
programs:

    yum install desktop-file-utils gettext libabigail /usr/bin/annocheck

The 'shellsyntax' inspection uses the actual shell programs listed in
the shells setting in the rpminspect configuration file.  Since this
can vary by system, you should make sure they are available in the
system PATH.  Or you can just not use the shellsyntax inspection.  The
spec file for rpminspect includes weak dependencies on the default
list of shells:

    yum install dash ksh zsh tcsh rc bash

An even easier option is to use the developer setup method described
in the next section.


Developer Setup
---------------

If you are planning on contributing to rpminspect, you should use the
developer setup target in the Makefile to make sure you have the required
packages installed:

    make instreqs

This target will install all BuildRequires, Requires, and Suggests
dependencies from the spec file template, but also the additional test
suite requirements listed in the files in the osdeps/ subdirectory.

If you are working on a distribution other than Fedora/RHEL/CentOS,
then have a look at the rpminspect.spec.in template for the packages
listed as Requires, BuildRequires, and Suggests.  These usually have
similar names on other distributions.  If you have instructions for
setting up a development and runtime environment on another
distribution, please send a pull request with the information.


Building
--------

If you got the source from a released tarball or from cloned from git,
you can do this:

    meson setup build
    ninja -C build

You can get verbose build output with "ninja -C build -v".

For more information on meson and ninja, see these sites:

    https://mesonbuild.com/
    https://ninja-build.org/


Testing
-------

Follow the steps for 'Building' above and then:

    meson test -C build

You can get verbose build output with:

    meson test -C build -v

The verbose mode is useful when tests fail because you can see the
debugging information dumps and other output.


Run and debug integration tests
-------------------------------
Follow the steps for 'Building' above.
Install pip and virtualenv for Python.

To prepare a virtual environment for testing simple run:

    virtualenv -p python3.8 my_test_env
    . my_test_env/bin/activate
    pip-3 install meson ninja rpm-py-installer rpmfluff setuptools
    export RPMINSPECT=$PWD/build/src/rpminspect
    export RPMINSPECT_YAML=$PWD/data/generic.yaml
    export RPMINSPECT_TEST_DATA_PATH=$PWD/test/data

To run all tests, execute the command:
    python3 -Bm unittest discover -v test/

To run a specific test suite, execute:
    python3 -Bm unittest discover -v test/ test_emptyrpm.py


Releasing
---------

The 'make release' command creates a new release.  The command handles
the version number change, tests, git tag, and repo push.  Release
entries on GitHub are created manually.  The signed archive is
attached manually to the release entry.

The 'make koji' command submits official builds for Fedora and EPEL.


How To Use
----------

### Setup

Make sure you have installed the program and corresponding data file
collection.  Fedora users can use yum:

    yum install rpminspect rpminspect-data-fedora

The first package is the actual program.  The rpminspect-data-fedora
package delivers data files used by the various inspections performed
by rpminspect.  It also delivers the rpminspect configuration file.
Have a look at this file and make sure it works for your environment.
The idea is to support per-vendor data packages efficiently.  For
example, if CentOS needs different settings, it should provide a
rpminspect-data-centos package.

Ideally, the configuration file should not need changing.  If you do
find mistakes, please file an issue or send a pull request to the
project:

    https://github.com/rpminspect/rpminspect-data-fedora

The aim should be the vendor-specific data packages providing
everything a developer needs to run rpminspect locally for that
product successfully.

### Usage

rpminspect runs from the command line.  The inputs must be local RPM
packages, a Koji build specification (NVR), a Koji module
specification, or a locally cached Koji build output (regular build or
module).  For inputs originating from Koji, rpminspect talks to Koji
and download the build artifacts.  For repeated runs, you may want to
cache a remote build locally to avoid downloading it with each run.

Remember that you will need the corresponding vendor data package for
rpminspect in addition to the rpminspect program.  For Fedora, you
will need to install 'rpminspect-data-fedora'.  You can then invoke
rpminspect using the 'rpminspect-fedora' script or 'rpminspect -c
/usr/share/rpminspect/fedora.yaml'.

Here is a simple invocation using tmux as an example:

    $ rpminspect-fedora -v tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

This just runs with verbose mode enabled and compares tmux-2.9a-2.fc31
to tmux-2.9a-3.fc31.  rpminspect downloads the packages for these
builds and runs the inspections.

If you want to keep temporary files created during the run, pass -k.
rpminspect tells you where those files are when it finishes.

You can list available inspections with the -l option:

    $ rpminspect-fedora -l

Say you only want to run the license inspection on the builds above:

    $ rpminspect-fedora -v -k -T license tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

Now let's say you want to run the license and manpage inspections:

    $ rpminspect-fedora -v -k -T license,manpage tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

And lastly, what if you want to run *all* inspections except the license
one:

    $ rpminspect-fedora -v -k -E license tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

What about specify a locally cached build?  First, let's start by caching
the builds we have been using:

    $ mkdir ~/builds
    $ rpminspect-fedora -v -w ~/builds -f tmux-2.9a-2.fc31
    $ rpminspect-fedora -v -w ~/builds -f tmux-2.9a-3.fc31

Now let's run all the inspections but specify the locally cached builds:

    $ rpminspect-fedora -v ~/builds/tmux-2.9a-2.fc31 ~/builds/tmux-2.9a-3.fc31

Easy.  Again, these locally cached builds must look like what rpminspect
would download from koji.  Hence using rpminspect first to download it.

rpminspect can also run inspections on local RPM packages.  Similar to Koji
inputs, you may specify a single RPM or two RPMs to compare.  For example:

    $ rpminspect-fedora -v ~/rpmbuild/RPMS/x86_64/tmux-2.9a-2.fc31.x86_64.rpm

Or:

    $ cd ~/rpmbuild/RPMS/x86_64
    $ rpminspect-fedora -v tmux-2.9a-2.fc31.x86_64.rpm tmux-2.9a-3.fc31.x86_64.rpm

All of the other command-line options that apply to Koji tests work for
local RPM packages.

For more information, see the man page for rpminspect(1).  And see the
--help output for information on command-line option syntax.


Contributing
------------

Patches are welcome, as are bug reports.  There is a lot to do in this
project.  Some things to keep in mind:

* Please follow the existing coding style in files you modify.  Things
  like variable and function naming, spacing, and indentation.  I want
  to avoid wildly varying coding styles throughout the tree.

* New inspections in librpminspect need to be in the form of an
  inspect_NAME.c file with a driver added to the main struct.  You may
  add static and non-static support functions to your inspect_NAME.c
  file and expose those as part of the librpminspect API.  If the
  support functions are generic enough, feel free to start a new
  source file.

* Test cases or updates to existing test cases need to accompany
  patches and new code submissions.

* Use the standard C library whenever possible.  Code using glib,
  libbsd or any other type of generic utility library is going to be
  reviewed and likely rejected until it is modified to use the
  standard C library.

* That said, inspections should make use of available libraries for
  performing their work.  When given the option between a library and
  forking and executing a program, please use the library.

* Alternative libraries intended to replace a build requirement are ok
  so long as meson.build and meson_options.txt are modified to
  conditionalize the choices.  For example:

      -Djson_lib=other_json_lib

  If this requires modification in the code, try to minimize the use
  of cpp macros.

* See the TODO file for a current list of things that need work.


Licensing and Copyright
-----------------------

The program (src/), the test suite (test/), and various developer
tools in this source tree are available under the GNU General Public
License version 3 or, at your option, any later version.

The librpminspect library (include/ and lib/) is available under the
GNU Lesser General Public License version 3 or, at your option, any
later version.

The data files (data/) are licensed under the Creative Commons
Attribution 4.0 International Public License.

Some source files in the project carry Apache License 2.0 licenses or
BSD licenses.  Both of these cases are allowed under the GPLv3 and
LGPLv3.  The combined work is licensed as described in the previous
paragraph.

Copyright statements are in the boilerplates of each source file.
