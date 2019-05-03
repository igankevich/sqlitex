#ifndef SQLITEX_ALLOCATOR_HH
#define SQLITEX_ALLOCATOR_HH

#include <cstddef>
#include <type_traits>

#include <sqlitex/forward.hh>

namespace sqlite {

	template <class T>
	class allocator {

	public:
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = types::uint64;
		using difference_type = std::make_signed<size_type>::type;
		template <class U>
		struct rebind { using other = allocator<U>; };

	public:

		allocator() = default;
		~allocator() = default;
		allocator(const allocator&) = default;
		allocator& operator=(const allocator&) = default;
		allocator(allocator&&) = default;
		allocator& operator=(allocator&&) = default;
		template <class U> allocator(const allocator<U,F>& rhs) {}

		inline value_type*
		allocate(size_type n) {
			auto ptr = ::sqlite3_malloc64(n*sizeof(value_type));
			if (!ptr) { throw std::bad_alloc("sqlite3_malloc64"); }
			return ptr;
		}

		inline value_type*
		reallocate(size_type n) {
			auto ptr = ::sqlite3_realloc64(n*sizeof(value_type));
			if (!ptr) { throw std::bad_alloc("sqlite3_realloc64"); }
			return ptr;
		}

		inline size_type size(pointer p) const { return ::sqlite3_msize(p); }
		inline value_type* allocate(size_type n, const void*) { return this->allocate(n); }
		inline void deallocate(pointer ptr, size_type) { ::sqlite3_free(ptr); }
		inline void construct(pointer p, const_reference val) { ::new((void *)p) T(val); }

		template <class U, class ... Args>
		inline void
		construct(U* p, Args&&... args) {
			::new((void *)p) U(std::forward<Args>(args)...);
		}

		inline void destroy(pointer p) { ((value_type*)p)->~value_type(); }
		template <class U> inline void destroy(U* p) { p->~U(); }
		inline pointer address(reference x) const { return &x; }
		inline const_pointer address(const_reference x) const { return &x; }
		inline size_type max_size() const { return std::numeric_limits<size_type>::max(); }

	};

}

#endif // vim:filetype=cpp
