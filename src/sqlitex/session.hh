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
		session(types::database* db, const char* name="main") {
			call(::sqlite3session_create(db, name, &this->_ptr));
		}

		inline ~session() { this->close(); }
		inline types::session* get() { return this->_ptr; }
		inline const types::session* get() const { return this->_ptr; }

		inline void
		close() {
			::sqlite3session_delete(this->_ptr);
			this->_ptr = nullptr;
		}

		inline void record(bool enable) { call(::sqlite3session_enable(get(), enable)); }
		inline bool indirect() const { return ::sqlite3session_indirect(get(), -1); }
		inline bool indirect(bool rhs) { return ::sqlite3session_indirect(get(), rhs); }
		inline bool empty() const { return ::sqlite3session_isempty(get()); }
		inline void attach(const char* table) { call(::sqlite3session_attach(get(), table)); }

	};
	#endif

}

#endif // vim:filetype=cpp
