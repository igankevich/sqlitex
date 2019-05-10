sqlite_src = files([
	'blob.cc',
	'database.cc',
	'errc.cc',
	'statement.cc',
])
sqlite_deps = [sqlite3]
sqlite_name = 'sqlitex'

sqlite_lib = library(
	sqlite_name,
	sources: sqlite_src,
	include_directories: src,
	implicit_include_directories: false,
	dependencies: sqlite_deps,
	version: meson.project_version(),
	install: true,
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
		'allocator_base.hh',
		'allocator.hh',
		'any.hh',
		'backup.hh',
		'blob.hh',
		'collation.hh',
		'column_metadata.hh',
		'configure.hh',
		'context.hh',
		'database.hh',
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
	subdir: sqlite_name
)
