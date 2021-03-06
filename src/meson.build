#
# target: libcdvar.so
#

libcdvar_symfile = join_paths(meson.current_source_dir(), 'libcdvar.sym')

libcdvar_private = static_library(
        'cdvar-private',
        [
                'c-dvar.c',
                'c-dvar-reader.c',
                'c-dvar-type.c',
                'c-dvar-writer.c',
        ],
        c_args: [
                '-fvisibility=hidden',
                '-fno-common',
        ],
        pic: true,
)

libcdvar_shared = shared_library(
        'cdvar',
        objects: libcdvar_private.extract_all_objects(),
        install: not meson.is_subproject(),
        soversion: 0,
        link_depends: libcdvar_symfile,
        link_args: [
                '-Wl,--no-undefined',
                '-Wl,--version-script=@0@'.format(libcdvar_symfile),
        ],
)

libcdvar_dep = declare_dependency(
        include_directories: include_directories('.'),
        link_with: libcdvar_private,
        version: meson.project_version(),
)

if not meson.is_subproject()
        install_headers(
                'c-dvar.h',
                'c-dvar-type.h',
        )

        mod_pkgconfig.generate(
                libraries: libcdvar_shared,
                version: meson.project_version(),
                name: 'libcdvar',
                filebase: 'libcdvar',
                description: 'D-Bus Variant Type-System',
        )
endif

#
# target: test-*
#

test_api = executable('test-api', ['test-api.c'], link_with: libcdvar_shared)
test('API Symbol Visibility', test_api)

test_basic = executable('test-basic', ['test-basic.c'], dependencies: libcdvar_dep)
test('Basic API Behavior', test_basic)

if libdbus_typenum_dep.found()
        test_enumerated = executable('test-enumerated', ['test-enumerated.c'], dependencies: [ libcdvar_dep, libdbus_typenum_dep ])
        test('Type and Data Verification with Enumerated Types', test_enumerated)
endif

test_string = executable('test-string', ['test-string.c'], dependencies: libcdvar_dep)
test('D-Bus String Restrictions', test_string)

test_type = executable('test-type', ['test-type.c'], dependencies: libcdvar_dep)
test('Type and Signature Parser', test_type)

test_utf8 = executable('test-utf8', ['test-utf8.c'])
test('UTF-8 Validation', test_utf8)
