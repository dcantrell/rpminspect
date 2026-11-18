Usage
=====

Setup
-----

Make sure you have installed the program and corresponding data file
collection.  `Fedora Linux <https://getfedora.org>`_ users can use
dnf_::

    dnf install rpminspect rpminspect-data-fedora

The first package is the actual program.  The rpminspect-data-fedora_
package delivers data files used by the various inspections performed
by rpminspect_.  It also delivers the rpminspect_ configuration file.
Have a look at this file and make sure it works for your environment.
The idea is to support per-vendor data packages efficiently.  For
example, if CentOS_ needs different settings, it should provide a
**rpminspect-data-centos** package.

Ideally, the configuration file should not need changing.  If you do
find mistakes, please file an issue or send a pull request to the
project:

    https://github.com/dcantrell/rpminspect-data-fedora

The aim should be the vendor-specific data packages providing
everything a developer needs to run rpminspect_ locally for that
product successfully.


Input Types
-----------

rpminspect_ looks at files contained in RPM_ packages.  But RPM_
packages have a lot of infrastructure built up around them too.  For
starters, a source RPM_ can generate multiple binary RPMs.  Second,
Linux distributions tend to build for multiple architectures.  The
collection of all of the built RPMs, or artifacts, is collectively
referred to as a *build*.  `Fedora Linux <https://getfedora.org>`_
tends to refer to a build using the *name-version-release*, or NVR,
syntax.  For example, **glibc-2.33-8.fc34**.  Here the name is
**glibc**, the version is **2.33**, and the release is **8.fc34**.

rpminspect_ requires at least an RPM_ of some sort as input.  However,
since it is very common for RPM_ maintenance to deal with all of the
built RPMs and all applicable architectures, rpminspect_ can take a
complete build as input.  In `Fedora Linux <https://getfedora.org>`_,
when you specify an *NVR* build rpminspect_ will query Koji_ for
information on that build and if it's found, download all of the RPM_
packages.  You may also cache these Koji_ builds locally and refer to
them by their filesystem path, in which case rpminspect_ will not go
and download the build again.

If you are performing package work locally and just want to look at a
single RPM_, you can specify that as well.  Just pass the path to the
RPM_ on the command line.  There are other RPM_ build systems out
there and it would be interesting to see rpminspect_ expand to gain
support for fetching builds from them.


Modes
-----

rpminspect_ can operate in one of two modes when processing RPM_
packages.  The first is called **analysis mode**.  This mode is where
you provide a single build or RPM package as the input.  rpminspect_
will check that single input and perform all of the inspections that
apply to single inputs only.

The second mode is a **comparison mode**.  This mode performs all of
the analysis mode checks but also performs checks that can happen when
there is a before and after build to compare.  These checks cover
things like reporting when things change between builds.  The analysis
mode is more checks that enforce policies on built packages.

The mode cannot be specified via a command line option.  The mode of
operation is implied based on provided one or two builds as input on
the command line.


Reporting Levels
----------------

rpminspect will exit with code 0 indicating success if all of the
results reported are either at the **INFO** or **OK** level.  Anything
that is **VERIFY** or **BAD** will trigger an exit code of 1
indicating a failure.  The failure reporting threshold can be adjusted
with the ``-t`` option.  The default failure threshold is **VERIFY**.
The ``-s`` option allows you to specify a reporting level for results
suppression.  That is, anything below the specified level will be
suppressed in the output.  For example, ``-s VERIFY`` will suppress
all results below the **VERIFY** level.

The results also contain a value indicating who is the appropriate
entity or party to waive a particular result.  This information can be
changed by sending pull requests to the project.  It has no bearing on
the exit code of 0 and is only present for use by environments
integrating rpminspect_ in to their workflows.


Workflow Examples
-----------------

rpminspect_ runs from the command line.  The inputs must be local RPM_
packages, a Koji_ build specification (*NVR*), a Koji_ task number, a
Koji_ module specification, or a locally cached Koji_ build output
(regular build or module).  For inputs originating from Koji_,
rpminspect_ talks to Koji_ and download the build artifacts.  For
repeated runs, you may want to cache a remote build locally to avoid
downloading it with each run.  The examples below use `Fedora Linux
<https://getfedora.org>`_, so they will reference the
**rpminspect-fedora** wrapper script provided by
rpminspect-data-fedora_ as the command to run.

