#ifndef SQLITE_EXT_HH
#define SQLITE_EXT_HH

#include <sqlitex/call.hh>

namespace sqlite {

	template <class Function>
	inline void
	load(Function* ptr) {
		typedef void (*entry_point_type)(void);
		call(::sqlite3_auto_extension(reinterpret_cast<entry_point_type>(ptr)));
	}

}

#endif // vim:filetype=cpp
