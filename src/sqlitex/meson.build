sqlite_src = files([
	'error_category.cc',
	'rstream.cc',
	'database.cc',
])
sqlite_deps = [sqlite3]
sqlite_name = 'sqlitex'

sqlite_lib = library(
	sqlite_name,
	sources: sqlite_src,
	include_directories: src,
	dependencies: sqlite_deps,
	version: meson.project_version(),
	install: true
)

pkgconfig.generate(
	libraries: [sqlite_lib],
	version: meson.project_version(),
	name: sqlite_name,
	filebase: sqlite_name,
	description: project_description,
)

install_headers(
	[
		'backup.hh',
		'call.hh',
		'database.hh',
		'errc.hh',
		'error_category.hh',
		'ext.hh',
		'rstream.hh',
		'rstream_base.hh',
		'transaction.hh',
		'types.hh',
	],
	subdir: sqlite_name
)
