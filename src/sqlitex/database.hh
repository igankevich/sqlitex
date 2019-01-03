#ifndef SQLITE_DATABASE_HH
#define SQLITE_DATABASE_HH

#include <sqlitex/call.hh>
#include <sqlitex/rstream.hh>

namespace sqlite {

	typedef ::sqlite3 db_type;

	class database {

	private:
		db_type* _db = nullptr;

	public:

		database() = default;

		inline explicit
		database(const char* filename) {
			this->open(filename);
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
		open(const char* filename) {
			call(::sqlite3_open(filename, &this->_db));
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

		inline const db_type*
		db() const noexcept {
			return this->_db;
		}

		inline db_type*
		db() noexcept {
			return this->_db;
		}

		template <class ... Args>
		inline rstream
		prepare(const std::string& sql, const Args& ... args) {
			statement_type* stmt = nullptr;
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
			this->prepare(sql, args...).step();
		}

		inline void
		execute(const std::string& sql) {
			call(this->do_execute(sql));
		}

		inline int
		num_rows_modified() const noexcept {
			return ::sqlite3_changes(this->_db);
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
			this->execute("ATTACH DATABASE ? AS ?", path, name);
		}

		inline void
		vacuum() {
			this->execute("VACUUM");
		}

		inline void
		user_version(int64_t version) {
			std::string sql;
			sql += "PRAGMA user_version=";
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

		inline void
		swap(database& rhs) {
			std::swap(this->_db, rhs._db);
		}

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


	};

	inline void
	swap(database& lhs, database& rhs) {
		lhs.swap(rhs);
	}

}

#endif // vim:filetype=cpp