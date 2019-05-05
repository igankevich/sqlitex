#ifndef SQLITEX_ALLOCATOR_BASE_HH
#define SQLITEX_ALLOCATOR_BASE_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class allocator_base {
	public:
		inline void* allocate(int size) { return nullptr; }
		inline void free(void* ptr) {}
		inline void* resize(void* ptr, int size) { return nullptr; }
		inline int size(void* ptr) { return 0; }
		inline int roundup(int size) { return size; }
		inline int init() { return 0; }
		inline void shutdown() {}
	};

}

#endif // vim:filetype=cpp
