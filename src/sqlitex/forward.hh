#ifndef SQLITEX_FORWARD_HH
#define SQLITEX_FORWARD_HH

#include <chrono>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>

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
		using allocator_methods = ::sqlite3_mem_methods;
		using mutex_methods = ::sqlite3_mutex_methods;
		using api_routines = ::sqlite3_api_routines;
		using page_cache_methods = ::sqlite3_pcache_methods2;
		using vfs = ::sqlite3_vfs;
		using blob = ::sqlite3_blob;
		using backup = ::sqlite3_backup;
		using scalar_function = void(*)(context*,int,value**);
		using commit_hook = int(*)(void*);
		using rollback_hook = void(*)(void*);
		using update_hook = void(*)(void*,int,char const *,char const *,int64);
		using log_callback = void(*)(void*,int,const char*);
		using sql_log_callback = void(*)(void*,database*,const char*, int);
		using page_cache = ::sqlite3_pcache;
		using page = ::sqlite3_pcache_page;
		using destructor = void (*)(void*);
		using unlock_notify = void (*)(void **apArg, int nArg);
		using snapshot = ::sqlite3_snapshot;
		using virtual_table = ::sqlite3_vtab;
		using module = ::sqlite3_module;
		using virtual_table_cursor = ::sqlite3_vtab_cursor;
		using index_info = ::sqlite3_index_info;
		using virtual_table_function = void (*)(context*,int,value**);
		using syscall_ptr = ::sqlite3_syscall_ptr;
		using symbol = void(*)(void);
		#if defined(SQLITE_ENABLE_SESSION)
		using session = ::sqlite3_session;
		using changeset_iterator = ::sqlite3_changeset_iter;
		using changegroup = ::sqlite3_changegroup;
		#endif

	}

	namespace bits {
		template <class T> inline void destroy(void* ptr) { delete reinterpret_cast<T*>(ptr); }
	}

	class cstream;
	class preupdate_database;
	class rstream;
	class statement_counters;
	class context;
	class rstream_base;
	class database;
	class backup;
	class snapshot;
	class virtual_table;
	class virtual_table_cursor;
	class virtual_table_context;
	class virtual_table_index;
	class random_device;
	class any;
	class blob_stream;
	class column_metadata;
	template <class T> class allocator;
	class static_database;
	class uri;

	using string = std::basic_string<char,std::char_traits<char>,allocator<char>>;

	using u8string = std::string;
	using u16string = std::u16string;
	using int64 = types::int64;
	using uint64 = types::uint64;

	template <class Alloc>
	using basic_u8string = std::basic_string<char,std::char_traits<char>,Alloc>;

	template <class Alloc>
	using basic_u16string = std::basic_string<char16_t,std::char_traits<char16_t>,Alloc>;

	struct sqlite_deleter {
		inline void operator()(const void* ptr) { ::sqlite3_free(const_cast<void*>(ptr)); }
	};

	template <class T>
	using unique_ptr = std::unique_ptr<T,sqlite_deleter>;

	enum class errc: int;

	enum class file_flag: int {
		read_only = SQLITE_OPEN_READONLY,
		read_write = SQLITE_OPEN_READWRITE,
		create = SQLITE_OPEN_CREATE,
		no_mutex = SQLITE_OPEN_NOMUTEX,
		full_mutex = SQLITE_OPEN_FULLMUTEX,
		shared_cache = SQLITE_OPEN_SHAREDCACHE,
		private_cache = SQLITE_OPEN_PRIVATECACHE,
		uri = SQLITE_OPEN_URI,
		delete_on_close=SQLITE_OPEN_DELETEONCLOSE,
		exclusive=SQLITE_OPEN_EXCLUSIVE,
		autoproxy=SQLITE_OPEN_AUTOPROXY,
		memory=SQLITE_OPEN_MEMORY,
		main_db=SQLITE_OPEN_MAIN_DB,
		temp_db=SQLITE_OPEN_TEMP_DB,
		transient_db=SQLITE_OPEN_TRANSIENT_DB,
		main_journal=SQLITE_OPEN_MAIN_JOURNAL,
		temp_journal=SQLITE_OPEN_TEMP_JOURNAL,
		subjournal=SQLITE_OPEN_SUBJOURNAL,
		master_journal=SQLITE_OPEN_MASTER_JOURNAL,
		wal=SQLITE_OPEN_WAL,
	};

	inline file_flag
	operator|(file_flag a, file_flag b) {
		return file_flag(int(a) | int(b));
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

	enum class action: int {
		create_index=SQLITE_CREATE_INDEX,
		create_table=SQLITE_CREATE_TABLE,
		create_temp_index=SQLITE_CREATE_TEMP_INDEX,
		create_temp_table=SQLITE_CREATE_TEMP_TABLE,
		create_temp_trigger=SQLITE_CREATE_TEMP_TRIGGER,
		create_temp_view=SQLITE_CREATE_TEMP_VIEW,
		create_trigger=SQLITE_CREATE_TRIGGER,
		create_view=SQLITE_CREATE_VIEW,
		delete_=SQLITE_DELETE,
		drop_index=SQLITE_DROP_INDEX,
		drop_table=SQLITE_DROP_TABLE,
		drop_temp_index=SQLITE_DROP_TEMP_INDEX,
		drop_temp_table=SQLITE_DROP_TEMP_TABLE,
		drop_temp_trigger=SQLITE_DROP_TEMP_TRIGGER,
		drop_temp_view=SQLITE_DROP_TEMP_VIEW,
		drop_trigger=SQLITE_DROP_TRIGGER,
		drop_view=SQLITE_DROP_VIEW,
		insert=SQLITE_INSERT,
		pragma=SQLITE_PRAGMA,
		read=SQLITE_READ,
		select=SQLITE_SELECT,
		transaction=SQLITE_TRANSACTION,
		update=SQLITE_UPDATE,
		attach=SQLITE_ATTACH,
		detach=SQLITE_DETACH,
		alter_table=SQLITE_ALTER_TABLE,
		reindex=SQLITE_REINDEX,
		analyze=SQLITE_ANALYZE,
		create_vtable=SQLITE_CREATE_VTABLE,
		drop_vtable=SQLITE_DROP_VTABLE,
		function=SQLITE_FUNCTION,
		savepoint=SQLITE_SAVEPOINT,
		copy=SQLITE_COPY,
		recursive=SQLITE_RECURSIVE,
	};

	enum class permission: int {
		allow=SQLITE_OK,
		deny=SQLITE_DENY,
		ignore=SQLITE_IGNORE,
	};

	enum class trace: unsigned {
		statement=SQLITE_TRACE_STMT,
		profile=SQLITE_TRACE_PROFILE,
		row=SQLITE_TRACE_ROW,
		close=SQLITE_TRACE_CLOSE,
		all=std::numeric_limits<unsigned>::max()
	};

	inline trace
	operator|(trace a, trace b) {
		using tp = std::underlying_type<trace>::type;
		return trace(tp(a) | tp(b));
	}

	enum class prepare_f: unsigned int {
		#if defined(SQLITE_PREPARE_PERSISTENT)
		persistent=SQLITE_PREPARE_PERSISTENT,
		#endif
		#if defined(SQLITE_PREPARE_NORMALIZE)
		normalize=SQLITE_PREPARE_NORMALIZE,
		#endif
		#if defined(SQLITE_PREPARE_NO_VTAB)
		no_virtual_tables=SQLITE_PREPARE_NO_VTAB,
		#endif
	};

	inline prepare_f
	operator|(prepare_f a, prepare_f b) {
		using tp = std::underlying_type<prepare_f>::type;
		return prepare_f(tp(a) | tp(b));
	}

	enum class encoding {
		utf8 = SQLITE_UTF8,
		utf16 = SQLITE_UTF16,
		utf16be = SQLITE_UTF16BE,
		utf16le = SQLITE_UTF16LE,
		utf16_aligned = SQLITE_UTF16_ALIGNED,
	};

	template <encoding enc> struct encoding_traits;

	template <> struct encoding_traits<encoding::utf8> {
		using string = u8string;
	};

	template <> struct encoding_traits<encoding::utf16> {
		using string = u16string;
	};

	enum class checkpoint_mode: int {
		passive=SQLITE_CHECKPOINT_PASSIVE,
		full=SQLITE_CHECKPOINT_FULL,
		restart=SQLITE_CHECKPOINT_RESTART,
		truncate=SQLITE_CHECKPOINT_TRUNCATE,
	};

	struct checkpoint_result { int nframes; int ncheckpointed; };

	enum class conflict_policy: int {
		rollback=SQLITE_ROLLBACK,
		fail=SQLITE_FAIL,
		replace=SQLITE_REPLACE,
	};

	enum class file_operation: int {
		lockstate=SQLITE_FCNTL_LOCKSTATE,
		get_lockproxyfile=SQLITE_FCNTL_GET_LOCKPROXYFILE,
		set_lockproxyfile=SQLITE_FCNTL_SET_LOCKPROXYFILE,
		last_errno=SQLITE_FCNTL_LAST_ERRNO,
		size_hint=SQLITE_FCNTL_SIZE_HINT,
		chunk_size=SQLITE_FCNTL_CHUNK_SIZE,
		file_pointer=SQLITE_FCNTL_FILE_POINTER,
		sync_omitted=SQLITE_FCNTL_SYNC_OMITTED,
		win32_av_retry=SQLITE_FCNTL_WIN32_AV_RETRY,
		persist_wal=SQLITE_FCNTL_PERSIST_WAL,
		overwrite=SQLITE_FCNTL_OVERWRITE,
		vfsname=SQLITE_FCNTL_VFSNAME,
		powersafe_overwrite=SQLITE_FCNTL_POWERSAFE_OVERWRITE,
		pragma=SQLITE_FCNTL_PRAGMA,
		busyhandler=SQLITE_FCNTL_BUSYHANDLER,
		tempfilename=SQLITE_FCNTL_TEMPFILENAME,
		mmap_size=SQLITE_FCNTL_MMAP_SIZE,
		trace=SQLITE_FCNTL_TRACE,
		has_moved=SQLITE_FCNTL_HAS_MOVED,
		sync=SQLITE_FCNTL_SYNC,
		commit_phasetwo=SQLITE_FCNTL_COMMIT_PHASETWO,
		win32_set_handle=SQLITE_FCNTL_WIN32_SET_HANDLE,
		wal_block=SQLITE_FCNTL_WAL_BLOCK,
		zipvfs=SQLITE_FCNTL_ZIPVFS,
		rbu=SQLITE_FCNTL_RBU,
		vfs_pointer=SQLITE_FCNTL_VFS_POINTER,
		journal_pointer=SQLITE_FCNTL_JOURNAL_POINTER,
		win32_get_handle=SQLITE_FCNTL_WIN32_GET_HANDLE,
		pdb=SQLITE_FCNTL_PDB,
		begin_atomic_write=SQLITE_FCNTL_BEGIN_ATOMIC_WRITE,
		commit_atomic_write=SQLITE_FCNTL_COMMIT_ATOMIC_WRITE,
		rollback_atomic_write=SQLITE_FCNTL_ROLLBACK_ATOMIC_WRITE,
	};

	enum class scan_status: int {
		nloop=SQLITE_SCANSTAT_NLOOP,
		nvisit=SQLITE_SCANSTAT_NVISIT,
		estimate=SQLITE_SCANSTAT_EST,
		name=SQLITE_SCANSTAT_NAME,
		explain=SQLITE_SCANSTAT_EXPLAIN,
		select_id=SQLITE_SCANSTAT_SELECTID,
	};

	enum class auto_vacuum: int {none=0, full=1, incremental=2};

	enum class locking_mode { normal, exclusive };
	const char* to_string(locking_mode rhs);
	void operator>>(const u8string& str, locking_mode& rhs);

	enum class journal_mode { del, truncate, persist, memory, wal, none };
	const char* to_string(journal_mode rhs);
	void operator>>(const u8string& str, journal_mode& rhs);

	enum class sync_mode: int {none=0, normal=1, full=2, extra=3};

	enum class temp_store_mode: int {def=0, file=1, memory=2};

	template <class T>
	inline auto
	downcast(T value) -> typename std::enable_if<
		std::is_enum<T>::value, typename std::underlying_type<T>::type>::type {
		using tp = typename std::underlying_type<T>::type;
		return static_cast<tp>(value);
	}

	inline const char* version() { return ::sqlite3_version; }
	inline const char* library_version() { return ::sqlite3_libversion(); }
	inline const char* source_id() { return ::sqlite3_sourceid(); }
	inline int library_version_number() { return ::sqlite3_libversion_number(); }
	inline void sleep(std::chrono::milliseconds ms) { ::sqlite3_sleep(ms.count()); }
	inline int release_memory(int nbytes) { return ::sqlite3_release_memory(nbytes); }
	inline const char* temporary_directory() { return ::sqlite3_temp_directory; }
	inline const char* data_directory() { return ::sqlite3_data_directory; }
	inline int64 soft_heap_limit(int64 nbytes) { return ::sqlite3_soft_heap_limit64(nbytes); }
	inline int64 soft_heap_limit() { return soft_heap_limit(-1); }
	inline bool is_complete(const char* sql) { return ::sqlite3_complete(sql) != 0; }
	inline bool is_complete(const u8string& sql) { return ::sqlite3_complete(sql.data()) != 0; }
	inline bool is_complete(const u16string& sql) { return ::sqlite3_complete16(sql.data()) != 0; }
	inline bool is_threadsafe() { return ::sqlite3_threadsafe() != 0; }
	inline uint64 memory_used() { return ::sqlite3_memory_used(); }
	inline uint64 max_memory_used() { return ::sqlite3_memory_highwater(0); }
	inline uint64 max_memory_used(int reset) { return ::sqlite3_memory_highwater(reset); }

	#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
	inline bool
	compiled_with(const char* option) { return ::sqlite3_compileoption_used(option) != 0; }
	inline const char*
	compile_option(int index) { return ::sqlite3_compileoption_get(index); }
	#endif

	template <class ... Args>
	inline unique_ptr<const char>
	format(const char* format, const Args& ... args) {
		return unique_ptr<const char>(::sqlite3_mprintf(format, args...));
	}

	template <class ... Args>
	inline void
	log(errc code, const char* format, const Args& ... args) {
		::sqlite3_log(code, format, args...);
	}

	struct statistic { int64 current; int64 highwater; };

	inline int
	compare(const u8string& lhs, const u8string& rhs) {
		return ::sqlite3_stricmp(lhs.data(), rhs.data());
	}

	inline int
	compare(const u8string& lhs, const u8string& rhs, int length) {
		return ::sqlite3_strnicmp(lhs.data(), rhs.data(), length);
	}

	inline bool
	like(const u8string& lhs, const u8string& rhs, unsigned int escape=0) {
		return ::sqlite3_strlike(lhs.data(), rhs.data(), escape) == 0;
	}

	inline bool
	glob(const u8string& lhs, const u8string& rhs) {
		return ::sqlite3_strglob(lhs.data(), rhs.data()) == 0;
	}

}

#endif // vim:filetype=cpp
