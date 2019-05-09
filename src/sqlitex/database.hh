#ifndef SQLITE_DATABASE_HH
#define SQLITE_DATABASE_HH

#include <chrono>
#include <functional>

#include <sqlitex/blob_stream.hh>
#include <sqlitex/call.hh>
#include <sqlitex/collation.hh>
#include <sqlitex/column_metadata.hh>
#include <sqlitex/forward.hh>
#include <sqlitex/function.hh>
#include <sqlitex/mutex.hh>
#include <sqlitex/open.hh>
#include <sqlitex/rstream.hh>
#include <sqlitex/snapshot.hh>
#include <sqlitex/virtual_table.hh>

namespace sqlite {

	class static_database {

	public:
		enum class status: int {
			lookaside_used=SQLITE_DBSTATUS_LOOKASIDE_USED,
			cache_used=SQLITE_DBSTATUS_CACHE_USED,
			schema_used=SQLITE_DBSTATUS_SCHEMA_USED,
			statement_used=SQLITE_DBSTATUS_STMT_USED,
			lookaside_hit=SQLITE_DBSTATUS_LOOKASIDE_HIT,
			lookaside_miss_size=SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE,
			lookaside_miss_full=SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL,
			cache_hit=SQLITE_DBSTATUS_CACHE_HIT,
			cache_miss=SQLITE_DBSTATUS_CACHE_MISS,
			cache_write=SQLITE_DBSTATUS_CACHE_WRITE,
			deferred_fks=SQLITE_DBSTATUS_DEFERRED_FKS,
			cache_used_shared=SQLITE_DBSTATUS_CACHE_USED_SHARED,
		};
		struct statistic { int current; int highwater; };

	protected:
		types::database* _ptr = nullptr;

	public:

		inline explicit static_database(types::database* db): _ptr(db) {}

		static_database() = default;
		static_database(static_database&&) = default;
		static_database& operator=(static_database&&) = default;
		static_database(const static_database&) = default;
		static_database& operator=(const static_database&) = default;

		inline const types::database* get() const { return this->_ptr; }
		inline types::database* get() { return this->_ptr; }

		inline void flush() { call(::sqlite3_db_cacheflush(this->_ptr)); }

		inline const char*
		filename(const char* name="main") const {
			return ::sqlite3_db_filename(this->_ptr, name);
		}

		inline bool
		read_only(const char* name="main") const {
			return ::sqlite3_db_readonly(this->_ptr, name) != 0;
		}

		inline void*
		commit_hook(types::commit_hook rhs, void* ptr=nullptr) {
			return ::sqlite3_commit_hook(this->_ptr, rhs, ptr);
		}

		inline void*
		rollback_hook(types::rollback_hook rhs, void* ptr=nullptr) {
			return ::sqlite3_rollback_hook(this->_ptr, rhs, ptr);
		}

		inline void*
		update_hook(types::update_hook rhs, void* ptr=nullptr) {
			return ::sqlite3_update_hook(this->_ptr, rhs, ptr);
		}

		template <class ... Args>
		inline rstream
		prepare(const u8string& sql, const Args& ... args) {
			types::statement* stmt = nullptr;
			call(::sqlite3_prepare_v2(this->_ptr, sql.data(), sql.size(), &stmt, nullptr));
			rstream rstr(stmt);
			bind(rstr, 1, args...);
			return rstr;
		}

		template <class ... Args>
		inline rstream
		prepare(const u8string& sql, const Args& ... args, prepare_f flags) {
			types::statement* stmt = nullptr;
			call(::sqlite3_prepare_v3(
				this->_ptr,
				sql.data(),
				sql.size(),
				downcast(flags),
				&stmt,
				nullptr
			));
			rstream rstr(stmt);
			bind(rstr, 1, args...);
			return rstr;
		}

		template <class ... Args>
		inline rstream
		prepare(const u16string& sql, const Args& ... args) {
			types::statement* stmt = nullptr;
			call(::sqlite3_prepare16_v2(
				this->_ptr,
				sql.data(),
				sql.size()*sizeof(char16_t),
				&stmt,
				nullptr
			));
			rstream rstr(stmt);
			bind(rstr, 1, args...);
			return rstr;
		}

