sqlite_src = files([
	'error_category.cc',
	'rstream.cc'
])
sqlite_deps = [sqlite3]
sqlite_name = 'sqlite'

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
