#ifndef SQLITEX_CONFIGURE_HH
#define SQLITEX_CONFIGURE_HH

#include <sqlitex/errc.hh>
#include <sqlitex/forward.hh>

namespace sqlite {

	namespace config {

		enum class key: int {
			single_thread=SQLITE_CONFIG_SINGLETHREAD,
			multi_thread=SQLITE_CONFIG_MULTITHREAD,
			serialized=SQLITE_CONFIG_SERIALIZED,
			set_allocator=SQLITE_CONFIG_MALLOC,
			get_allocator=SQLITE_CONFIG_GETMALLOC,
			page_cache=SQLITE_CONFIG_PAGECACHE,
			heap=SQLITE_CONFIG_HEAP,
			memory_status=SQLITE_CONFIG_MEMSTATUS,
			set_mutex=SQLITE_CONFIG_MUTEX,
			get_mutex=SQLITE_CONFIG_GETMUTEX,
			lookaside=SQLITE_CONFIG_LOOKASIDE,
			log=SQLITE_CONFIG_LOG,
			uri=SQLITE_CONFIG_URI,
			set_page_cache=SQLITE_CONFIG_PCACHE2,
			get_page_cache=SQLITE_CONFIG_GETPCACHE2,
			covering_index_scan=SQLITE_CONFIG_COVERING_INDEX_SCAN,
			sql_log=SQLITE_CONFIG_SQLLOG,
			mmap_size=SQLITE_CONFIG_MMAP_SIZE,
			win32_heap_size=SQLITE_CONFIG_WIN32_HEAPSIZE,
			page_cache_header_size=SQLITE_CONFIG_PCACHE_HDRSZ,
			pma_size=SQLITE_CONFIG_PMASZ,
			#if defined(SQLITE_CONFIG_STMTJRNL_SPILL)
			statement_journal_spill=SQLITE_CONFIG_STMTJRNL_SPILL,
			#endif
			#if defined(SQLITE_CONFIG_SMALL_MALLOC)
			small_malloc=SQLITE_CONFIG_SMALL_MALLOC,
			#endif
		};

		template <class ... Args>
		inline void configure(config::key key, Args ... args) {
			call(::sqlite3_config(int(key), args...));
		}

		inline void multiple_threads() { configure(key::multi_thread); }
		inline void single_thread() { configure(key::single_thread); }
		inline void serialise() { configure(key::serialized); }

		template <class Allocator>
		inline void
		allocator(Allocator* ptr) {
			static Allocator* obj = ptr;
			types::allocator_methods m{};
			m.xMalloc = [](int size) { return obj->allocate(size); };
			m.xFree = [](void* ptr) { return obj->free(ptr); };
			m.xRealloc = [](void* ptr, int size) { return obj->resize(ptr, size); };
			m.xSize = [](void* ptr) { return obj->size(ptr); };
			m.xRoundup = [](int size) { return obj->roundup(size); };
			m.xInit = [](void* ptr) {
				obj = reinterpret_cast<Allocator*>(ptr);
				return obj->init();
			};
			m.xShutdown = [](void* ptr) { reinterpret_cast<Allocator*>(ptr)->shutdown(); };
			m.pAppData = ptr;
			configure(key::set_allocator, &m);
		}

		inline types::allocator_methods
		allocator_methods() {
			types::allocator_methods m{};
			configure(key::get_allocator, &m);
			return m;
		}

		#if defined(SQLITE_CONFIG_SMALL_MALLOC)
		inline void small_memory_allocations(bool b) { configure(key::small_malloc, b); }
		#endif

		inline void memory_statistics(bool b) { configure(key::memory_status, b); }
		inline void
		page_cache(int line_size, int nlines, void* ptr=nullptr) {
			configure(key::page_cache, ptr, line_size, nlines);
		}
		inline int
		page_cache_header_size() {
			int n = 0; configure(key::page_cache_header_size, &n); return n;
		}
		inline void
		heap(int size, int min_allocation_size, void* ptr=nullptr) {
			configure(key::heap, ptr, size, min_allocation_size);
		}