Here is a simple invocation using **tmux** as an example::

    $ rpminspect-fedora -v tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

This just runs with verbose mode enabled and compares
**tmux-2.9a-2.fc31** to **tmux-2.9a-3.fc31**.  rpminspect_ downloads
the packages for these builds and runs the inspections.

If you want to keep temporary files created during the run, pass
``-k``.  rpminspect_ tells you where those files are when it finishes.

You can list available inspections with the ``-l`` option::

    $ rpminspect-fedora -l

Say you only want to run the license inspection on the builds above::

    $ rpminspect-fedora -v -k -T license tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

Now let's say you want to run the license and manpage inspections::

    $ rpminspect-fedora -v -k -T license,manpage tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

And lastly, what if you want to run *all* inspections except the
license one::

    $ rpminspect-fedora -v -k -E license tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

What about specify a locally cached build?  First, let's start by
caching the builds we have been using::

    $ mkdir ~/builds
    $ rpminspect-fedora -v -w ~/builds -f tmux-2.9a-2.fc31 tmux-2.9a-3.fc31

Now let's run all the inspections but specify the locally cached
builds::

    $ rpminspect-fedora -v ~/builds/tmux-2.9a-2.fc31 ~/builds/tmux-2.9a-3.fc31

Easy.  Again, these locally cached builds must look like what
rpminspect_ would download from Koji_.  Hence using rpminspect_ first
to download it.

rpminspect_ can also run inspections on local RPM_ packages.  Similar
to Koji_ inputs, you may specify a single RPM_ or two RPM_ packages to
compare.  For example::

    $ rpminspect-fedora -v ~/rpmbuild/RPMS/x86_64/tmux-2.9a-2.fc31.x86_64.rpm

Or::

    $ cd ~/rpmbuild/RPMS/x86_64
    $ rpminspect-fedora -v tmux-2.9a-2.fc31.x86_64.rpm tmux-2.9a-3.fc31.x86_64.rpm

All of the other command-line options that apply to Koji_ tests work for
local RPM_ packages.

For more information, see the man page for ``rpminspect(1)``.  And see
the ``--help`` output for information on command-line option syntax.


Command Line Options
--------------------

Compare package builds for policy compliance and consistency.

Usage: ``rpminspect [OPTIONS] [before build] [after build]``

Options:

-c FILE, --config=FILE   Configuration file to use
-p NAME, --profile=NAME  Configuration profile to use
-T LIST, --tests=LIST    List of tests to run (default: ALL)
-E LIST, --exclude=LIST  List of tests to exclude (default: none)
-a LIST, --arches=LIST   List of architectures to check
-r STR, --release=STR    Product release string
-n, --no-rebase          Disable build rebase detection
-o FILE, --output=FILE   Write results to FILE (default: stdout)
-F TYPE, --format=TYPE   Format output results as TYPE (default: text)
-t TAG, --threshold=TAG  Result threshold triggering exit failure (default: **VERIFY**)
-s TAG, --suppress=TAG   Results suppression threshold (default: off, report everything)
-l, --list               List available tests and formats
-w PATH, --workdir=PATH  Temporary directory to use (default: ``/var/tmp/rpminspect``)
-f, --fetch-only         Fetch builds only, do not perform inspections (implies ``-k``)
-k, --keep               Do not remove the comparison working files
-d, --debug              Debugging mode output
-D, --dump-config        Dump configuration settings used (in YAML_ format)
-v, --verbose            Verbose inspection output when finished, display full path
-?, --help               Display usage information
-V, --version            Display program version

See the ``rpminspect(1)`` man page for more information.



Available Inspections
---------------------

**Analysis Mode**

These inspections run when a single build is provided as input to the
program.

- **license**

    Verify the string specified in the License tag of the RPM_
    metadata describes permissible software licenses as defined by the
    license database. Also checks to see if the License tag contains
    any unprofessional words as defined in the configuration file.

