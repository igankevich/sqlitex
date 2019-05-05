#ifndef SQLITEX_CONTEXT_HH
#define SQLITEX_CONTEXT_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class context {

	private:
		types::context* _ptr = nullptr;

	public:
		inline context(types::context* ptr): _ptr(ptr) {}
		inline types::context* get() { return this->_ptr; }
		inline static_database database();
		inline void* data() { return ::sqlite3_user_data(this->_ptr); }

		template <class T> inline T*
		data() { return reinterpret_cast<T*>(::sqlite3_user_data(this->_ptr)); }

		inline void*
		temporary_buffer(int size) {
			return ::sqlite3_aggregate_context(this->_ptr, size);
		}

	};

}

#endif // vim:filetype=cpp
