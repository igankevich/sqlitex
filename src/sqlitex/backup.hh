#ifndef SQLITEX_BACKUP_HH
#define SQLITEX_BACKUP_HH

#include <sqlitex/database.hh>
#include <sqlitex/forward.hh>

namespace sqlite {

	class backup {

	private:
		types::backup* _backup = nullptr;
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
				destination.get(),
				destination_name,
				source.get(),
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
			errc ret =
				errc(::sqlite3_backup_step(this->_backup, npages));
			if (ret == errc::done) {
				this->_done = true;
				return;
			}
			if (ret == errc::ok) {
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

		inline types::backup* get() { return this->_backup; }
		inline const types::backup* get() const { return this->_backup; }

	};

}

#endif // vim:filetype=cpp