- **emptyrpm**

    Check all binary RPM_ packages in the build for any empty
    payloads. When comparing two builds, report new packages in the
    after build with empty payloads.  Packages that only carry %ghost
    entries are reported at the INFO level.

- **metadata**

    Perform some RPM_ header checks. First, check that the Vendor
    contains the expected string as defined in the configuration
    file. Second, check that the build host is in the expected
    subdomain as defined in the configuration file. Third, check the
    Summary string for any unprofessional words. Fourth, check the
    Description for any unprofessional words. Lastly, if there is a
    before build specified, check for differences between the before
    and after build values of the previous RPM_ header values and
    report them.

- **manpage**

    Perform some checks on man pages in the RPM_ payload. First, check
    that each man page is compressed. Second, check that each man page
    contains valid content. Lastly, check that each man page is
    installed to the correct path.

- **xml**

    Check that XML files included in the RPM_ payload are well-formed.

- **elf**

    Perform several checks on ELF_ files. First, check that ELF_
    objects do not contain an executable stack. Second, check that
    ELF_ objects do not contain text relocations. When comparing
    builds, check that the ELF_ objects in the after build did not
    lose a ``PT_GNU_RELRO`` segment. When comparing builds, check that
    the ELF_ objects in the after build did not lose
    ``-D_FORTIFY_SOURCE``.

- **desktop**

    Perform syntax and file reference checks on ``*.desktop``
    files. Syntax errors and invalid file references are reported as
    errors.

- **disttag**

    Check that the Release tag in the RPM_ spec file includes the
    ``%{?dist}`` directive.

- **specname**

    Ensure the spec file name conforms to the *NAME.spec* naming
    format.

- **modularity**

    Ensure compliance with modularity build and packaging policies
    (only valid for module builds, no-op otherwise).

- **javabytecode**

    Check minimum required Java_ bytecode version in class files,
    report bytecode version changes between builds, and report if
    bytecode versions are exceeded. The bytecode version is vendor
    specific to releases and defined in the configuration file.

- **addedfiles**

    Report added files from the before build to the after
    build. Debuginfo files are ignored as are files that match the
    patterns defined in the configuration file. Files added to
    security paths generate special reporting in case a security
    review is required. New setuid and setgid files raise a security
    warning unless the file is in the whitelist.

- **ownership**

    Report files and directories owned by unexpected users and
    groups. Check to make sure executables are owned by the correct
    user and group. If a before and after build have been specified,
    also report ownership changes.

- **shellsyntax**

    For all shell scripts in the build, perform a syntax check on it
    using the shell defined in its #! line (shell must also be listed
    in shell section of the configuration data). If the syntax check
    returns non-zero, report it to the user and return a combined
    stdout and stderr. If comparing two builds, perform the previous
    check but also report if a previously bad script is now passing
    the syntax check.

- **annocheck**

    Perform annocheck tests defined in the configuration file on all
    ELF_ files in the build. A single build specified will perform an
    analysis only. Two builds specified will compare the test results
    between the before and after build. If no annocheck tests are
    defined in the configuration file, this inspection is skipped.

- **permissions**

    Report ``stat(2)`` mode changes between builds. Checks against the
    fileinfo lists for the product release specified or
    determined. Any setuid or setgid changes will raise a message
    indicating a security team should review it.

- **capabilities**

    Report ``capabilities(7)`` changes between builds. Checks against
    the capabilities list for the product release specified or
    determined. Any capabilities changes not on the list will raise a
    message indicating a security team should review the change.  This
    inspection is only present if rpminspect was built with libcap
    support.

- **pathmigration**

    Report files that are packaged in paths that have migrated to new
    locations. For example, packages should not package anything
    directly in ``/bin`` but rather ``/usr/bin``. The path migrations
    are defined in the **rpminspect.yaml** file.

- **lto**

    Link Time Optimization (LTO) produces smaller and faster shared ELF_
    executables and libraries. LTO bytecode is not stable from one release
    of gcc_ to the next. As such, LTO bytecode should not be present in
    ``.a`` and ``.o`` ELF_ objects shipped in packages. This inspection
    looks for LTO bytecode in ELF_ relocatable objects and reports if any
    is present.

