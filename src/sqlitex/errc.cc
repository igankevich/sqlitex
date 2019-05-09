#include <sqlitex/errc.hh>

namespace sqlite {

	error_category sqlite_category;

}

const char*
sqlite::error_category::name() const noexcept {
	return "sqlite";
}

std::string
sqlite::error_category::message(int ev) const noexcept {
	return ::sqlite3_errstr(ev);
}
