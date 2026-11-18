Introduction
============


Goals
-----

rpminspect_ is an RPM_ policy enforcing and deviation analysis tool.
It looks at the output of an RPM_ build (e.g., the output of rpmbuild
or a build system like Koji_) and examines the contents of the build
artifacts to report:

* Policy compliance

* Changes from a previous build to the current build

* General correctness and best practices

rpminspect_ is the frontend tool, and librpminspect provides the
inspection engine and inspections.  The program is configured through
a configuration file and runtime data provided by a vendor-specific
rpminspect data package.  The **rpminspect-data-generic** package in
this source tree provides a template for constructing the
vendor-specific data package.  For example, in `Fedora Linux
<https://getfedora.org>`_ the rpminspect-data-fedora_ package provides
configuration files and runtime data for rpminspect_ when checking
`Fedora Linux <https://getfedora.org>`_ RPM_ packages.


Build Types Support
-------------------

rpminspect_ expects its input to either be local RPM_ packages or a
Koji_ build.  Koji produces certain types of builds that are not
supported by rpminspect.  Right now, the following input types are
supported:

- Local RPM_ packages (source and binary)

- Koji_ builds (i.e., a source RPM_ run with 'rpmbuild -ba' on all
  specified architectures)

- Modules_

If comparing local RPM_ packages, rpminspect assumes the before and
after specifications are peers whereas for a Koji_ build, rpminspect
matches peer packages (e.g., in the gcc package, gcc-gfortran in the
before build is peered with gcc-gfortran in the after build).


Intended Audience
-----------------

Developers, QE, release engineers, and system administrators who
regularly build RPM_ packages. for use in some environments or
products.

From an individual user standpoint, rpminspect_ is a command-line tool
you can use as a linter of sorts.  rpminspect_ reports, and that's it.
rpminspect_ can output information in JSON_ or xUnit_ format, which
makes it easier to integrate with automated workflows or web
frontends.

The reporting uses certain classifications for different things found,
but it up to the end-user to determine what to do with that
information.  If used with an external build tool, the JSON_ or xUnit_
data may be more useful as you can construct decision making around
those results.

.. _rpminspect: https://github.com/dcantrell/rpminspect

.. _Koji: https://pagure.io/koji/

.. _rpminspect-data-fedora: https://github.com/dcantrell/rpminspect-data-fedora

.. _RPM: https://rpm-packaging-guide.github.io/

.. _Modules: https://docs.fedoraproject.org/en-US/modularity/

.. _JSON: https://www.json.org

.. _xUnit: https://xunit.net
