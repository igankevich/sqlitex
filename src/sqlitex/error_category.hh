#ifndef SQLITE_ERROR_CATEGORY_HH
#define SQLITE_ERROR_CATEGORY_HH

#include <sqlitex/errc.hh>

namespace sqlite {

	class sqlite_error_category: public std::error_category {

	public:
		const char*
		name() const noexcept override {
			return "sqlite";
		}

		inline std::error_condition
		default_error_condition(int ev) const noexcept override;

		bool
		equivalent(const std::error_code& code, int condition)
		const noexcept override {
			return *this==code.category() && code.value() == condition;
		}

		std::string
		message(int ev) const noexcept override {
			return ::sqlite3_errstr(ev);
		}

	};

	extern sqlite_error_category sqlite_category;

}

namespace std {
	inline error_condition
	make_error_condition(sqlite::sqlite_errc e) noexcept {
		return std::error_condition(
			static_cast<int>(e),
			sqlite::sqlite_category
		);
	}
}

#endif // vim:filetype=cpp
