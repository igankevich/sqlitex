project(
	'sqlitex',
	'cpp',
	version: '0.5.0',
	meson_version: '>=0.46',
	default_options: ['cpp_std=c++11'],
	license: ['Public domain']
)

project_description = 'A library of C++ abstractions for sqlite'

pkgconfig = import('pkgconfig')
sqlite3 = dependency('sqlite3')

cpp = meson.get_compiler('cpp')
foreach arg : [
    '-Werror=return-type',
    '-Werror=return-local-addr',
    '-Werror=cast-align',
    '-Wcast-align=strict']
    if cpp.has_argument(arg)
        add_global_arguments(arg, language: 'cpp')
    endif
endforeach

if not get_option('buildtype').contains('debug')
	add_global_arguments('-DNDEBUG', language: 'cpp')
endif

subdir('src')
