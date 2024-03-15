# Build rpminspect
deps = [ rpm ]

# On some systems, GNU libiberty provides strverscmp()
if libiberty.found()
    deps += [ libiberty ]
endif

# FreeBSD systems require linking with libintl
if build_machine.system() == 'freebsd'
    deps += [ intl ]
endif

rpminspect_prog = executable(
    'rpminspect',
    [ 'rpminspect.cxx' ],
    include_directories : inc,
    install : true,
    link_with : [
        librpminspect,
    ],
    dependencies : deps
)

# Install man pages
install_man('rpminspect.1')
