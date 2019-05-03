#ifndef SQLITE_EXT_HH
#define SQLITE_EXT_HH

#include <sqlitex/call.hh>
#include <sqlitex/forward.hh>

namespace sqlite {

	template <class Function>
	inline void
	load(Function* ptr) {
		call(::sqlite3_auto_extension(reinterpret_cast<entry_point_type>(ptr)));
	}

	template <class Function>
	inline void
	unload(Function* ptr) {
		call(::sqlite3_cancel_auto_extension(reinterpret_cast<entry_point_type>(ptr)));
	}

	inline void unload_all() { ::sqlite3_reset_auto_extension(); }

}

#endif // vim:filetype=cpp
