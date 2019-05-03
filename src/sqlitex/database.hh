#ifndef SQLITE_DATABASE_HH
#define SQLITE_DATABASE_HH

#include <chrono>

#include <sqlitex/call.hh>
#include <sqlitex/encoding.hh>
#include <sqlitex/forward.hh>
#include <sqlitex/mutex.hh>
#include <sqlitex/open.hh>
#include <sqlitex/rstream.hh>

namespace sqlite {

	class database {

	private:
		types::database* _db = nullptr;

	public:

		database() = default;

		inline explicit
		database(
			const char* filename,
			open_flag flags = open_flag::read_write | open_flag::create
		) {
			this->open(filename, flags);
		}

		inline
		~database() {
			this->close();
		}

		inline
		database(database&& rhs):
		_db(rhs._db) {
			rhs._db = nullptr;
		}

		inline database&
		operator=(database&& rhs) {
			this->swap(rhs);
			return *this;
		}

		database(const database&) = delete;
		database& operator=(const database&) = delete;

		inline void
		open(
			const char* filename,
			open_flag flags = open_flag::read_write | open_flag::create
		) {
			this->close();
			call(::sqlite3_open_v2(filename, &this->_db, int(flags), nullptr));
		}

		/**
		\brief Executes pragmas and closes database connection.
		\date 2018-10-08
		\author Ivan Gankevich
		\details
		\see \link https://www.sqlite.org/pragma.html#pragma_optimize\endlink
		\arg Executes <code>PRAGMA optimize</code>.
		*/
		inline void
		close() {
			if (this->_db) {
				this->do_execute("PRAGMA optimize");
				call(::sqlite3_close(this->_db));
				this->_db = nullptr;
			}
		}

		inline void flush() { call(::sqlite3_db_cacheflush(this->_db)); }
		inline const types::database* db() const { return this->_db; }
		inline types::database* db() { return this->_db; }

		inline const char*
		filename(const char* name="main") const {
			return ::sqlite3_db_filename(this->_db, name);
		}

		inline bool
		read_only(const char* name="main") const {
			return ::sqlite3_db_readonly(this->_db, name) != 0;
		}

		inline void*
		commit_hook(types::commit_hook rhs, void* ptr=nullptr) {
			return ::sqlite3_commit_hook(this->_db, rhs, ptr);
		}

		inline void*
		rollback_hook(types::rollback_hook rhs, void* ptr=nullptr) {
			return ::sqlite3_rollback_hook(this->_db, rhs, ptr);
		}

		inline void*
		update_hook(types::update_hook rhs, void* ptr=nullptr) {
			return ::sqlite3_update_hook(this->_db, rhs, ptr);
		}

		template <class ... Args>
		inline rstream
		prepare(const std::string& sql, const Args& ... args) {
			types::statement* stmt = nullptr;
			call(
				::sqlite3_prepare_v2(
					this->_db,
					sql.data(),
					sql.size(),
					&stmt,
					nullptr
				)
			);
			rstream rstr(stmt);
			bind(rstr, 1, args...);
			return rstr;
		}

		template <class ... Args>
		inline void
		execute(const std::string& sql, const Args& ... args) {
			rstream&& rstr = this->prepare(sql, args...);
			rstr.step();
			rstr.close();
		}

		inline void
		execute(const std::string& sql) {
			call(this->do_execute(sql));
		}

		inline int
		num_rows_modified() const noexcept {
			return ::sqlite3_changes(this->_db);
		}

		inline int
		total_rows_modified() const noexcept {
			return ::sqlite3_total_changes(this->_db);
		}

		inline int64_t
		last_insert_row_id() const noexcept {
			return ::sqlite3_last_insert_rowid(this->_db);
		}

