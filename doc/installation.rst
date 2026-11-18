Installation
============


Program and Vendor Configuration
--------------------------------

rpminspect_ works directly with RPM_ packages, so the focus is on
RPM-based Linux distributions.  The software does build and run on
non-RPM distributions if all of the build requirements are available.

The easiest way to rpminspect_ is to use your distribution's packaging
system.  In `Fedora Linux <https://getfedora.org>`_, you can use dnf_
to install rpminspect_ and the required dependencies::

    dnf install rpminspect

That command will install the program, library, and required
dependencies.  The other important package you will need is your
vendor's data package for rpminspect_.  Using `Fedora Linux
<https://getfedora.org>`_ as an example, you will need to install
rpminspect-data-fedora_::

    dnf install rpminspect-data-fedora

If you have reported a bug to the rpminspect_ project and are asked to
test a development snapshot, you can do that by enabling the Copr_
repository for rpminspect_ and using the same dnf_ commands.
Instructions on where to find the Copr repository are in the project
README file in the rpminspect source tree.

The above instructions also apply to CentOS_ and `Red Hat Enterprise
Linux
<https://www.redhat.com/en/technologies/linux-platforms/enterprise-linux>`_.
Those products will have different vendor data packages, so be sure to
install the one that applies to the environment your packages are
built for.  You can be running `Fedora Linux <https://getfedora.org>`_
and run rpminspect_ on packages built for CentOS_.  All you need to do
is make sure you have the vendor data package installed for the target
environment.

If you are installing rpminspect_ on another distribution and it is
available a prebuilt package, please let us know so the documentation
here can be updated.


Runtime Requirements
--------------------

NOTE: This information is provided as an explanation of how to
manually install the typical runtime requirements for rpminspect_.
The vendor data package for rpminspect_, such as
rpminspect-data-fedora_, should carry dependencies on any optional
requirements necessary.  In most cases, it is sufficient to install
the rpminspect package and the appropriate vendor data package.

In addition to the libraries that will be linked in to librpminspect,
there are a number of userspace programs used:

+--------------------------------+-----------+---------------------------------------------------------------+
| Executable                     | Required? | Project                                                       |
+================================+===========+===============================================================+
| /usr/bin/desktop-file-validate | No        | https://www.freedesktop.org/wiki/Software/desktop-file-utils/ |
+--------------------------------+-----------+---------------------------------------------------------------+
| /usr/bin/msgunfmt              | **Yes**   | https://www.gnu.org/software/gettext/                         |
+--------------------------------+-----------+---------------------------------------------------------------+
| /usr/bin/abidiff               | No        | https://sourceware.org/libabigail/                            |
+--------------------------------+-----------+---------------------------------------------------------------+
| /usr/bin/kmidiff               | No        | https://sourceware.org/libabigail/                            |
+--------------------------------+-----------+---------------------------------------------------------------+
| /usr/bin/annocheck             | No        | https://sourceware.org/git/annobin.git                        |
+--------------------------------+-----------+---------------------------------------------------------------+

The provided RPM_ spec file template uses the `Fedora Linux
<https://getfedora.org>`_ locations for these files, but in the
program, they must be on the runtime system.  Different inspections
use these tools at runtime. Most distributions include the above
tools. If they are available, you should use those packages.

In `Fedora Linux <https://getfedora.org>`_, for example, you can run
the following to install these programs::

    dnf install desktop-file-utils gettext libabigail /usr/bin/annocheck

The *shellsyntax* inspection uses the actual shell programs listed in
the shells setting in the rpminspect_ configuration file.  Since this
can vary by system, you should make sure they are available in the
system PATH.  Or you can just not use the shellsyntax inspection.  The
RPM_ spec file for rpminspect_ includes weak dependencies on the
default list of shells::

    dnf install dash ksh zsh tcsh rc bash

An even easier option is to use the developer setup method described
in the next section.


Help for non-RPM Distributions
------------------------------

The osdeps/ directory contains scripts and package listings to set up
different Linux distributions so they can build rpminspect_ and run
the test suite.  You may find the information in here helpful if you
are trying to use rpminspect_ on a non-RPM Linux distribution.


.. _rpminspect: https://github.com/dcantrell/rpminspect

.. _rpminspect-data-fedora: https://github.com/dcantrell/rpminspect-data-fedora

.. _RPM: https://rpm-packaging-guide.github.io/

.. _CentOS: https://www.centos.org/

.. _dnf: https://github.com/rpm-software-management/dnf

.. _Copr: https://copr.fedorainfracloud.org/
