# Generate inspect_elf_bits.cxx

pic_bits = find_program('pic_bits.sh')

inspect_elf_bits_cxx = custom_target(
    'inspect_elf_bits_cxx',
    output : 'inspect_elf_bits.cxx',
    command : [pic_bits, '@OUTPUT@'],
    install : false
)

inspect_elf_bits_dep = declare_dependency(sources : [inspect_elf_bits_cxx])

# Build librpminspect
librpminspect_sources = [
    'abi.cxx',
    'abspath.cxx',
    'arches.cxx',
    'array.cxx',
    'badwords.cxx',
    'builds.cxx',
    'checksums.cxx',
    'copyfile.cxx',
    'curl.cxx',
    'debug.cxx',
    'delta.cxx',
    'deprules.cxx',
    'diags.cxx',
    'filecmp.cxx',
    'fileinfo.cxx',
    'files.cxx',
    'flags.cxx',
    'free.cxx',
    'fs.cxx',
    'humansize.cxx',
    'init.cxx',
    'inspect.cxx',
    'inspect_addedfiles.cxx',
    'inspect_abidiff.cxx',
    'inspect_arch.cxx',
    'inspect_badfuncs.cxx',
    'inspect_changedfiles.cxx',
    'inspect_changelog.cxx',
    'inspect_config.cxx',
    'inspect_debuginfo.cxx',
    'inspect_desktop.cxx',
    'inspect_disttag.cxx',
    'inspect_doc.cxx',
    'inspect_dsodeps.cxx',
    'inspect_elf.cxx',
    'inspect_emptyrpm.cxx',
    'inspect_files.cxx',
    'inspect_filesize.cxx',
    'inspect_javabytecode.cxx',
    'inspect_kmidiff.cxx',
    'inspect_license.cxx',
    'inspect_lostpayload.cxx',
    'inspect_lto.cxx',
    'inspect_manpage.cxx',
    'inspect_metadata.cxx',
    'inspect_movedfiles.cxx',
    'inspect_ownership.cxx',
    'inspect_patches.cxx',
    'inspect_pathmigration.cxx',
    'inspect_permissions.cxx',
    'inspect_politics.cxx',
    'inspect_removedfiles.cxx',
    'inspect_rpmdeps.cxx',
    'inspect_runpath.cxx',
    'inspect_shellsyntax.cxx',
    'inspect_specname.cxx',
    'inspect_subpackages.cxx',
    'inspect_symlinks.cxx',
    'inspect_types.cxx',
    'inspect_udevrules.cxx',
    'inspect_unicode.cxx',
    'inspect_upstream.cxx',
    'inspect_virus.cxx',
    'inspect_xml.cxx',
    'joinpath.cxx',
    'koji.cxx',
    'listfuncs.cxx',
    'local.cxx',
    'llvm.cxx',
    'macros.cxx',
    'magic.cxx',
    'mkdirp.cxx',
    'output.cxx',
    'output_json.cxx',
    'output_summary.cxx',
    'output_text.cxx',
    'output_xunit.cxx',
    'ownership.cxx',
    'parse_dson.cxx',
    'parse_json.cxx',
    'parse_yaml.cxx',
    'pairfuncs.cxx',
    'paths.cxx',
    'peers.cxx',
    'permissions.cxx',
    'readelf.cxx',
    'readfile.cxx',
    'rebase.cxx',
    'release.cxx',
    'remedy.cxx',
    'results.cxx',
    'rmtree.cxx',
    'rpm.cxx',
    'runcmd.cxx',
    'secrule.cxx',
    'strfuncs.cxx',
    'tty.cxx',
    'uncompress.cxx',
    'unpack.cxx',
]

deps = [
    inspect_elf_bits_dep,
    tomlpp,
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
    librpminspect_sources += ['inspect_modularity.cxx']
endif

if build_machine.system() != 'netbsd'
    deps += [dl]
endif

if build_machine.system() != 'freebsd'
    if get_option('with_libkmod')
        librpminspect_sources += ['inspect_kmod.cxx', 'kmods.cxx']
        deps += [libkmod]
    endif

    if get_option('with_libcap')
        librpminspect_sources += ['inspect_capabilities.cxx']
        deps += [libcap]
    endif

    if get_option('with_annocheck') or get_option('with_libannocheck')
        librpminspect_sources += ['inspect_annocheck.cxx']

        if get_option('with_libannocheck')
            deps += [libannocheck]
        endif
    endif
endif

if build_machine.system() == 'freebsd'
    deps += [intl]
endif

incdirs = [inc, incxdiff]
locallibs = [libxdiff]

librpminspect = library(
    'rpminspect',
    librpminspect_sources,
    include_directories : incdirs,
    version : '0.5.0',
    install : true,
    link_with : locallibs,
    dependencies : deps
)
