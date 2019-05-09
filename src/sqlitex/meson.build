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
		'blob_stream.hh',
		'call.hh',
		'collation.hh',
		'column_metadata.hh',
		'configure.hh',
		'context.hh',
		'database.hh',
		'errc.hh',
		'error_category.hh',
		'ext.hh',
		'forward.hh',
		'function.hh',
		'mutex.hh',
		'named_ptr.hh',
		'random_device.hh',
		'rstream_base.hh',
		'rstream.hh',
		'session.hh',
		'snapshot.hh',
		'status.hh',
		'transaction.hh',
		'types.hh',
		'uri.hh',
		'vfs.hh',
		'virtual_table.hh',
	],
	subdir: sqlite_name
)
