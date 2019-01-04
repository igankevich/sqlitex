#ifndef SQLITEX_ENCODING_HH
#define SQLITEX_ENCODING_HH

#include <sqlite3.h>

namespace sqlite {

	enum class encoding {
		utf8 = SQLITE_UTF8,
		utf16 = SQLITE_UTF16,
		utf16be = SQLITE_UTF16BE,
		utf16le = SQLITE_UTF16LE,
	};

}

#endif // vim:filetype=cpp
