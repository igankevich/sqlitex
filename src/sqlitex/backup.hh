#ifndef SQLITEX_BACKUP_HH
#define SQLITEX_BACKUP_HH

#include <sqlitex/database.hh>

namespace sqlite {

	typedef ::sqlite3_backup backup_type;

	class backup {

	private:
		backup_type* _backup = nullptr;
		bool _done = false;

	public:

		inline
		backup(
			database& source,
			database& destination,
			const char* source_name = "main",
			const char* destination_name = "main"
		) {
			this->_backup = ::sqlite3_backup_init(
				destination.db(),
				destination_name,
				source.db(),
				source_name
			);
			if (!this->_backup) {
				throw_error(destination.extended_error_code());
			}
		}

		inline
		~backup() {
			this->finish();
		}

		inline void
		finish() {
			if (this->_backup) {
				call(::sqlite3_backup_finish(this->_backup));
				this->_backup = nullptr;
			}
		}

		inline void
		step(int npages=-1) {
			sqlite_errc ret =
				sqlite_errc(::sqlite3_backup_step(this->_backup, npages));
			if (ret == sqlite_errc::done) {
				this->_done = true;
				return;
			}
			if (ret == sqlite_errc::ok) {
				return;
			}
			throw_error(ret);
		}

		inline bool
		done() const {
			return this->_done;
		}

		inline int
		remaining() const {
			return ::sqlite3_backup_remaining(this->_backup);
		}

		inline int
		total() const {
			return ::sqlite3_backup_pagecount(this->_backup);
		}

	};

}

#endif // vim:filetype=cpp
