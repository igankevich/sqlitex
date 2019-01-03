#ifndef VTESTBED_SQLITE_ERROR_CATEGORY_CC
#define VTESTBED_SQLITE_ERROR_CATEGORY_CC

#include "error_category.hh"

namespace sqlite {

	sqlite_error_category sqlite_category;

}

std::error_condition
sqlite::sqlite_error_category::default_error_condition(int ev) const noexcept {
	return std::error_condition(ev, sqlite_category);
}

#endif // vim:filetype=cpp
