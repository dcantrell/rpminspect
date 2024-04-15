project('rpminspect',
        'cpp',
        version : '2.0',
        default_options : [
            'cpp_std=c++17',
            'warning_level=3',
            'werror=true',
            'buildtype=debugoptimized'
        ],
        license : 'GPL-3.0-or-later AND LGPL-3.0-or-later AND Apache-2.0 AND MIT')

fs = import('fs')

cpp = meson.get_compiler('cpp')

# Define the program version
add_global_arguments('-DPACKAGE_VERSION="@0@"'.format(meson.project_version()), language : ['cpp'])

# Always add _GNU_SOURCE because some other libraries rely on this macro
add_global_arguments('-D_GNU_SOURCE', language : ['cpp'])

# Define this to get around a problematic json_object.h macro
add_global_arguments('-D__STRICT_ANSI__', language : ['cpp'])

# Relax the compiler warnings->errors a bit on FreeBSD
if build_machine.system() == 'freebsd'
    add_global_arguments('-Wno-language-extension-token', language : ['cpp'])
    add_global_arguments('-Wno-unused-but-set-variable', language : ['cpp'])
endif

# On NetBSD we need to build without pedantic errors because of some
# things in pkgsrc
if build_machine.system() == 'netbsd'
    add_global_arguments('-Wno-pedantic', language : ['cpp'])
endif

# Vendor data directory
add_global_arguments('-DVENDOR_DATA_DIR="@0@"'.format(get_option('vendor_data_dir')), language : ['cpp'])

# Library search dirs
search_dirs = ['/usr/local/lib', '/usr/pkg/lib', '/usr/lib64', '/usr/lib']

# Header search dirs
if build_machine.system() == 'netbsd'
    include_dirs = ['/usr/local/include', '/usr/pkg/include', '/usr/include']
else
    include_dirs = ['/usr/local/include', '/usr/include']
endif
existing_include_dirs = []
foreach dir : include_dirs
    if fs.is_dir(dir)
        existing_include_dirs += dir
    endif
endforeach
sysinc = include_directories(existing_include_dirs)

# See if we have reallocarray in libc
if cpp.has_function('reallocarray')
    add_global_arguments('-D_HAVE_REALLOCARRAY', language : ['cpp'])
endif

# On FreeBSD, figure out where strverscmp() is
libiberty = cpp.find_library('iberty',
                             dirs : search_dirs,
                             required : false,
                             static : true)

if build_machine.system() == 'freebsd'
    # FreeBSD >= 14.x has it in libc
    if cpp.has_function('strverscmp')
        add_global_arguments('-D__BSD_VISIBLE', language : ['cpp'])
    else
        # check GNU libiberty
        if not libiberty.found()
            error('*** unable to find libiberty.a for strverscmp()')
        endif

        add_global_arguments('-D_FREEBSD_LIBIBERTY', language : ['cpp'])
    endif
endif

# Translations
if find_program('xgettext', required : get_option('nls')).found()
    if build_machine.system() == 'freebsd'
        intl = cpp.find_library('intl',
                                dirs : search_dirs,
                                required : true,
                                static : false)

        if not intl.found() or not cpp.has_function('libintl_gettext', dependencies : [intl])
            error('*** unable to find libintl (with symbol libintl_gettext())')
        endif
    endif

    add_global_arguments('-DGETTEXT_DOMAIN="' + meson.project_name() + '"', language : ['cpp'])
    subdir('po')
endif

# Dependencies that use pkg-config
jsonc = dependency('json-c', required : true)
libxml = dependency('libxml-2.0', required : true)
libcurl = dependency('libcurl', required : true)
zlib = dependency('zlib', required : true)
yaml = dependency('yaml-0.1', required : true)
clamav = dependency('libclamav', required : true)
icu_uc = dependency('icu-uc', required : true)
icu_io = dependency('icu-io', required : true)

# Check for xmlSetGenericErrorFunc in newer releases of libxml
if cpp.has_function('xmlSetGenericErrorFunc', dependencies : [libxml])
    add_project_arguments('-D_HAVE_XMLSETGENERICERRORFUNC', language : ['cpp'])
endif

# Check for newer CURLcode in libcurl
curlinfo_src = '''
    #include <curl/curl.h>
    int main(void)
    {
        CURLcode info = CURLINFO_CONTENT_LENGTH_DOWNLOAD_T;
        return 0;
    }
    '''
have_newer_curlinfo = cpp.compiles(curlinfo_src,
                                   dependencies: [libcurl],
                                   name: 'CURLINFO_CONTENT_LENGTH_DOWNLOAD_T availability test',
                                   no_builtin_args: true)

if have_newer_curlinfo
    add_project_arguments('-D_HAVE_NEWER_CURLINFO', language : ['cpp'])
endif

