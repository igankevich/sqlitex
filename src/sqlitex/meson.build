sqlitex_src = files([
	'blob.cc',
	'connection.cc',
	'errc.cc',
	'statement.cc',
])
sqlitex_deps = [sqlite3]
sqlitex_name = 'sqlitex'

sqlitex_lib = library(
	sqlitex_name,
	sources: sqlitex_src,
	include_directories: src,
	implicit_include_directories: false,
	dependencies: sqlitex_deps,
	version: meson.project_version(),
	install: true,
)

pkgconfig.generate(
    sqlitex_lib,
    requires: [sqlite3],
	version: meson.project_version(),
	name: sqlitex_name,
	filebase: sqlitex_name,
	description: project_description,
)

install_headers(
	[
		'allocator_base.hh',
		'allocator.hh',
		'any.hh',
		'backup.hh',
		'blob.hh',
		'collation.hh',
		'column_metadata.hh',
		'configure.hh',
		'context.hh',
		'connection.hh',
		'errc.hh',
		'forward.hh',
		'function.hh',
		'mutex.hh',
		'named_ptr.hh',
		'random_device.hh',
		'statement.hh',
		'session.hh',
		'snapshot.hh',
		'status.hh',
		'transaction.hh',
		'uri.hh',
		'vfs.hh',
		'virtual_table.hh',
	],
	subdir: sqlitex_name
)