- **symlinks**

    Symbolic links must be resolvable on the installed system. This
    inspection ensures absolute and relative symlinks are valid. It
    also checks for any symlink usage that will cause problems for
    RPM_.

- **files**

    Reads each ``%files`` section in the spec file and looks for any
    forbidden path references. Forbidden path references are defined in
    the configuration file under the **files:forbidden_paths** section. An
    example might be reporting spec files that use ``/usr/lib`` rather
    than ``%{_libdir}``.

- **patches**

    Report patches defined in the spec file that are under 4 bytes as
    invalid patch files. Report the percentage by which patches change
    between builds. Report how many lines are touched by a
    patch. Based on size and line count thresholds in the
    configuration file, report findings at either the INFO or VERIFY
    level.

- **virus**

    Check every file in the after build for viruses using
    libclamav_. Any positive result is reported as a BAD result.

- **politics**

    Check for possible politically sensitive files in packages. The
    things to check for are defined in the per-product release files
    in ``/usr/share/rpminspect/politics`` that are provided by the
    vendor data package.  This inspection was originally introduced to
    catch potentially political names or phrases in filenames.

- **badfuncs**

    Check for forbidden functions in ELF_ files. Forbidden functions
    are defined in the runtime configuration files. Usually this
    inspection is used to catch built packages that make use of
    deprecated API functions if you wish built packages to conform to
    replacement APIs.

- **runpath**

    Check for forbidden paths in both the ``DT_RPATH`` and ``DT_RUNPATH``
    settings in ELF_ shared objects. If both ``DT_RPATH`` and
    ``DT_RUNPATH`` are found in an ELF_ object, report it as a BAD result
    since that would be a linker error.

- **unicode**

    Scan extracted and patched source code files, scripts, and RPM_
    spec files for any prohibited Unicode code points, as defined in
    the configuration file.  Any prohibited code points are reported
    as a possible security risk.

- **rpmdeps**

    Check for correct RPM_ dependency metadata.  Report incorrect or
    conflicting findings as well as expected changes when comparing a
    new build to an older build.  Changes are only reported when
    comparing builds, but this inspection will check for correct RPM
    dependency metadata when inspecting a single build and report
    findings.  Gained, changed, and lost dependencies in rebased
    packages are reported at the INFO level by default.  The
    rebase_reporting_level setting in the rpmdeps section of the
    configuration file changes this level.  Non-rebased packages
    report these changes at the VERIFY level.  Retained dependencies
    and dependencies on subpackages of the same build are always
    reported at the INFO level.  Dependencies with a version that
    follows the package version are also always reported at the
    INFO level.  Other expected changes in non-rebased packages are
    reported at the INFO level.  Subpackages that depend on a shared
    library from another subpackage must be held at the same
    version-release.  An explicit Requires on the exact
    version-release of a subpackage holds both subpackages together.
    The Requires may be in the subpackage using the library, in the
    subpackage providing the library, or in other subpackages that
    link the two.

- **udevrules**

    Perform syntax check on udev rules files using udevadm verify.

**Comparison Mode**

These inspections run when a before and after build are specified as
the input to the program.  All of the analysis mode inspections also
run when in comparison mode.

- **lostpayload**

    Check all binary RPM_ packages in the before and after builds for
    any empty payloads. Packages that lost payload data from the
    before build to the after build are reported.  Packages that only
    carry %ghost entries and had no payload in the before build are
    not reported.

- **changedfiles**

    Report changed files from the before build to the after
    build. Certain file changes will raise additional warnings if the
    concern is more critical than just reporting changes (e.g., a
    suspected security impact). Any gzip, bzip2, or xz compressed
    files will have their uncompressed content compared only, which
    will allow changes through in the compression level used. Message
    catalog files (.mo) are unpacked and compared.  Public C and C++
    header files are preprocessed and compared. Any changes with
    unified diff output are included in the results.

- **movedfiles**

    Report files that have moved installation paths or across
    subpackages between builds. Files moved with a security path
    prefix generate special reporting in case a security review is
    required. Rebased packaged report these findings at the INFO level
    while non-rebased packages report them at the VERIFY level or
    higher.