		template <class Mutex>
		inline void
		mutex() {
			types::mutex_methods m{};
			m.xMutexInit = []() { return Mutex::init(); };
			m.xMutexEnd = []() { return Mutex::shutdown(); };
			m.xMutexAlloc = [](int type) {
				return reinterpret_cast<types::mutex*>(new Mutex(type));
			};
			m.xMutexFree = [](types::mutex* ptr) { delete reinterpret_cast<Mutex*>(ptr); };
			m.xMutexEnter = [](types::mutex* ptr) { reinterpret_cast<Mutex*>(ptr)->lock(); };
			m.xMutexLeave = [](types::mutex* ptr) { reinterpret_cast<Mutex*>(ptr)->unlock(); };
			m.xMutexTry = [](types::mutex* ptr) {
				return reinterpret_cast<Mutex*>(ptr)->try_lock() ? SQLITE_OK : SQLITE_BUSY;
			};
			m.xMutexHeld = [](types::mutex* ptr) -> int {
				return reinterpret_cast<Mutex*>(ptr)->held();
			};
			m.xMutexNotheld = [](types::mutex* ptr) -> int {
				return reinterpret_cast<Mutex*>(ptr)->not_held();
			};
			configure(key::set_mutex, &m);
		}

		inline types::mutex_methods
		mutex_methods() {
			types::mutex_methods m{};
			configure(key::get_mutex, &m);
			return m;
		}

		inline void
		lookaside(int buffer_size, int nslots) {
			configure(key::lookaside, buffer_size, nslots);
		}

		template <class Cache>
		inline void
		page_cache(Cache* ptr, int version) {
			static Cache* obj = ptr;
			types::allocator_methods m{};
			m.iVersion = version;
			m.pArg = nullptr;
			m.xInit = [](void*) -> int { return Cache::init(); };
			m.xShutdown = [](void*) { Cache::shutdown(); };
			m.xCreate = [](int szPage, int szExtra, int bPurgeable) -> types::page_cache* {
				return reinterpret_cast<types::page_cache*>(
					new Cache(szPage, szExtra, bPurgeable)
				);
			};
			m.xCachesize = [](types::page_cache* ptr, int size) {
				reinterpret_cast<Cache*>(ptr)->size(size);
			};
			m.xPagecount = [](types::page_cache* ptr) -> int {
				return reinterpret_cast<Cache*>(ptr)->num_pages();
			};
			m.xFetch = [](
				types::page_cache* ptr,
				unsigned key,
				int createFlag
			) -> types::page* {
				return reinterpret_cast<Cache*>(ptr)->fetch(key, createFlag);
			}
			m.xUnpin = [](types::page_cache* ptr, types::page* ptr2, int discard) {
				return reinterpret_cast<Cache*>(ptr)->unpin(ptr, ptr2, discard);
			}
			m.xRekey = [](
				types::page_cache* ptr,
				types::page* ptr2,
				unsigned oldKey,
				unsigned newKey
			) {
				return reinterpret_cast<Cache*>(ptr)->rekey(ptr, ptr2, oldKey, newKey);
			}
			m.xTruncate = [](types::page_cache* ptr, unsigned size) {
				reinterpret_cast<Cache*>(ptr)->truncate(size);
			};
			m.xDestroy = [](types::page_cache* ptr) { delete reinterpret_cast<Cache*>(ptr); };
			m.xShrink = [](types::page_cache* ptr) {
				reinterpret_cast<Cache*>(ptr)->shrink();
			};
			configure(key::set_page_cache, &m);
		}

		inline types::mutex_methods
		page_cache_methods() {
			types::page_cache_methods m{};
			configure(key::get_page_cache, &m);
			return m;
		}

		inline void
		log(log_callback func=nullptr, void* ptr=nullptr) {
			configure(key::log, func, ptr);
		}

		inline void
		sql_log(sql_log_callback func=nullptr, void* ptr=nullptr) {
			configure(key::sql_log, func, ptr);
		}

		inline void uri(bool b) { configure(key::uri, b); }
		inline void covering_index_scan(bool b) { configure(key::covering_index_scan, b); }
		inline void memory_map(int64 size, int64 max_size) {
			configure(key::mmap_size, size, max_size);
		}
		inline void win32_heap_size(int size) { configure(key::win32_heap_size, size); }
		inline void pma_size(unsigned int size) { configure(key::pma_size, size); }

		#if defined(SQLITE_CONFIG_STMTJRNL_SPILL)
		inline void
		statement_journal_spill_to_disk_threshold(int threshold) {
			configure(key::statement_journal_spill, threshold);
		}
		#endif

	}

	using config::configure;

}

#endif // vim:filetype=cpp
