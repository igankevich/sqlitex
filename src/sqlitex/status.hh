#ifndef SQLITEX_STATUS_HH
#define SQLITEX_STATUS_HH

#include <sqlitex/call.hh>
#include <sqlitex/forward.hh>

namespace sqlite {

	enum class status: int {
		memory_used=SQLITE_STATUS_MEMORY_USED,
		pagecache_used=SQLITE_STATUS_PAGECACHE_USED,
		pagecache_overflow=SQLITE_STATUS_PAGECACHE_OVERFLOW,
		scratch_used=SQLITE_STATUS_SCRATCH_USED,
		scratch_overflow=SQLITE_STATUS_SCRATCH_OVERFLOW,
		malloc_size=SQLITE_STATUS_MALLOC_SIZE,
		parser_stack=SQLITE_STATUS_PARSER_STACK,
		pagecache_size=SQLITE_STATUS_PAGECACHE_SIZE,
		scratch_size=SQLITE_STATUS_SCRATCH_SIZE,
		malloc_count=SQLITE_STATUS_MALLOC_COUNT,
	};

	inline statistic
	get(status key, bool reset=false) {
		statistic s;
		call(::sqlite3_status64(int(key), &s.current, &s.highwater, reset));
		return s;
	}

}

#endif // vim:filetype=cpp
