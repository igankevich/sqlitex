#ifndef SQLITE_CALL_HH
#define SQLITE_CALL_HH

#include <sqlitex/error_category.hh>

namespace sqlite {

	inline errc
	call(int ret) {
		if (ret != int(errc::ok)) {
			throw std::system_error(ret, sqlite_category);
		}
		return errc(ret);
	}

	inline void
	throw_error(errc err) {
		throw std::system_error(int(err), sqlite_category);
	}


}

#endif // vim:filetype=cpp