- **removedfiles**

    Report removed files from the before build to the after
    build. Shared libraries with a SONAME get additional reporting
    output as they may be unexpected dependency removals. Files removed with a
    security path prefix generated special reporting in case a
    security review is required. Source RPM_ packages and debuginfo
    files are ignored by this inspection.

- **upstream**

    Report Source archives defined in the RPM_ spec file changing
    content between the before and after build. If the source archives
    change and the package is on the rebaseable list, the change is
    reported as informational. Otherwise the change is reported as a
    rebase of the package and requires inspection.

- **dsodeps**

    Compare ``DT_NEEDED`` entries in dynamic ELF_ executables and
    shared libraries between the before and after build and report
    changes.

- **filesize**

    Report file size changes between builds. If empty files became
    non-empty or non-empty files became empty, report those as results
    needing verification. Report file change percentages as info-only.

- **kmod**

    Report kernel module parameter, dependency, PCI ID, or symbol
    differences between builds. Added and removed parameters are
    reported and if the package version is unchanged, these messages
    are reported as failures. The same is true module dependencies,
    PCI IDs, and symbols. This inspection is only available is
    rpminspect was built with libkmod support.

- **arch**

    Report RPM_ architectures that appear and disappear between the before
    and after builds.

- **subpackages**

    Report RPM_ subpackages that appear and disappear between the before
    and after builds.

- **changelog**

    Ensure packages contain an entry in the ``%changelog`` for the
    version built. Reports any other differences in the existing
    changelog between builds and that the new entry contains new text
    entries.

- **types**

    Compare MIME_ types of files between builds and report any changes for
    verification.

- **abidiff**

    When comparing two builds or two packages, compare ELF_ files
    using ``abidiff(1)`` from the libabigail_ project. Differences are
    reported. If the package is a rebase and not on the rebaseable
    list and the rebase inspection is enabled, ABI differences are
    reported as failures. The assumption here is that rpminspect_ is
    comparing builds for maintenance purposes and you do not want to
    introduce any ABI changes for users. If you do not care about
    that, turn off the abidiff inspection or add the package name to
    the rebaseable list.

- **kmidiff**

    kmidiff compares the binary kernel Module Interfaces of two Linux_
    kernel trees. The binary KMI is the interface that the Linux_
    kernel exposes to its modules. The trees we are interested in here
    are the result of the build of the Linux_ kernel source tree. If
    the builds compared are not considered a rebase, an incompatible
    change reported by ``kmidiff(1)`` is reported for verification.

- **config**

    Report ``%config`` files changing to/from ``%config`` status
    between the before and after builds. Report whitespace only
    changes as INFO messages and report content changes as VERIFY
    messages unless the comparison is for a rebased package in which
    case the INFO message level is used. For ``%config`` files that
    are symlinks, compare the link destinations and report changes
    using the reporting levels just mentioned.

- **doc**

    Report ``%doc`` files changing to/from ``%doc`` status between the
    before and after builds. These messages are at the INFO level for
    rebased builds and the VERIFY level otherwise. The main objective
    here is to catch projects that may rename documentation files
    (e.g., README to README.md) in a minor update that the package
    maintainer might overlook.

.. _rpminspect: https://github.com/dcantrell/rpminspect

.. _rpminspect-data-fedora: https://github.com/dcantrell/rpminspect-data-fedora

.. _dnf: https://github.com/rpm-software-management/dnf

.. _CentOS: https://www.centos.org/

.. _RPM: https://rpm-packaging-guide.github.io/

.. _Koji: https://pagure.io/koji/

.. _YAML: https://yaml.org/

.. _Linux: https://www.kernel.org/

.. _ELF: https://en.wikipedia.org/wiki/Executable_and_Linkable_Format

.. _Java: https://www.java.com/

.. _gcc: https://gcc.gnu.org/

.. _libclamav: https://www.clamav.net/

.. _MIME: https://en.wikipedia.org/wiki/MIME

.. _libabigail: https://sourceware.org/libabigail/
