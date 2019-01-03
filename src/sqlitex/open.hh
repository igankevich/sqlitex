#ifndef SQLITEX_OPEN_HH
#define SQLITEX_OPEN_HH

#include <sqlite3.h>

namespace sqlite {

	enum class open_flag: int {
		read_only = SQLITE_OPEN_READONLY,
		read_write = SQLITE_OPEN_READWRITE,
		create = SQLITE_OPEN_CREATE,
		no_mutex = SQLITE_OPEN_NOMUTEX,
		full_mutex = SQLITE_OPEN_FULLMUTEX,
		shared_cache = SQLITE_OPEN_SHAREDCACHE,
		private_cache = SQLITE_OPEN_PRIVATECACHE,
		uri = SQLITE_OPEN_URI,
	};

	inline open_flag
	operator|(open_flag a, open_flag b) {
		return open_flag(int(a) | int(b));
	}

}

#endif // vim:filetype=cpp
