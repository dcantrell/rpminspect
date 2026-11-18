Development
===========


Build Requirements
------------------

A typical Linux_ system with a toolchain for building C_ software
using meson_ and ninja_, plus the following libraries:

+-------------+-----------------------------------------------------------+-------------------+
| Requirement | URL                                                       | License           |
+=============+===========================================================+===================+
| json-c      | http://json-c.github.io/json-c                            | MIT               |
+-------------+-----------------------------------------------------------+-------------------+
| xmlrpc-c    | http://xmlrpc-c.sourceforge.net/                          | BSD and MIT       |
+-------------+-----------------------------------------------------------+-------------------+
| libxml-2.0  | http://xmlsoft.org/                                       | MIT               |
+-------------+-----------------------------------------------------------+-------------------+
| rpm         | https://github.com/rpm-software-management/rpm            | GPL-2.0-or-later  |
+-------------+-----------------------------------------------------------+-------------------+
| libarchive  | https://www.libarchive.org/                               | BSD               |
+-------------+-----------------------------------------------------------+-------------------+
| elfutils    | https://sourceware.org/elfutils/                          | LGPL-3.0-or-later |
+-------------+-----------------------------------------------------------+-------------------+
| kmod        | https://git.kernel.org/pub/scm/utils/kernel/kmod/kmod.git | GPL-2.0-or-later  |
+-------------+-----------------------------------------------------------+-------------------+
| zlib        | https://www.zlib.net/                                     | Zlib and BSL-1.0  |
+-------------+-----------------------------------------------------------+-------------------+
| mandoc      | https://mandoc.bsd.lv/                                    | ISC               |
+-------------+-----------------------------------------------------------+-------------------+
| libyaml     | https://github.com/yaml/libyaml                           | MIT               |
+-------------+-----------------------------------------------------------+-------------------+
| file        | http://www.darwinsys.com/file/                            | BSD               |
+-------------+-----------------------------------------------------------+-------------------+
| OpenSSL     | https://www.openssl.org/                                  | OpenSSL           |
+-------------+-----------------------------------------------------------+-------------------+
| libcap      | https://sites.google.com/site/fullycapable/               | BSD-3-Clause      |
+-------------+-----------------------------------------------------------+-------------------+

Additionally, the unit test suite requires the following packages:

+-------------+-----------------------------------------------------------+-------------------+
| Requirement | URL                                                       | License           |
+=============+===========================================================+===================+
| CUnit       | http://cunit.sourceforge.net/                             | LGPL-2.0-or-later |
+-------------+-----------------------------------------------------------+-------------------+

Most distributions include the above projects in prebuilt and packaged
form.  If those are available, you should use those packages.


Developer Setup
---------------

If you are planning on contributing to rpminspect_, you should use the
developer setup target in the Makefile to make sure you have the
required packages installed::

    make instreqs

This target will install all BuildRequires, Requires, and Suggests
dependencies from the spec file template, but also the additional test
suite requirements listed in the files in the ``osdeps/`` subdirectory.

If you are working on a distribution other than Fedora/RHEL/CentOS,
then have a look at the ``rpminspect.spec.in`` template for the
packages listed as Requires, BuildRequires, and Suggests.  These
usually have similar names on other distributions.  If you have
instructions for setting up a development and runtime environment on
another distribution, please send a pull request with the information.


Building
--------

If you got the source from a released tarball or from cloned from
git_, you can do this::

    meson setup build
    ninja -C build

You can get verbose build output with ``ninja -C build -v``.

.. note::
    On CentOS 7.x and RHEL 7.x systems, the ninja command is named
    ``ninja-build`` and installed in ``/usr/bin``.


Contributing
------------

Patches are welcome, as are bug reports.  There is a lot to do in this
project.  Some things to keep in mind:

- Please follow the existing coding style in files you modify.  Things
  like variable and function naming, spacing, and indentation.  I want
  to avoid wildly varying coding styles throughout the tree.

- New inspections in librpminspect need to be in the form of an
  ``inspect_NAME.c`` file with a driver added to the main struct.  You
  may add static and non-static support functions to your
  ``inspect_NAME.c`` file and expose those as part of the
  librpminspect API.  If the support functions are generic enough,
  feel free to start a new source file.

- Test cases or updates to existing test cases need to accompany
  patches and new code submissions.

- Use the standard C library whenever possible.  Code using glib,
  libbsd or any other type of generic utility library is going to be
  reviewed and likely rejected until it is modified to use the
  standard C library.

- That said, inspections should make use of available libraries for
  performing their work.  When given the option between a library and
  forking and executing a program, please use the library.

- Alternative libraries intended to replace a build requirement are ok
  so long as ``meson.build`` and ``meson_options.txt`` are modified to
  conditionalize the choices.  For example::

      -Djson_lib=other_json_lib

  If this requires modification in the code, try to minimize the use
  of C preprocessor macros.

- See the TODO file for a current list of things that need work.


Licensing and Copyright
-----------------------

The program (``src/``), the test suite (``test/``), and various
developer tools in this source tree are available under the GNU
General Public License version 3 or, at your option, any later
version.

The librpminspect library (``include/`` and ``lib/``) is available
under the GNU Lesser General Public License version 3 or, at your
option, any later version.

The data files (``data/``) are licensed under the Creative Commons
Attribution 4.0 International Public License.

Some source files in the project carry Apache License 2.0 licenses or
BSD licenses.  Both of these cases are allowed under the GPLv3 and
LGPLv3.  The combined work is licensed as described in the previous
paragraph.

Copyright statements are in the boilerplates of each source file.

.. _meson: https://mesonbuild.com/

.. _ninja: https://ninja-build.org/

.. _Linux: https://www.kernel.org/

.. _C: https://en.wikipedia.org/wiki/C_(programming_language)

.. _rpminspect: https://github.com/dcantrell/rpminspect

.. _git: https://git-scm.com/