# libkmod
if get_option('with_libkmod')
    if build_machine.system() == 'freebsd'
        message('libkmod is not supported on FreeBSD, disabling support')
        libkmod = disabler()
    elif build_machine.system() == 'netbsd'
        message('libkmod is not supported on NetBSD, disabling support')
        libkmod = disabler()
    else
        libkmod = dependency('libkmod', required : true)
        add_project_arguments('-D_WITH_LIBKMOD', language : ['cpp'])

        if not cpp.has_header('libkmod.h', include_directories : sysinc)
            if cpp.has_header('kmod/libkmod.h', include_directories : sysinc)
                add_project_arguments('-D_LIBKMOD_HEADER_SUBDIR', language : ['cpp'])
            else
                error('*** unable to find libkmod.h')
            endif
        endif
    endif
else
    message('disabling Linux kernel module support (libkmod)')
    libkmod = disabler()
endif

# If we're on an older API, note that for the build
rpm = dependency('rpm', required : true)
if rpm.version().version_compare('<4.15.0')
    add_project_arguments('-D_HAVE_OLD_RPM_API', language : ['cpp'])
endif

# librpmbuild
rpmbuild = cpp.find_library('rpmbuild', dirs : search_dirs, required : true)

if rpmbuild.found()
    message('found librpmbuild')

    if not cpp.has_function('rpmSpecParse', dependencies : [rpmbuild])
        error('*** unable to find rpmSpecParse() in librpmbuild')
    endif
else
    error('*** unable to find librpmbuild')
endif

# check for RPMTAG_MODULARITYLABEL usability
rpmtag_src = '''
    #include <rpm/rpmtag.h>
    int main(void)
    {
        return RPMTAG_MODULARITYLABEL;
    }
    '''
have_modularitylabel = cpp.compiles(rpmtag_src,
                                    name: 'RPMTAG_MODULARITYLABEL availability test')

if have_modularitylabel
    add_project_arguments('-D_HAVE_MODULARITYLABEL', language : ['cpp'])
endif

# libarchive (need to check for specific functions)
libarchive = dependency('libarchive', required : true)

if cpp.has_function('archive_version_details', dependencies : [libarchive])
    add_project_arguments('-D_HAVE_ARCHIVE_VERSION_DETAILS', language : ['cpp'])
elif cpp.has_function('archive_version_string', dependencies : [libarchive])
    add_project_arguments('-D_HAVE_ARCHIVE_VERSION_STRING', language : ['cpp'])
endif

# openssl (need to check for the version info function)
openssl = dependency('openssl', required : true)

have_openssl_version = cpp.has_function('OpenSSL_version', dependencies : [openssl])
have_ssleay_version = cpp.has_function('SSLeay_version', dependencies : [openssl])

if not have_openssl_version and have_ssleay_version
    add_project_arguments('-DOpenSSL_version=SSLeay_version', language : ['cpp'])
elif not have_openssl_version and not have_ssleay_version
    add_project_arguments('-D_NO_OPENSSL_VERSION_FUNCTION', language : ['cpp'])
endif

# Test suite dependencies
run_tests = get_option('tests')
if run_tests
    cunit = dependency('cunit', method : 'pkg-config', required : true)
    python = find_program(get_option('python_program'), required : false)
else
    cunit = disabler()
    python = disabler()
endif

# Other dependencies

# libelf
libelf = dependency('libelf', method : 'pkg-config', required : false)

if not libelf.found() and cpp.has_function('elf_begin', args : ['-lelf'])
    libelf = declare_dependency(link_args : ['-lelf'])
endif

if not libelf.found()
    error('*** unable to find libelf')
endif

# xmlrpc-c
xmlrpc_cmd = find_program('xmlrpc-c-config', required : true)
xmlrpc_libs_cmd = run_command(xmlrpc_cmd, '--libs', check : true)
xmlrpc_cflags_cmd = run_command(xmlrpc_cmd, '--cflags', check : true)

if xmlrpc_libs_cmd.returncode() == 0 and xmlrpc_cflags_cmd.returncode() == 0
    xmlrpc = declare_dependency(compile_args: xmlrpc_cflags_cmd.stdout().strip().split(),
                                link_args: xmlrpc_libs_cmd.stdout().strip().split())
    message('Declared dependency xmlrpc')
endif

if not xmlrpc.found()
    error('*** unable to find libxmlrpc')
endif

xmlrpc_cmd = find_program('xmlrpc-c-config', required : true)
xmlrpc_client_libs_cmd = run_command(xmlrpc_cmd, 'client', '--libs', check : true)
xmlrpc_client_cflags_cmd = run_command(xmlrpc_cmd, 'client', '--cflags', check : true)

