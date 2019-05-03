#ifndef SQLITEX_FORWARD_HH
#define SQLITEX_FORWARD_HH

#include <chrono>

#include <sqlite3.h>

namespace sqlite {

	namespace types {

		using mutex = ::sqlite3_mutex;
		using database = ::sqlite3;
		using int64 = ::sqlite3_int64;
		using uint64 = ::sqlite3_uint64;
		using file = ::sqlite3_file;
		using busy_handler = int(*)(void*,int);
		using statement = ::sqlite3_stmt;
		using value = ::sqlite3_value;
		using context = ::sqlite3_context;
		using entry_point = void(*)(void);
		using io_methods = ::sqlite3_io_methods;
		using memory_methods = ::sqlite3_mem_methods;
		using mutex_methods = ::sqlite3_mutex_methods;
		using api_routines = ::sqlite3_api_routines;
		using vfs = ::sqlite3_vfs;
		using blob = ::sqlite3_blob;
		using backup = ::sqlite3_backup;
		using scalar_function = void(*)(context*,int,value**);
		using commit_hook = int(*)(void*);
		using rollback_hook = void(*)(void*);
		using update_hook = void(*)(void*,int,char const *,char const *,int64);

	}

	enum class limit {
		length=SQLITE_LIMIT_LENGTH,
		sql_length=SQLITE_LIMIT_SQL_LENGTH,
		column=SQLITE_LIMIT_COLUMN,
		expression_depth=SQLITE_LIMIT_EXPR_DEPTH,
		compound_select=SQLITE_LIMIT_COMPOUND_SELECT,
		vdbe_instructions=SQLITE_LIMIT_VDBE_OP,
		function_arguments=SQLITE_LIMIT_FUNCTION_ARG,
		attached=SQLITE_LIMIT_ATTACHED,
		like_pattern_length=SQLITE_LIMIT_LIKE_PATTERN_LENGTH,
		variable_number=SQLITE_LIMIT_VARIABLE_NUMBER,
		trigger_depth=SQLITE_LIMIT_TRIGGER_DEPTH,
		worker_threads=SQLITE_LIMIT_WORKER_THREADS,
	};

	enum class data_type {
		integer = SQLITE_INTEGER,
		floating_point = SQLITE_FLOAT,
		blob = SQLITE_BLOB,
		null = SQLITE_NULL,
		text = SQLITE_TEXT,
	};

	class cstream;
	class rstream;
	class rstream_base;
	class database;
	class backup;
	class random_device;
	class any;
	class allocator;
	class static_database;

	inline const char* version() { return ::sqlite3_version; }
	inline const char* library_version() { return ::sqlite3_libversion(); }
	inline const char* source_id() { return ::sqlite3_sourceid(); }
	inline int library_version_number() { return ::sqlite3_libversion_number(); }
	inline void sleep(std::chrono::milliseconds ms) { ::sqlite3_sleep(ms.count()); }
	inline int release_memory(int nbytes) { return ::sqlite3_release_memory(nbytes); }
	inline const char* temporary_directory() { return ::sqlite3_temp_directory; }
	inline const char* data_directory() { return ::sqlite3_data_directory; }
	inline types::int64 soft_heap_limit(types::int64 nbytes) {
		return ::sqlite3_soft_heap_limit64(nbytes);
	}
	inline types::int64 soft_heap_limit() { return soft_heap_limit(-1); }
	inline bool is_complete(const char* sql) { return ::sqlite3_complete(sql) != 0; }
	inline types::uint64 memory_used() { return ::sqlite3_memory_used(); }
	inline types::uint64 max_memory_used() { return ::sqlite3_memory_highwater(0); }
	inline types::uint64 max_memory_used(int reset) { return ::sqlite3_memory_highwater(reset); }

}

#endif // vim:filetype=cpp
