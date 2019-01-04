project(
	'sqlitex',
	'cpp',
	version: '0.1.2',
	meson_version: '>=0.46',
	default_options: ['cpp_std=c++11']
)

project_description = 'A library of C++ abstractions for sqlite'

pkgconfig = import('pkgconfig')

sqlite3 = dependency('sqlite3')

subdir('src')
subdir('pkg')