		inline void
		last_insert_row_id(int64_t rhs) noexcept {
			::sqlite3_set_last_insert_rowid(this->_db, rhs);
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
		user_version(int64_t version, const char* name="main") {
			std::string sql;
			sql += "PRAGMA ";
			sql += name;
			sql += ".user_version=";
			sql += std::to_string(version);
			this->execute(sql);
		}

		inline int64_t
		user_version() {
			int64_t version = 0;
			rstream rstr{this->prepare("PRAGMA user_version")};
			cstream cstr(rstr);
			rstr >> cstr;
			cstr >> version;
			return version;
		}

		inline int64_t
		user_version(const char* name) {
			std::string sql;
			sql += "PRAGMA ";
			sql += name;
			sql += ".user_version";
			int64_t version = 0;
			rstream rstr{this->prepare(sql.data())};
			cstream cstr(rstr);
			rstr >> cstr;
			cstr >> version;
			return version;
		}

		template <class Rep, class Period>
		inline void
		busy_timeout(const std::chrono::duration<Rep,Period>& dur) {
			using namespace std::chrono;
			call(::sqlite3_busy_timeout(
				this->_db,
				duration_cast<milliseconds>(dur).count()
			));
		}

		inline void
		busy_handler(types::busy_handler handler, void* data) {
			call(::sqlite3_busy_handler(this->_db, handler, data));
		}

		inline int release_memory() { return ::sqlite3_db_release_memory(this->_db); }

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
			call(::sqlite3_enable_load_extension(this->_db, enable));
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
			call(::sqlite3_extended_result_codes(this->_db, enable));
		}

		inline void
		lookaside(size_t slot_size, size_t nslots, void* buffer=nullptr) {
			call(::sqlite3_db_config(
				this->_db, SQLITE_DBCONFIG_LOOKASIDE,
				buffer, slot_size, nslots
			));
		}

		inline void
		name(const char* name) {
			call(::sqlite3_db_config(this->_db, SQLITE_DBCONFIG_MAINDBNAME, name));
		}

		inline void interrupt() { ::sqlite3_interrupt(this->_db); }

		inline bool
		is_in_auto_commit_mode() {
			return ::sqlite3_get_autocommit(this->_db) != 0;
		}

		inline bool
		transaction_is_active() {
			return !this->is_in_auto_commit_mode();
		}

		inline void
		scalar_function(
			types::scalar_function func,
			const char* name,
			int narguments,
			encoding enc = encoding::utf8,
			bool deterministic = true,
			void* ptr = nullptr
		) {
			int flags = static_cast<int>(enc);
			if (deterministic) {
				flags |= SQLITE_DETERMINISTIC;
			}
			call(::sqlite3_create_function_v2(
				this->_db,
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
			encoding enc = encoding::utf8
		) {
			int flags = static_cast<int>(enc);
			call(::sqlite3_create_function_v2(
				this->_db,
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

		inline sqlite_errc
		error_code() const {
			return sqlite_errc(::sqlite3_errcode(this->_db));
		}

		inline sqlite_errc
		extended_error_code() const {
			return sqlite_errc(::sqlite3_extended_errcode(this->_db));
		}

		inline std::errc
		system_error_code() const {
			return std::errc(::sqlite3_system_errno(this->_db));
		}

		inline const char* error_message() const { return ::sqlite3_errmsg(this->_db); }

		inline static_mutex
		mutex() const {
			return static_mutex(::sqlite3_db_mutex(this->_db));
		}

		inline int
		limit(::sqlite::limit key, int value) {
			return ::sqlite3_limit(this->_db, int(key), value);
		}

		inline int limit(::sqlite::limit key) { return this->limit(key, -1); }

		inline void
		swap(database& rhs) {
			std::swap(this->_db, rhs._db);
		}

	protected:
		inline void reset() { this->_db = nullptr; }
		inline explicit database(types::database* db): _db(db) {}

	private:

		inline int
		do_execute(const std::string& sql) {
			return ::sqlite3_exec(
				this->_db,
				sql.data(),
				nullptr,
				nullptr,
				nullptr
			);
		}

		inline void configure_int(int key, int value) {
			call(::sqlite3_db_config(this->_db, key, value, nullptr));
		}

	};

	inline void swap(database& lhs, database& rhs) { lhs.swap(rhs); }

	class static_database: public database {
	public:
		inline explicit static_database(types::database* db): database(db) {}
		inline ~static_database() { this->reset(); }
		static_database(static_database&&) = default;
	};

}

#endif // vim:filetype=cpp
