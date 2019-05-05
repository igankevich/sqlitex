#ifndef SQLITEX_URI_HH
#define SQLITEX_URI_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class uri {

	public:
		using const_pointer = const char*;

	private:
		const_pointer _ptr=nullptr;

	public:
		inline explicit uri(const_pointer ptr): _ptr(ptr) {}

		inline const_pointer
		string(const_pointer name) const {
			return ::sqlite3_uri_parameter(this->_ptr, name);
		}

		inline bool
		boolean(const_pointer name) const {
			return ::sqlite3_uri_boolean(this->_ptr, name);
		}

		inline int64
		integer(const_pointer name) const {
			return ::sqlite3_uri_int64(this->_ptr, name);
		}

	};

}

#endif // vim:filetype=cpp
