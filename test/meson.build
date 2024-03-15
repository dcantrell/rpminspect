# Test setups
add_test_setup(
    'valgrind',
    exe_wrapper : ['valgrind', '--verbose', '--error-exitcode=1', '--leak-check=full', '--track-origins=yes'])

# Unit test suite (disabled if we do not have CUnit installed)
if cunit.found()
    test_badwords = executable(
        'test-badwords',
        ['lib/test-badwords.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_koji = executable(
        'test-koji',
        ['lib/test-koji.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_tty = executable(
        'test-tty',
        ['lib/test-tty.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_strfuncs = executable(
        'test-strfuncs',
        ['lib/test-strfuncs.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_init = executable(
        'test-init',
        ['lib/test-init.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_inspect_elf = executable(
        'test-inspect_elf',
        ['lib/test-inspect_elf.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [
            cunit,
            libelf,
            libkmod,
        ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_abspath = executable(
        'test-abspath',
        ['lib/test-abspath.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_humansize = executable(
        'test-humansize',
        ['lib/test-humansize.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_arches = executable(
        'test-arches',
        ['lib/test-arches.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    test_results = executable(
        'test-results',
        ['lib/test-results.cxx',
         'lib/test-main.cxx'],
        include_directories : inc,
        dependencies : [ cunit, libkmod ],
        cpp_args : '-D_BUILDDIR_="@0@"'.format(meson.current_build_dir()),
        link_with : [ librpminspect ],
    )

    # Support program used by test-inspect-elf
    execstack_prog = executable(
        'execstack',
        ['lib/elftest.cxx'],
        link_args : ['-Wl,-z,execstack', '-Wl,-z,relro,-z,lazy']
    )

    noexecstack_prog = executable(
        'noexecstack',
        ['lib/elftest.cxx'],
        link_args : ['-Wl,-z,noexecstack', '-Wl,-z,relro']
    )

    # Unit tests
    test('test-badwords', test_badwords)
    test('test-koji', test_koji)
    test('test-tty', test_tty)
    test('test-strfuncs', test_strfuncs)
    test('test-init', test_init)
    test('test-inspect_elf',
         test_inspect_elf,
         depends : [execstack_prog, noexecstack_prog]
    )
    test('test-abspath', test_abspath)
    test('test-humansize', test_humansize)
    test('test-arches', test_arches)
    test('test-results', test_results)
else
    warning('CUnit not found, skipping unit test suite')
endif

# Integration test suite
if python.found()
    test_env = environment()
    test_env.set('RPMINSPECT', rpminspect_prog.full_path())
    test_env.set('RPMINSPECT_YAML', meson.source_root() + '/data/generic.yaml')
    test_env.set('RPMINSPECT_TEST_DATA_PATH', meson.source_root() + '/test/data')

    test_suites = [
        'test_abidiff.py',
        'test_addedfiles.py',
        'test_annocheck.py',
        'test_badfuncs.py',
        'test_capabilities.py',
        'test_cfg.py',
        'test_changedfiles.py',
        'test_changelog.py',
        'test_command.py',
        'test_config.py',
        'test_default.py',
        'test_debuginfo.py',
        'test_desktop.py',
        'test_disttag.py',
        'test_doc.py',
        'test_elf.py',
        'test_emptyrpm.py',
        'test_files.py',
        'test_filesize.py',
        'test_kmod.py',
        'test_license.py',
        'test_lostpayload.py',
        'test_lto.py',
        'test_manpage.py',
        'test_metadata.py',
        'test_modularity.py',
        'test_ownership.py',
        'test_patches.py',
        'test_pathmigration.py',
        'test_permissions.py',
        'test_politics.py',
        'test_rpmdeps_requires.py',
        'test_rpmdeps_provides.py',
        'test_rpmdeps_conflicts.py',
        'test_rpmdeps_obsoletes.py',
        'test_rpmdeps_enhances.py',
        'test_rpmdeps_recommends.py',
        'test_rpmdeps_suggests.py',
        'test_rpmdeps_supplements.py',
        'test_runpath.py',
        'test_securitypath.py',
        'test_shellsyntax.py',
        'test_specname.py',
        'test_symlinks.py',
        'test_types.py',
        'test_udevrules.py',
        'test_unicode.py',
        'test_upstream.py',
        'test_virus.py',
        'test_xml.py'
        ]

    foreach suite : test_suites
        test('rpminspect-test-suite.' + suite.split('.')[0],
             python,
             args : ['-B', '-m', 'unittest', 'discover', '-v', meson.source_root() + '/test', suite],
             env : test_env,
             is_parallel : false,
             timeout : 1500
            )
    endforeach

else
    warning('Python not found, skipping integration test suite')
endif
