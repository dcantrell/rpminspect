Configuration
=============


Main Configuration
------------------

rpminspect_ requires configuration and runtime data in order to
perform the various checks on RPM_ packages.  At the very least you
will need to supply a main configuration file to rpminspect_.  This
main configuration file is supplied by the vendor data package, such
as rpminspect-data-fedora_ on `Fedora Linux <https://getfedora.org>`_.
The file is installed in **/usr/share/rpminspect** and usually matches
the product name and ends with **.yaml**.  In rpminspect-data-fedora_
you will find **/usr/share/rpminspect/fedora.yaml**.  This
configuration contains the settings that direct rpminspect to the
other runtime data in **/usr/share/rpminspect**.

.. note::
   You always need to tell rpminspect_ what main configuration file to
   use with the ``-c`` option.  For example, when checking a `Fedora
   Linux <https://getfedora.org>`_ build, your command line must begin
   with::

       rpminspect -c /usr/share/rpminspect/fedora.yaml

   To make this easier on users, the vendor data packages usually
   include a wrapper script to do this for you.  With
   rpminspect-data-fedora_ installed, you can run::

       rpminspect-fedora

   Instead of the longer command.


Product Releases
----------------

The program needs to know the product release when performing
inspections.  rpminspect_ uses the *dist tag* value to determine the
product release. If you are comparing builds, the product releases
must match, otherwise rpminspect_ will ask you to clarify which
product you want to use for the inspection. Vendors can change
policies as products evolve so the rules defined in the vendor data
package for one product may be different from another, and so on.

Because dist tags vary, rpminspect_ can have a set of regular
expressions to match those strings to products for the purposes of
inspections. For example, in `Fedora Linux <https://getfedora.org>`_
the product release string for `Fedora Linux <https://getfedora.org>`_
35 builds is **fc35** but that includes any dist tag that matches the
**^\.fc35.$** regular expression. These expressions are defined in the
**fedora.yaml** file in the rpminspect-data-fedora_ package.

.. note::
   It is important to note that the dist tag value matches what
   product environment you are building for, not necessarily what
   product your package is part of. For example, add on packages for
   `Fedora Linux <https://getfedora.org>`_ 35 are considered part of
   the **fc35** product environment in rpminspect_. It is important to
   keep this in mind if you are building a third party repository of
   packages intended for a particular vendor product.

In cases where rpminspect_ cannot determine a product release, you can
always pass the ``-r`` option, such as ``-r fc35``. This forces
rpminspect_ to use the **fc35** product rules.

Looking in the rpminspect-data-fedora_ package, you will see the data
files in there map to product release strings. Product release strings
allow rpminspect_ to find per-product rules in the subdirectories in
**/usr/share/rpminspect**.



Profiles
--------

The next level of configuration available are profiles.  These exist
in **/usr/share/rpminspect/profiles/VENDOR**.  They can contain any of
the settings available in the main configuration file.  The intent of
profiles is to create further configuration customizations for
categories of builds.  For example, all kernel builds could use the
*kernel* profile.  That starts by loading the main configuration file
then reading in the settings from the kernel profile.  The kernel
profile could then turn off inspections that do not apply to kernel
builds or adjust other settings from the main configuration file.

Profiles are effectively free form.  You can create any profiles you
want.  To share them across the product, profiles should be
coordinated and grouped together in the vendor data package.  In
`Fedora Linux <https://getfedora.org>`_ we have the
rpminspect-data-fedora_ package and any developer can submit a pull
request to this project to add to the configuration data.  You can add
or modify profiles this way too.

To use a profile, specify the name with the ``-p`` option.  For
example::

    rpminspect-fedora -p kernel


rpminspect.yaml
---------------

The last configuration option available to users, and probably the
most common, will be the **rpminspect.yaml** file you can place in the
current directory.  This file lets you adjust configuration values for
your rpminspect_ run.  The **rpminspect.yaml** file can contain any
settings available in the main configuration file.  Common uses of
this file are to set ignore lists per inspection.

Within `Fedora Linux <https://getfedora.org>`_ these files are
committed to the git repository for the package on the applicable
release branch.  In `Fedora CI
<https://docs.fedoraproject.org/en-US/ci/>`_ these files are read by
the rpminspect_ job.  As the package maintainer you can control how
rpminspect_ runs for your builds by adjusting this file in the git
repository for the package.

Here are some examples:

- The package provides no desktop files. You can disable the *desktop*
  inspection by creating an **rpminspect.yaml** file that contains::

      ---
      inspections:
          desktop: off

- The package contains no Java **.class** files. You can disable the
  *javabytecode* inspection by creating an **rpminspect.yaml** file that
  contains::

      ---
      inspections:
          javabytecode: off

- The *filesize* inspection is set too low and reports too many
  changes. The default is 20%, but you want to increase it to 47% so
  rpminspect_ will not report file size changes between builds that are
  under that percentage. You can create an **rpminspect.yaml** file that
  contains::

      ---
      filesize:
          size_threshold: 47

- The package needs to allow **/usr/lib64/systemd** DT_RPATH values. You can
  add the path to the list of allowed DT_RPATH values by creating an
  **rpminspect.yaml** file that contains::

      ---
      runpath:
          allowed_paths:
              - /usr/lib64/systemd

A full example is provided in the rpminspect_ source tree under the
name **data/generic.yaml** (latest upstream one is available
here_). You should also check the main configuration in the vendor
data package to see what the setting is for that product and then
adjust accordingly in your local configuration file.


Runtime Data
------------

Certain inspections pull their rules from runtime data files in
**/usr/share/rpminspect**.  These files map to the product release
string and can be found in subdirectories in
**/usr/share/rpminspect**.  For an example of what can go in each
file, see the corresponding files in the **data/** subdirectory of the
rpminspect_ source tree.

Here is a description of the subdirectories found in
**/usr/share/rpminspect**:

- **abi/**

    Per-product files (e.g., **el8**) defining the ABI compatibility
    levels and what packages belong to each one.

- **capabilities/**

    The capabilities list. A list consisting of 4 whitespace delimited
    fields. The first field is the package name, second field is the
    installed file path, third field is always **=**, and the fourth
    field is a comma-delimited list of ``capabilities(7)`` we expect
    and allow on that file in that package.

- **licenses/**

    Contains a JSON_ database of all approved licenses that packages
    may reference in the *License* field.

- **politics/**

    Per-product files listing allow or deny rules based on regular
    expression matching and comparing message digests.  This check
    originated as a way to catch country or political substrings in
    filenames.

- **fileinfo/**

    Per-product files (e.g., **fc35**) containing ``ls(1)`` style
    output of expected permissions, owners, groups, and filenames in
    packages.  The format is 4 whitespace-delimited fields. The first
    column is a human readable permission mask (e.g.,
    **-rwsr-s-r-x**), the second field is the owner name, the third
    field is the group name, the fourth field is the filename.

- **rebaseable/**

    Per-product files that list package names that are always allowed
    to rebase. That is, the names of the before and after packages
    match but the version numbers differ. That is considered a rebase
    and in some cases is not something we want to allow. Listing the
    file here permits it through the rebase check.

- **profiles/**

    This directory contains rpminspect_ profiles in the form of
    **NAME.yaml** where **NAME** is the name of the profile.  Any
    setting in **rpminspect.yaml** is valid in a profile.  Anyone can
    define a new profile. rpminspect_ uses the profile defined by the
    ``-p`` option. Profiles are intended for categories of packages
    that need to make the same adjustments to the rpminspect_
    configuration at runtime.  You can create a profile for those
    packages rather than modifying the **rpminspect.yaml** files for
    each of those packages.

- **security/**

    Contains per-product files that specify the actions to take for
    different security rules on a per package and version basis.  For
    example, a product may have an older build of a package like
    openssl and needs to relax some of the security rules in
    rpminspect.  Later product versions can define stricter rules.
    This file is also how you instruct rpminspect to ignore things it
    reports as needed product security inspection.  The idea here is
    that the package maintainer will consult with their product
    security team to determine if the reported problem warrants a
    security rule and if so add it to this file for that product.
    Subsequent runs of rpminspect on the package in question will
    follow the rule here and ideally not continue reporting the
    security failure.
    Each rule type only applies to the findings it names.  For
    example, the securitypath rule only applies to files under a
    security_path_prefix entry.  A rule for one type does not cause
    other inspections to treat the file as security related.

.. _rpminspect: https://github.com/dcantrell/rpminspect

.. _rpminspect-data-fedora: https://github.com/dcantrell/rpminspect-data-fedora

.. _RPM: https://rpm-packaging-guide.github.io/

.. _JSON: https://www.json.org

.. _here: https://github.com/dcantrell/rpminspect/blob/master/data/generic.yaml