		template <class ... Args>
		inline rstream
		prepare(const u16string& sql, const Args& ... args, prepare_f flags) {
			types::statement* stmt = nullptr;
			call(::sqlite3_prepare16_v3(
				this->_ptr,
				sql.data(),
				sql.size()*sizeof(char16_t),
				downcast(flags),
				&stmt,
				nullptr
			));
			rstream rstr(stmt);
			bind(rstr, 1, args...);
			return rstr;
		}

		template <class ... Args>
		inline void
		execute(const u8string& sql, const Args& ... args) {
			rstream&& rstr = this->prepare(sql, args...);
			rstr.step();
			rstr.close();
		}

		inline void
		execute(const char* sql) {
			call(this->do_execute(sql));
		}

		inline void
		execute(const u8string& sql) {
			call(this->do_execute(sql.data()));
		}

		inline int
		num_rows_modified() const noexcept {
			return ::sqlite3_changes(this->_ptr);
		}

		inline int
		total_rows_modified() const noexcept {
			return ::sqlite3_total_changes(this->_ptr);
		}

		inline int64_t
		last_insert_row_id() const noexcept {
			return ::sqlite3_last_insert_rowid(this->_ptr);
		}

		inline void
		last_insert_row_id(int64_t rhs) noexcept {
			::sqlite3_set_last_insert_rowid(this->_ptr, rhs);
		}

		inline void
		attach(std::string path, std::string name) {
			this->attach(path.data(), name.data());
		}

		inline void
		attach(const char* path, const char* name) {
			this->execute("ATTACH DATABASE ? AS ?", path, name);
		}

		inline void vacuum() { this->execute("VACUUM"); }

		inline void
		incremental_vacuum(int npages=0) {
			execute(format("PRAGMA foreign_key_check(%d)", npages).get());
		}