if xmlrpc_client_libs_cmd.returncode() == 0 and xmlrpc_client_cflags_cmd.returncode() == 0
    xmlrpc_client = declare_dependency(compile_args: xmlrpc_client_cflags_cmd.stdout().strip().split(),
                                       link_args: xmlrpc_client_libs_cmd.stdout().strip().split())
    message('Declared dependency xmlrpc_client')
endif

if not xmlrpc_client.found()
    error('*** unable to find libxmlrpc_client')
endif

# libmandoc (favor static one over shared one)
mandoc = cpp.find_library('mandoc',
                          dirs : search_dirs,
                          required : false,
                          static : true)

if mandoc.found()
    message('found static libmandoc')
else
    mandoc = cpp.find_library('mandoc',
                              dirs : search_dirs,
                              required : false,
                              static : false)

    if mandoc.found()
        message('found shared libmandoc')
    else
        error('*** unable to find libmandoc')
    endif
endif

if not cpp.has_function('mparse_alloc', dependencies : [mandoc, zlib])
    error('*** unable to find mparse_alloc() in libmandoc')
endif

if cpp.has_header('mandoc_parse.h', include_directories : sysinc)
    add_project_arguments('-DNEWLIBMANDOC', language : ['cpp'])
else
    if cpp.has_header('mandoc/mandoc_parse.h', include_directories : sysinc)
        add_project_arguments('-DNEWLIBMANDOC', language : ['cpp'])
        add_project_arguments('-DMANDOC_INCLUDE_SUBDIR', language : ['cpp'])
    else
        message('using libmandoc < 1.14.5 API')
    endif
endif

# libmagic
if not cpp.has_function('magic_open', args : ['-lmagic'])
    error('*** unable to find magic_open() in libmagic')
endif

if cpp.has_function('magic_version', args : ['-lmagic'])
    add_project_arguments('-D_HAVE_MAGIC_VERSION', language : ['cpp'])
endif

magic = declare_dependency(link_args : ['-lmagic'])

# libcap
if get_option('with_libcap')
    if build_machine.system() == 'freebsd'
        message('libcap is not supported on FreeBSD, disabling support')
        libcap = disabler()
    elif build_machine.system() == 'netbsd'
        message('libcap is not supported on NetBSD, disabling support')
        libcap = disabler()
    else
        libcap = dependency('libcap', method : 'pkg-config', required : false)

        if not libcap.found() and cpp.has_function('cap_to_text', args : ['-lcap'])
            libcap = declare_dependency(link_args : ['-lcap'])
        endif

        if not libcap.found()
            error('*** unable to find libcap')
        endif

        add_project_arguments('-D_WITH_LIBCAP', language : ['cpp'])
    endif
else
    message('disabling Linux capability(7) support (libcap)')
    libcap = disabler()
endif

# annocheck(1) or libannocheck
if get_option('with_annocheck') and get_option('with_libannocheck')
    error('*** you may only use with_annocheck or with_libannocheck, not both')
endif

if get_option('with_annocheck')
    if build_machine.system() == 'freebsd'
        message('disabling annocheck(1) support on FreeBSD')
    else
        add_project_arguments('-D_WITH_ANNOCHECK', language : ['cpp'])
    endif
endif

if get_option('with_libannocheck')
    if build_machine.system() == 'freebsd'
        message('libannocheck is not supported on FreeBSD, disabling support')
    else
        libannocheck = dependency('libannocheck', required : true)
        add_project_arguments('-D_WITH_LIBANNOCHECK', language : ['cpp'])
    endif
else
    message('disabling libannocheck support')
endif

# dlopen
if build_machine.system() != 'netbsd'
    # dlopen() is in libc on NetBSD, but in libdl elsewhere
    if not cpp.has_function('dlopen', args : ['-ldl'], dependencies : [zlib])
        error('*** unable to find dlopen() in libdl')
    endif

    dl = declare_dependency(link_args : ['-ldl'])
endif

# libm
if not cpp.has_function('round', args : ['-lm'])
    error('*** unable to find round() in libm')
endif

if not cpp.has_function('lround', args : ['-lm'])
    error('*** unable to find lround() in libm')
endif

m = declare_dependency(link_args : ['-lm'])

# cdson
cdson = dependency('cdson', required : true)

# Check for sys/queue.h
if not cpp.has_header('sys/queue.h', include_directories : sysinc)
    message('<sys/queue.h> not found, using bundled copy')
    add_project_arguments('-D_COMPAT_QUEUE', language : ['cpp'])
endif

# toml++
tomlpp = dependency('tomlplusplus', required : true)
add_project_arguments('-DTOML_EXCEPTIONS=0', language : ['cpp'])

# Header files for builds
inc = include_directories('include')
incxdiff = include_directories('libxdiff')

# Include all of the relevant subdirectories of the source tree
subdir('libxdiff')
subdir('lib')
subdir('src')
subdir('include')
subdir('data')
subdir('test')
