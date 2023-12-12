# Generate inspect_elf_bits.c

pic_bits = find_program('pic_bits.sh')

inspect_elf_bits_c = custom_target(
    'inspect_elf_bits_c',
    output : 'inspect_elf_bits.c',
    command : [ pic_bits, '@OUTPUT@' ],
    install : false
)

inspect_elf_bits_dep = declare_dependency(sources : [ inspect_elf_bits_c ])

# Build librpminspect
librpminspect_sources = [
    'abi.c',
    'abspath.c',
    'arches.c',
    'array.c',
    'badwords.c',
    'builds.c',
    'checksums.c',
    'copyfile.c',
    'curl.c',
    'debug.c',
    'delta.c',
    'deprules.c',
    'diags.c',
    'filecmp.c',
    'fileinfo.c',
    'files.c',
    'flags.c',
    'free.c',
    'fs.c',
    'humansize.c',
    'init.c',
    'inspect.c',
    'inspect_addedfiles.c',
    'inspect_abidiff.c',
    'inspect_arch.c',
    'inspect_badfuncs.c',
    'inspect_changedfiles.c',
    'inspect_changelog.c',
    'inspect_config.c',
    'inspect_debuginfo.c',
    'inspect_desktop.c',
    'inspect_disttag.c',
    'inspect_doc.c',
    'inspect_dsodeps.c',
    'inspect_elf.c',
    'inspect_emptyrpm.c',
    'inspect_files.c',
    'inspect_filesize.c',
    'inspect_javabytecode.c',
    'inspect_kmidiff.c',
    'inspect_license.c',
    'inspect_lostpayload.c',
    'inspect_lto.c',
    'inspect_manpage.c',
    'inspect_metadata.c',
    'inspect_movedfiles.c',
    'inspect_ownership.c',
    'inspect_patches.c',
    'inspect_pathmigration.c',
    'inspect_permissions.c',
    'inspect_politics.c',
    'inspect_removedfiles.c',
    'inspect_rpmdeps.c',
    'inspect_runpath.c',
    'inspect_shellsyntax.c',
    'inspect_specname.c',
    'inspect_subpackages.c',
    'inspect_symlinks.c',
    'inspect_types.c',
    'inspect_udevrules.c',
    'inspect_unicode.c',
    'inspect_upstream.c',
    'inspect_virus.c',
    'inspect_xml.c',
    'joinpath.c',
    'koji.c',
    'listfuncs.c',
    'local.c',
    'llvm.c',
    'macros.c',
    'magic.c',
    'mkdirp.c',
    'mofile.c',
    'output.c',
    'output_json.c',
    'output_summary.c',
    'output_text.c',
    'output_xunit.c',
    'ownership.c',
    'parse_dson.c',
    'parse_json.c',
    'parse_yaml.c',
    'pairfuncs.c',
    'paths.c',
    'peers.c',
    'permissions.c',
    'readelf.c',
    'readfile.c',
    'rebase.c',
    'release.c',
    'results.c',
    'rmtree.c',
    'rpm.c',
    'runcmd.c',
    'secrule.c',
    'strfuncs.c',
    'tty.c',
    'uncompress.c',
    'unpack.c',
]

deps = [
    inspect_elf_bits_dep,
    cdson,
    jsonc,
    xmlrpc,
    xmlrpc_client,
    libxml,
    rpm,
    rpmbuild,
    libarchive,
    libelf,
    libcurl,
    zlib,
    yaml,
    openssl,
    mandoc,
    magic,
    clamav,
    icu_uc,
    icu_io,
    m,
]

if have_modularitylabel
    librpminspect_sources += [ 'inspect_modularity.c' ]
endif

if build_machine.system() != 'netbsd'
    deps += [ dl ]
endif

if build_machine.system() != 'freebsd'
    if get_option('with_libkmod')
        librpminspect_sources += [ 'inspect_kmod.c', 'kmods.c' ]
        deps += [ libkmod ]
    endif

    if get_option('with_libcap')
        librpminspect_sources += [ 'inspect_capabilities.c' ]
        deps += [ libcap ]
    endif

    if get_option('with_annocheck') or get_option('with_libannocheck')
        librpminspect_sources += [ 'inspect_annocheck.c' ]

        if get_option('with_libannocheck')
            deps += [ libannocheck ]
        endif
    endif
endif

if build_machine.system() == 'freebsd'
    deps += [ intl ]
endif

librpminspect = library(
    'rpminspect',
    librpminspect_sources,
    include_directories : [ inc, incxdiff ],
    version : '0.5.0',
    install : true,
    link_with : [ libxdiff, ],
    dependencies : deps
)