		#define SQLITEX_PRAGMA(field, type, type2, fmt) \
		inline type \
		field(const char* name="main") { \
			type2 value{}; \
			rstream rstr(prepare(format("PRAGMA %Q." #field, name).get())); \
			cstream cstr(rstr); \
			rstr >> cstr; \
			cstr >> value; \
			return static_cast<type>(value); \
		} \
		inline void \
		field(type rhs, const char* name="main") { \
			execute(format("PRAGMA %Q." #field "=" fmt, name, type2(rhs)).get()); \
		}

		#define SQLITEX_PRAGMA_STRING(field, type) \
		inline type \
		field(const char* name="main") { \
			type value{}; \
			u8string str; \
			rstream rstr(prepare(format("PRAGMA %Q." #field, name).get())); \
			cstream cstr(rstr); \
			rstr >> cstr; \
			cstr >> str; \
			str >> value; \
			return value; \
		} \
		inline void \
		field(type rhs, const char* name="main") { \
			execute(format("PRAGMA %Q." #field "=%d", name, to_string(rhs)).get()); \
		}

		SQLITEX_PRAGMA(application_id, std::int32_t, std::int32_t, "%d");
		SQLITEX_PRAGMA(user_version, std::int64_t, std::int64_t, "%d");
		SQLITEX_PRAGMA(auto_vacuum, ::sqlite::auto_vacuum, int, "%d");
		SQLITEX_PRAGMA(automatic_index, bool, int, "%d");
		SQLITEX_PRAGMA(cache_size, int, int, "%d");
		SQLITEX_PRAGMA(cache_spill, int, int, "%d");
		SQLITEX_PRAGMA(case_sensitive_like, bool, int, "%d");
		SQLITEX_PRAGMA(cell_size_check, bool, int, "%d");
		SQLITEX_PRAGMA(checkpoint_fullfsync, bool, int, "%d");
		SQLITEX_PRAGMA(defer_foreign_keys, bool, int, "%d");
		SQLITEX_PRAGMA(encoding, u8string, u8string, "%s");
		SQLITEX_PRAGMA_STRING(journal_mode, ::sqlite::journal_mode);
		SQLITEX_PRAGMA(fullfsync, bool, int, "%d");
		SQLITEX_PRAGMA(ignore_check_constraints, bool, int, "%d");
		SQLITEX_PRAGMA(journal_size_limit, int, int, "%d");
		SQLITEX_PRAGMA(legacy_alter_table, bool, int, "%d");
		SQLITEX_PRAGMA(legacy_file_format, bool, int, "%d");
		SQLITEX_PRAGMA_STRING(locking_mode, ::sqlite::locking_mode);
		SQLITEX_PRAGMA(max_page_count, int, int, "%d");
		SQLITEX_PRAGMA(mmap_size, int, int, "%d");
		SQLITEX_PRAGMA(page_size, int, int, "%d");
		SQLITEX_PRAGMA(query_only, bool, int, "%d");
		SQLITEX_PRAGMA(read_uncommitted, bool, int, "%d");
		SQLITEX_PRAGMA(recursive_triggers, bool, int, "%d");
		SQLITEX_PRAGMA(reverse_unordered_selects, bool, int, "%d");
		SQLITEX_PRAGMA(secure_delete, bool, int, "%d");
		SQLITEX_PRAGMA(synchronous, sync_mode, int, "%d");
		SQLITEX_PRAGMA(temp_store, temp_store_mode, int, "%d");

		#undef SQLITEX_PRAGMA
		#undef SQLITEX_PRAGMA_STRING

		inline rstream collations() { return prepare("PRAGMA collation_list"); }
		inline rstream compile_options() { return prepare("PRAGMA compile_options"); }
		inline rstream attached_databases() { return prepare("PRAGMA database_list"); }
		inline rstream check_foreign_keys() { return prepare("PRAGMA foreign_key_check"); }

		inline rstream
		check_foreign_keys(const char* table) {
			return prepare(format("PRAGMA foreign_key_check(%Q)", table).get());
		}

		inline rstream
		check_integrity(int max_errors=100) {
			return prepare(format("PRAGMA integrity_check(%d)", max_errors).get());
		}

		inline rstream
		check_integrity_fast(int max_errors=100) {
			return prepare(format("PRAGMA quick_check(%d)", max_errors).get());
		}

		inline rstream
		foreign_keys(const char* table) {
			return prepare(format("PRAGMA foreign_key_list(%Q)", table).get());
		}

		inline int64
		data_version() {
			std::int64_t value = 0;
			rstream rstr(prepare("PRAGMA data_version"));
			cstream cstr(rstr);
			rstr >> cstr;
			cstr >> value;
			return value;
		}

		inline int
		num_unused_pages(const char* name="main") {
			int value = 0;
			rstream rstr(prepare(format("PRAGMA %Q.freelist_count", name).get()));
			cstream cstr(rstr);
			rstr >> cstr;
			cstr >> value;
			return value;
		}

		inline int
		num_pages(const char* name="main") {
			int value = 0;
			rstream rstr(prepare(format("PRAGMA %Q.page_count", name).get()));
			cstream cstr(rstr);
			rstr >> cstr;
			cstr >> value;
			return value;
		}

		inline rstream
		index(const char* name, const char* schema="main") {
			return prepare(format("PRAGMA %Q.index_info(%Q)", schema, name).get());
		}

		inline rstream
		indices(const char* table, const char* schema="main") {
			return prepare(format("PRAGMA %Q.index_list(%Q)", schema, table).get());
		}

		inline rstream
		index_columns(const char* name, const char* schema="main") {
			return prepare(format("PRAGMA %Q.index_xinfo(%Q)", schema, name).get());
		}

		inline void
		optimize(uint64 mask=0xfffe,const char* schema="main") {
			execute(format("PRAGMA %Q.optimize(%x)", schema, mask).get());
		}

		inline rstream
		table_columns(const char* table, const char* schema) {
			return prepare(format("PRAGMA %Q.table_info(%Q)", schema, table).get());
		}

		inline rstream
		table_columns_all(const char* table, const char* schema) {
			return prepare(format("PRAGMA %Q.table_xinfo(%Q)", schema, table).get());
		}

		template <class Rep, class Period>
		inline void
		busy_timeout(const std::chrono::duration<Rep,Period>& dur) {
			using namespace std::chrono;
			call(::sqlite3_busy_timeout(
				this->_ptr,
				duration_cast<milliseconds>(dur).count()
			));
		}

		inline void
		busy_handler(types::busy_handler handler, void* data) {
			call(::sqlite3_busy_handler(this->_ptr, handler, data));
		}

		inline int release_memory() { return ::sqlite3_db_release_memory(this->_ptr); }

		inline void
		foreign_keys(int enable) {
			this->configure_int(SQLITE_DBCONFIG_ENABLE_FKEY, enable);
		}

		inline void
		triggers(int enable) {
			this->configure_int(SQLITE_DBCONFIG_ENABLE_TRIGGER, enable);
		}

		inline void
		full_text_search(int enable) {
			this->configure_int(SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER, enable);
		}

		inline void
		extensions(int enable) {
			this->configure_int(SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, enable);
		}

		inline void
		sql_extensions(int enable) {
			call(::sqlite3_enable_load_extension(this->_ptr, enable));
		}

		inline void
		no_checkpoint_on_close(int enable) {
			this->configure_int(SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE, enable);
		}

		inline void
		stable_queries(int enable) {
			this->configure_int(SQLITE_DBCONFIG_ENABLE_QPSG, enable);
		}

		inline void
		explain_triggers(int enable) {
			this->configure_int(SQLITE_DBCONFIG_TRIGGER_EQP, enable);
		}

		inline void
		extended_error_codes(int enable) {
			call(::sqlite3_extended_result_codes(this->_ptr, enable));
		}

		inline void
		lookaside(size_t slot_size, size_t nslots, void* buffer=nullptr) {
			call(::sqlite3_db_config(
				this->_ptr, SQLITE_DBCONFIG_LOOKASIDE,
				buffer, slot_size, nslots
			));
		}

		inline void
		name(const char* name) {
			call(::sqlite3_db_config(this->_ptr, SQLITE_DBCONFIG_MAINDBNAME, name));
		}

		inline void interrupt() { ::sqlite3_interrupt(this->_ptr); }

		inline bool
		is_in_auto_commit_mode() {
			return ::sqlite3_get_autocommit(this->_ptr) != 0;
		}

		inline bool
		transaction_is_active() {
			return !this->is_in_auto_commit_mode();
		}

		template <class Function>
		inline void
		scalar_function(
			Function func,
			const char* name,
			int narguments,
			::sqlite::encoding enc = ::sqlite::encoding::utf8,
			bool deterministic = true
		) {
			int flags = static_cast<int>(enc);
			Function* ptr = new Function(func);
			if (deterministic) { flags |= SQLITE_DETERMINISTIC; }
			call(::sqlite3_create_function_v2(
				this->_ptr, name, narguments, flags, ptr,
				[] (types::context* ctx, int nargs, types::value** args) {
					context c(ctx);
					c.data<Function>()->func(&c, nargs, reinterpret_cast<any_base*>(args));
				},
				nullptr,
				nullptr,
				bits::destroy<Function>
			));
		}

		template <class Function>
		inline void
		scalar_function(
			Function* func,
			const char* name,
			int narguments,
			::sqlite::encoding enc = ::sqlite::encoding::utf8,
			bool deterministic = true
		) {
			int flags = static_cast<int>(enc);
			if (deterministic) { flags |= SQLITE_DETERMINISTIC; }
			call(::sqlite3_create_function_v2(
				this->_ptr, name, narguments, flags, func,
				[] (types::context* ctx, int nargs, types::value** args) {
					context c(ctx);
					c.data<Function>()->func(&c, nargs, reinterpret_cast<any_base*>(args));
				},
				nullptr,
				nullptr,
				nullptr
			));
		}

		template <class Function>
		inline void
		aggregate_function(
			Function* func,
			const char* name,
			int narguments,
			::sqlite::encoding enc = ::sqlite::encoding::utf8,
			bool deterministic = true
		) {
			int flags = static_cast<int>(enc);
			if (deterministic) { flags |= SQLITE_DETERMINISTIC; }
			call(::sqlite3_create_function_v2(
				this->_ptr, name, narguments, flags, func,
				[] (types::context* ctx, int nargs, types::value** args) {
					context c(ctx);
					c.data<Function>()->func(c, nargs, reinterpret_cast<any_base*>(args));
				},
				[] (types::context* ctx, int nargs, types::value** args) {
					context c(ctx);
					c.data<Function>()->step(c, nargs, reinterpret_cast<any_base*>(args));
				},
				[] (types::context* ctx, int nargs, types::value** args) {
					context c(ctx);
					c.data<Function>()->end(c, nargs, reinterpret_cast<any_base*>(args));
				},
				nullptr
			));
		}

		inline void
		scalar_function(
			types::scalar_function func,
			const char* name,
			int narguments,
			::sqlite::encoding enc = ::sqlite::encoding::utf8,
			bool deterministic = true,
			void* ptr = nullptr
		) {
			int flags = static_cast<int>(enc);
			if (deterministic) { flags |= SQLITE_DETERMINISTIC; }
			call(::sqlite3_create_function_v2(
				this->_ptr,
				name,
				narguments,
				flags,
				ptr,
				func,
				nullptr,
				nullptr,
				nullptr
			));
		}

		inline void
		remove_function(
			const char* name,
			int narguments,
			::sqlite::encoding enc=::sqlite::encoding::utf8
		) {
			int flags = static_cast<int>(enc);
			call(::sqlite3_create_function_v2(
				this->_ptr,
				name,
				narguments,
				flags,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr
			));
		}

		template <class Collation>
		inline void
		collation(const u8string& name, Collation coll) {
			using string = typename Collation::string;
			using const_pointer = const typename string::value_type*;
			Collation* copy = new Collation(coll);
			int ret = ::sqlite3_create_collation_v2(
				this->_ptr, name.data(), downcast(Collation::encoding()), copy,
				[] (void* ptr, int n1, const void* s1, int n2, const void* s2) -> int {
					string str1(static_cast<const_pointer>(s1), n1);
					string str2(static_cast<const_pointer>(s2), n2);
					return (*static_cast<Collation*>(ptr))(str1, str2);
				},
				bits::destroy<Collation>
			);
			if (ret != SQLITE_OK) { bits::destroy<Collation>(copy); }
			call(ret);
		}

		inline void
		remove_collation(
			const u8string& name,
			::sqlite::encoding enc=::sqlite::encoding::utf8
		) {
			call(::sqlite3_create_collation_v2(
				this->_ptr,
				name.data(),
				downcast(enc),
				nullptr,
				nullptr,
				nullptr
			));
		}

		inline void
		finalize() {
			types::statement* stmt;
			while ((stmt = ::sqlite3_next_stmt(this->_ptr, nullptr))) {
				::sqlite3_finalize(stmt);
			}
		}

		inline column_metadata
		table_column_metadata(u8string db, u8string table, u8string column) const {
			char const* type = nullptr;
			char const* collation = nullptr;
			int nn = 0, pk = 0, autoinc = 0;
			call(::sqlite3_table_column_metadata(
				this->_ptr,
				db.data(),
				table.data(),
				column.data(),
				&type,
				&collation,
				&nn,
				&pk,
				&autoinc
			));
			column_metadata col;
			col._type.reset(const_cast<char*>(type));
			col._collation.reset(const_cast<char*>(collation));
			col._notnull = nn;
			col._primarykey = pk;
			col._autoincrement = autoinc;
			return col;
		}

		inline void
		load_extension(const char* file, const char* entry_point) {
			call(::sqlite3_load_extension(this->_ptr, file, entry_point, nullptr));
		}

		inline blob_stream
		open_blob(u8string db, u8string table, u8string column, int64 rowid, int flags=0) {
			types::blob* b = nullptr;
			call(::sqlite3_blob_open(
				this->_ptr,
				db.data(),
				table.data(),
				column.data(),
				rowid,
				flags,
				&b
			));
			return blob_stream(b);
		}

		inline statistic
		get(status key, bool reset=false) {
			statistic s;
			call(::sqlite3_db_status(this->_ptr, int(key), &s.current, &s.highwater, reset));
			return s;
		}

		inline errc
		error_code() const {
			return errc(::sqlite3_errcode(this->_ptr));
		}

		inline errc
		extended_error_code() const {
			return errc(::sqlite3_extended_errcode(this->_ptr));
		}

		inline std::errc
		system_error_code() const {
			return std::errc(::sqlite3_system_errno(this->_ptr));
		}

		inline const char* error_message() const { return ::sqlite3_errmsg(this->_ptr); }

		inline u16string
		error_message_utf16() const {
			return reinterpret_cast<const char16_t*>(::sqlite3_errmsg16(this->_ptr));
		}

		inline static_mutex
		mutex() const {
			return static_mutex(::sqlite3_db_mutex(this->_ptr));
		}

		inline int
		limit(::sqlite::limit key, int value) {
			return ::sqlite3_limit(this->_ptr, int(key), value);
		}

		inline int limit(::sqlite::limit key) { return this->limit(key, -1); }

		inline void
		unlock_notify(types::unlock_notify cb, void* ptr=nullptr) {
			call(::sqlite3_unlock_notify(this->_ptr, cb, ptr));
		}

		inline void
		checkpoint_after(int nframes) {
			call(::sqlite3_wal_autocheckpoint(this->_ptr, nframes));
		}

		inline void
		checkpoint(const char* name=nullptr) {
			call(::sqlite3_wal_checkpoint(this->_ptr, name));
		}

		inline checkpoint_result
		checkpoint(const char* name=nullptr, checkpoint_mode mode=checkpoint_mode::passive) {
			checkpoint_result result{};
			call(::sqlite3_wal_checkpoint_v2(
				this->_ptr,
				name,
				downcast(mode),
				&result.nframes,
				&result.ncheckpointed
			));
			return result;
		}

		inline void
		declare_virtual_table(const char* sql) {
			call(::sqlite3_declare_vtab(this->_ptr, sql));
		}

		template <class Table, class Cursor>
		inline void virtual_table(const char* module_name) {
			types::module m{};
			m.iVersion = 2;
			m.xCreate = [] (
				types::database* db,
				void*,
				int argc,
				const char *const* argv,
				types::virtual_table** ptr,
				char**
			) -> int {
				try {
					Table::create(static_database(db), argc, argv);
					*ptr = reinterpret_cast<types::virtual_table*>(
						new Table(static_database(db), argc, argv)
					);
				} catch (const std::bad_alloc& err) {
					return SQLITE_NOMEM;
				}
				return SQLITE_OK;
			};
			m.xConnect = [] (
				types::database* db,
				void*,
				int argc,
				const char *const* argv,
				types::virtual_table** ptr,
				char**
			) -> int {
				try {
					*ptr = reinterpret_cast<types::virtual_table*>(
						new Table(static_database(db), argc, argv)
					);
				} catch (const std::bad_alloc& err) {
					return SQLITE_NOMEM;
				}
				return SQLITE_OK;
			};
			m.xBestIndex = [] (types::virtual_table* ptr, types::index_info* idx) -> int {
				return reinterpret_cast<::sqlite::virtual_table*>(ptr)->best_index(
					static_cast<virtual_table_index*>(idx)
				);
			};
			m.xOpen = [] (
				types::virtual_table* ptr,
				types::virtual_table_cursor** cursor
			) -> int {
				try {
					*cursor = reinterpret_cast<types::virtual_table_cursor*>(
						new Cursor(reinterpret_cast<Table*>(ptr))
					);
				} catch (const std::bad_alloc& err) {
					return SQLITE_NOMEM;
				}
				return SQLITE_OK;
			};
			#define SQLITEX_CURSOR_FIELD(field,method) \
				m.field = [] (types::virtual_table_cursor* ptr) -> int { \
					return reinterpret_cast<Cursor*>(ptr)->method(); \
				}
			SQLITEX_CURSOR_FIELD(xClose, close);
			m.xFilter = [] (
				types::virtual_table_cursor* ptr,
				int idxNum,
				const char* idxStr,
				int argc,
				types::value** argv
			) -> int {
				return reinterpret_cast<Cursor*>(ptr)->filter(
					idxNum, idxStr, argc, reinterpret_cast<any_base*>(argv)
				);
			};
			SQLITEX_CURSOR_FIELD(xNext, next);
			SQLITEX_CURSOR_FIELD(xEof, eof);
			m.xColumn = [] (
				types::virtual_table_cursor* ptr,
				types::context* ctx,
				int n
			) -> int {
				virtual_table_context c(ctx);
				return reinterpret_cast<Cursor*>(ptr)->column(&c, n);
			};
			m.xRowid = [] (
				types::virtual_table_cursor* ptr,
				int64* rowid
			) -> int {
				return reinterpret_cast<Cursor*>(ptr)->rowid(*rowid);
			};
			#undef SQLITEX_CURSOR_FIELD
			#define SQLITEX_TABLE_FIELD(field,method) \
				m.field = [] (types::virtual_table* ptr) -> int { \
					return reinterpret_cast<Table*>(ptr)->method(); \
				}
			SQLITEX_TABLE_FIELD(xDisconnect, disconnect);
			SQLITEX_TABLE_FIELD(xDestroy, destroy);
			SQLITEX_TABLE_FIELD(xBegin, begin);
			SQLITEX_TABLE_FIELD(xSync, sync);
			SQLITEX_TABLE_FIELD(xCommit, commit);
			SQLITEX_TABLE_FIELD(xRollback, rollback);
			m.xRename = [] (types::virtual_table* ptr, const char* name) -> int {
				return reinterpret_cast<Table*>(ptr)->name(name);
			};
			m.xSavepoint = [] (types::virtual_table* ptr, int n) -> int {
				return reinterpret_cast<Table*>(ptr)->savepoint(n);
			};
			m.xRelease = [] (types::virtual_table* ptr, int n) -> int {
				return reinterpret_cast<Table*>(ptr)->release(n);
			};
			m.xRollback = [] (types::virtual_table* ptr, int n) -> int {
				return reinterpret_cast<Table*>(ptr)->rollback(n);
			};
			m.xUpdate = [] (
				types::virtual_table* ptr,
				int argc,
				types::value** argv,
				int64* rowid
			) -> int {
				return reinterpret_cast<Table*>(ptr)->update(
					argc,
					reinterpret_cast<any_base*>(argv),
					*rowid
				);
			};
			m.xFindFunction = [] (
				types::virtual_table* ptr,
				int nargs,
				const char* name,
				types::virtual_table_function* func,
				void** user_data
			) -> int {
				return reinterpret_cast<Table*>(ptr)->find_function(
					nargs,
					name,
					func,
					user_data
				);
			};
			#undef SQLITEX_TABLE_FIELD
			call(::sqlite3_create_module_v2(this->_ptr, module_name, &m, nullptr, nullptr));
		}

		inline void
		overload_function(const char* name, int nargs) {
			call(::sqlite3_overload_function(this->_ptr, name, nargs));
		}

		template <class ... Args>
		inline void configure(virtual_table::key key, Args ... args) {
			call(::sqlite3_vtab_config(this->_ptr, int(key), args...));
		}

		inline void constraints(bool rhs) {
			configure(virtual_table::key::constraint_support, rhs);
		}

		inline ::sqlite::conflict_policy
		conflict_policy() {
			return ::sqlite::conflict_policy(::sqlite3_vtab_on_conflict(this->_ptr));
		}

		inline void
		file(const char* name, file_operation op, void* arg) {
			call(::sqlite3_file_control(this->_ptr, name, downcast(op), arg));
		}

		inline void
		file(file_operation op, void* arg) {
			this->file(nullptr, op, arg);
		}

	private:

		inline int
		do_execute(const char* sql) {
			return ::sqlite3_exec(this->_ptr, sql, nullptr, nullptr, nullptr);
		}

		inline void
		configure_int(int key, int value) {
			call(::sqlite3_db_config(this->_ptr, key, value, nullptr));
		}

	};

	class database: public static_database {

	public:
		using authorizer_type =
			std::function<permission(action,const char*,const char*,const char*,const char*)>;
		using tracer_type = std::function<int(trace,void*,void*)>;
		using progress_type = std::function<int()>;
		using collation_generator_type =
			std::function<void(static_database,::sqlite::encoding,u8string)>;
		using commit_hook_type = std::function<int(static_database,const char*,int)>;

	private:
		authorizer_type _authorizer;
		tracer_type _tracer;
		progress_type _progress;
		collation_generator_type _collation_generator;
		commit_hook_type _commit_hook;

	public:
		inline explicit database(types::database* db): static_database(db) {}

		inline explicit
		database(types::context* ctx):
		static_database(::sqlite3_context_db_handle(ctx)) {}

		inline ~database() { this->close(); }
		database(database&&) = default;
		database& operator=(database&&) = default;
		database(const database&) = delete;
		database& operator=(const database&) = delete;

		inline explicit
		database(
			const char* filename,
			file_flag flags = file_flag::read_write | file_flag::create
		) {
			this->open(filename, flags);
		}

		inline void
		open(
			const char* filename=":memory:",
			file_flag flags = file_flag::read_write | file_flag::create,
			const char* vfs_name=nullptr
		) {
			this->close();
			call(::sqlite3_open_v2(filename, &this->_ptr, int(flags), vfs_name));
		}

		inline void
		open(const u8string& filename) {
			this->close();
			call(::sqlite3_open(filename.data(), &this->_ptr));
		}

		inline void
		open(const u16string& filename) {
			this->close();
			call(::sqlite3_open16(filename.data(), &this->_ptr));
		}

		inline void
		close() {
			if (this->_ptr) {
				call(::sqlite3_close(this->_ptr));
				this->_ptr = nullptr;
			}
		}

		inline void
		close_async() {
			if (this->_ptr) {
				call(::sqlite3_close_v2(this->_ptr));
				this->_ptr = nullptr;
			}
		}

		inline void
		recover(const char* name="main") {
			call(::sqlite3_snapshot_recover(this->_ptr, name));
		}

		inline void
		open(const char* schema, const snapshot& snap) {
			call(::sqlite3_snapshot_open(
				this->_ptr,
				schema,
				const_cast<types::snapshot*>(snap.get())
			));
		}

		inline ::sqlite::snapshot
		snapshot(const char* schema) {
			types::snapshot* ptr = nullptr;
			call(::sqlite3_snapshot_get(this->_ptr, schema, &ptr));
			return ::sqlite::snapshot(ptr);
		}

		inline void
		authorizer(authorizer_type cb) {
			this->_authorizer = cb;
			call(::sqlite3_set_authorizer(
				this->_ptr,
				[] (void* ptr, int a,
					const char* a1, const char* a2, const char* a3, const char* a4
				) -> int {
					auto* db = static_cast<database*>(ptr);
					return static_cast<int>(db->_authorizer(action(a),a1,a2,a3,a4));
				},
				this
			));
		}

		inline void
		tracer(tracer_type cb, trace mask=trace::all) {
			this->_tracer = cb;
			call(::sqlite3_trace_v2(
				this->_ptr,
				static_cast<unsigned>(mask),
				[] (unsigned mask, void* ptr, void* a1, void* a2) -> int {
					auto* db = static_cast<database*>(ptr);
					return static_cast<int>(db->_tracer(trace(mask),a1,a2));
				},
				this
			));
		}

		inline void
		progress(progress_type cb, int ninstructions) {
			this->_progress = cb;
			::sqlite3_progress_handler(
				this->_ptr,
				ninstructions,
				[] (void* ptr) -> int {
					auto* db = static_cast<database*>(ptr);
					return db->_progress();
				},
				this
			);
		}

		inline void
		collation_generator(collation_generator_type cb) {
			this->_collation_generator = cb;
			call(::sqlite3_collation_needed(
				this->_ptr, this,
				[] (void* ptr, types::database* db, int enc, const char* name) {
					static_cast<database*>(ptr)->_collation_generator(
						static_database(db),
						::sqlite::encoding(enc),
						name
					);
				}
			));
		}

		inline void
		collation_generator(std::nullptr_t) {
			call(::sqlite3_collation_needed(this->_ptr, nullptr, nullptr));
		}

		inline void
		on_commit(commit_hook_type cb) {
			this->_commit_hook = cb;
			::sqlite3_wal_hook(
				this->_ptr,
				[] (void* ptr, types::database*, const char* name, int npages) {
					auto* db = static_cast<database*>(ptr);
					return db->_commit_hook(*db, name, npages);
				},
				this
			);
		}

		#if defined(SQLITE_ENABLE_PREUPDATE_HOOK)
		template <class Update>
		inline void
		on_update(Update* ptr) {
			call(::sqlite3_preupdate_hook(
				get(),
				[] (
					void* ptr,
					types::database* db,
					int op,
					char const* dbname,
					char const* table,
					sqlite3_int64 iKey1,
					sqlite3_int64 iKey2
				) {
					preupdate_database db2(db);
					(*reinterpret_cast<Update*>(ptr))(
						&db,
						action(op),
						dbname,
						table,
						rowid1,
						rowid2
					);
				},
				ptr
			));
		}
		#endif

	};

	#if defined(SQLITE_ENABLE_PREUPDATE_HOOK)
	class preupdate_database: public static_database {

	public:
		using static_database::static_database;

		inline int num_columns() { return ::sqlite3_preupdate_count(get()); }
		inline int depth() { return ::sqlite3_preupdate_depth(get()); }

		inline void
		old_values(int n, any_base* values) {
			call(::sqlite3_preupdate_old(get(), n, reinterpret_cast<types::value**>(values)));
		}

		inline void
		new_values(int n, any_base* values) {
			call(::sqlite3_preupdate_new(get(), n, reinterpret_cast<types::value**>(values)));
		}

	};
	#endif

	inline static_database
	context::database() {
		return static_database(::sqlite3_context_db_handle(this->_ptr));
	}

	inline void init() { call(::sqlite3_initialize()); }
	inline void shutdown() { call(::sqlite3_shutdown()); }
	inline void os_init() { call(::sqlite3_os_init()); }
	inline void os_shutdown() { call(::sqlite3_os_end()); }
	inline void shared_cache(bool b) { call(::sqlite3_enable_shared_cache(b)); }

}

#endif // vim:filetype=cpp
