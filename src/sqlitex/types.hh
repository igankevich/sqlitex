#ifndef SQLITE_TYPES_HH
#define SQLITE_TYPES_HH

#include <memory>
#include <string>

#include <sqlitex/forward.hh>

namespace sqlite {

	struct sqlite_deleter {
		inline void
		operator()(void* ptr) {
			::sqlite3_free(ptr);
		}
	};

	template <class T>
	using unique_ptr = std::unique_ptr<T,sqlite_deleter>;

	struct blob: public std::string {

		using std::string::string;

		blob() = default;
		~blob() = default;
		blob(blob&&) = default;
		blob(const blob&) = default;
		blob& operator=(blob&&) = default;
		blob& operator=(const blob&) = default;

		inline
		blob(std::string&& rhs):
		std::string(std::forward<std::string>(rhs)) {}

		inline void*
		ptr() {
			return &this->std::string::operator[](0);
		}

		inline const void*
		ptr() const {
			return this->std::string::data();
		}

	};

}

#endif // vim:filetype=cpp
