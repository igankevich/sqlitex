#ifndef SQLITEX_NAMED_PTR_HH
#define SQLITEX_NAMED_PTR_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class named_ptr {

	private:
		void* _ptr = nullptr;
		const char* _name = nullptr;

	public:
		inline named_ptr(void* ptr, const char* name): _ptr(ptr), _name(name) {}
		inline void* get() const { return this->_ptr; }
		inline const char* name() const { return this->_name; }
		named_ptr() = default;
		named_ptr(const named_ptr&) = default;
		named_ptr(named_ptr&&) = default;
		named_ptr& operator=(const named_ptr&) = default;
		named_ptr& operator=(named_ptr&&) = default;
	};

}

#endif // vim:filetype=cpp
