#ifndef SQLITEX_MUTEX_HH
#define SQLITEX_MUTEX_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class mutex {

	public:
		enum mutex_kind {
			#if defined(SQLITE_MUTEX_FAST)
			fast=SQLITE_MUTEX_FAST,
			#endif
			#if defined(SQLITE_MUTEX_RECURSIVE)
			recursive=SQLITE_MUTEX_RECURSIVE,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_MASTER)
			static_master=SQLITE_MUTEX_STATIC_MASTER,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_MEM)
			static_mem=SQLITE_MUTEX_STATIC_MEM,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_MEM2)
			static_mem2=SQLITE_MUTEX_STATIC_MEM2,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_OPEN)
			static_open=SQLITE_MUTEX_STATIC_OPEN,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_PRNG)
			static_prng=SQLITE_MUTEX_STATIC_PRNG,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_LRU)
			static_lru=SQLITE_MUTEX_STATIC_LRU,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_LRU2)
			static_lru2=SQLITE_MUTEX_STATIC_LRU2,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_PMEM)
			static_pmem=SQLITE_MUTEX_STATIC_PMEM,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_APP1)
			static_app1=SQLITE_MUTEX_STATIC_APP1,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_APP2)
			static_app2=SQLITE_MUTEX_STATIC_APP2,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_APP3)
			static_app3=SQLITE_MUTEX_STATIC_APP3,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_VFS1)
			static_vfs1=SQLITE_MUTEX_STATIC_VFS1,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_VFS2)
			static_vfs2=SQLITE_MUTEX_STATIC_VFS2,
			#endif
			#if defined(SQLITE_MUTEX_STATIC_VFS3)
			static_vfs3=SQLITE_MUTEX_STATIC_VFS3,
			#endif
		};

	private:
		types::mutex* _ptr = nullptr;

	public:

		inline explicit
		mutex(mutex_kind kind=fast): _ptr(::sqlite3_mutex_alloc(kind)) {}
		inline ~mutex() { ::sqlite3_mutex_free(this->_ptr); }
		inline void lock() { ::sqlite3_mutex_enter(this->_ptr); }
		inline void unlock() { ::sqlite3_mutex_leave(this->_ptr); }
		inline bool try_lock() { return ::sqlite3_mutex_try(this->_ptr) == SQLITE_OK; }
		#ifndef NDEBUG
		inline bool held() { return ::sqlite3_mutex_held(this->_ptr) != 0; }
		inline bool not_held() { return ::sqlite3_mutex_held(this->_ptr) != 0; }
		#endif

		mutex(const mutex&) = delete;
		mutex& operator=(const mutex&) = delete;

		inline mutex(mutex&& rhs): _ptr(rhs._ptr) { rhs._ptr = nullptr; }
		inline mutex& operator=(mutex&& rhs) { this->swap(rhs); return *this; }
		inline void swap(mutex& rhs) { std::swap(this->_ptr, rhs._ptr); }

	protected:
		inline explicit mutex(types::mutex* ptr): _ptr(ptr) {}
		inline void reset() { this->_ptr = nullptr; }

	};

	inline void swap(mutex& lhs, mutex& rhs) { lhs.swap(rhs); }

	class recursive_mutex: public mutex {
	public:
		inline recursive_mutex(): mutex(recursive) {}

		recursive_mutex(recursive_mutex&&) = default;
		recursive_mutex& operator=(recursive_mutex&&) = default;
		recursive_mutex(const recursive_mutex&) = delete;
		recursive_mutex& operator=(const recursive_mutex&) = delete;

		recursive_mutex(mutex&&) = delete;
		recursive_mutex& operator=(mutex&&) = delete;
		recursive_mutex(const mutex&) = delete;
		recursive_mutex& operator=(const mutex&) = delete;
	};

	class static_mutex: public mutex {

	public:
		inline explicit static_mutex(mutex_kind kind): mutex(kind) {}
		inline explicit static_mutex(types::mutex* ptr): mutex(ptr) {}
		inline ~static_mutex() { this->reset(); }
		static_mutex(static_mutex&&) = default;
		static_mutex& operator=(static_mutex&&) = default;
		static_mutex(const static_mutex&) = delete;
		static_mutex& operator=(const static_mutex&) = delete;

	};

}

#endif // vim:filetype=cpp
