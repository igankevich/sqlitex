#ifndef SQLITEX_SESSION_HH
#define SQLITEX_SESSION_HH

#include <sqlitex/forward.hh>

namespace sqlitex {

	#if defined(SQLITE_ENABLE_SESSION)
	class session {

	private:
		types::session* _ptr = nullptr;

	public:

		inline explicit
		session(types::connection* db, const char* name="main") {
			call(::sqlite3session_create(db, name, &this->_ptr));
		}

		inline ~session() noexcept { this->close(); }
		inline types::session* get() noexcept { return this->_ptr; }
		inline const types::session* get() const noexcept { return this->_ptr; }

		inline void
		close() noexcept {
			::sqlite3session_delete(this->_ptr);
			this->_ptr = nullptr;
		}

		inline void record(bool enable) { call(::sqlite3session_enable(get(), enable)); }
		inline bool indirect() const noexcept { return ::sqlite3session_indirect(get(), -1); }
		inline bool indirect(bool rhs) noexcept { return ::sqlite3session_indirect(get(), rhs); }
		inline bool empty() const noexcept { return ::sqlite3session_isempty(get()); }
		inline void attach(const char* table) { call(::sqlite3session_attach(get(), table)); }

	};
	#endif

}

#endif // vim:filetype=cpp
