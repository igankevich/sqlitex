#ifndef SQLITEX_VFS_HH
#define SQLITEX_VFS_HH

#include <sqlite/forward.hh>

namespace sqlite {

	enum class access_flags: int {
		exists=SQLITE_ACCESS_EXISTS,
		read_write=SQLITE_ACCESS_READWRITE,
		read=SQLITE_ACCESS_READ,
	};

	class vfs {

	public:

		static inline int max_path_size() const { return 0; }
		static inline const char* name() const { return nullptr; }

		inline int open(const char* name, types::file*, file_flag flags, int *pOutFlags) { return 0; }
		inline int remove(const char* name, int syncDir) { return 0; }
		inline int access(const char* name, access_flags flags, int *pResOut) { return 0; }
		inline int full_path(const char* name, int nOut, char *zOut) {}
		inline void* library_open(const char* name) { return nullptr; }
		inline void library_error(int nByte, char *zErrMsg) {}
		inline types::symbol library_symbol(void*, const char *zSymbol) { return nullptr; }
		inline void library_close(void*) {}
		inline int random(int nByte, char *zOut) { return 0; }
		inline int sleep(std::chrono::microseconds amount) { return 0; }
		inline int time(double*) { return 0; }
		inline int time(int64*) { return 0; }
		inline int last_error(int, char *) { return 0; }
		inline int system_call(const char* name, types::syscall_ptr ptr) { return 0; }
		inline types::syscall_ptr system_call(const char* name) { return nullptr; }
		inline const char* next_system_call(const char* name) { return nullptr; }

		static inline types::vfs* find(const char* name) { return ::sqlite3_vfs_find(name); }

		static inline void
		add(types::vfs* vfs, bool make_default=false) {
			call(::sqlite3_vfs_register(vfs, make_default));
		}

		static inline void remove(types::vfs* vfs) { call(::sqlite3_vfs_unregister(vfs)); }

	};

	template <class FS, class File>
	inline types::vfs
	make_vfs(FS* fs) {
		types::vfs m{};
		m.iVersion = 3;
		m.szOsFile = sizeof(File);
		m.mxPathname = FS::max_path_size();
		m.zName = FS::name();
		m.pAppData = fs;
		m.xOpen = [] (
			types::vfs* ptr,
			const char* name,
			types::file* file,
			int flags,
			int *pOutFlags
		) -> int {
			return reinterpret_cast<FS*>(ptr)->open(name, file, file_flag(flags), pOutFlags);
		};
		m.xDelete = [] (types::vfs* ptr, const char* name, int syncDir) -> int {
			return reinterpret_cast<FS*>(ptr)->remove(name, syncDir);
		};
		m.xAccess = [] (types::vfs* ptr, const char* name, int flags, int* pResOut) -> int {
			return reinterpret_cast<FS*>(ptr)->remove(name, access_flags(flags), pResOut);
		};
		m.xFullPathname = [] (types::vfs* ptr, const char* name, int nOut, char* zOut) -> int {
			return reinterpret_cast<FS*>(ptr)->remove(name, nOut, zOut);
		};
		m.xDlOpen = [] (types::vfs* ptr, const char* name) -> void* {
			return reinterpret_cast<FS*>(ptr)->library_open(name);
		};
		m.xDlError = [] (types::vfs* ptr, int nByte, char* zErrMsg) -> void {
			return reinterpret_cast<FS*>(ptr)->library_error(nByte, zErrMsg);
		};
		m.xDlSym = [] (types::vfs* ptr, void* library, const char* name) -> types::symbol {
			return reinterpret_cast<FS*>(ptr)->library_symbol(library, name);
		};
		m.xDlClose = [] (types::vfs* ptr, void* library) -> void {
			return reinterpret_cast<FS*>(ptr)->library_close(library);
		};
		m.xRandomness = [] (types::vfs* ptr, int nByte, char* zOut) -> int {
			return reinterpret_cast<FS*>(ptr)->random(nByte, zOut);
		};
		m.xSleep = [] (types::vfs* ptr, int amount) -> int {
			return reinterpret_cast<FS*>(ptr)->sleep(std::chrono::microseconds(amount));
		};
		m.xCurrentTime = [] (types::vfs* ptr, double* ret) -> int {
			return reinterpret_cast<FS*>(ptr)->time(ret);
		};
		m.xGetLastError = [] (types::vfs* ptr, int n, char* s) -> int {
			return reinterpret_cast<FS*>(ptr)->last_error(n, s);
		};
		m.xCurrentTimeInt64 = [] (types::vfs* ptr, int64* ret) -> int {
			return reinterpret_cast<FS*>(ptr)->time(ret);
		};
		m.xSetSystemCall = [] (
			types::vfs* ptr,
			const char* name,
			types::syscall_ptr ptr
		) -> int {
			return reinterpret_cast<FS*>(ptr)->system_call(name, ptr);
		};
		m.xGetSystemCall = [] (types::vfs* ptr, const char* name) -> types::syscall_ptr {
			return reinterpret_cast<FS*>(ptr)->system_call(name);
		};
		m.xNextSystemCall = [] (types::vfs* ptr, const char* name) -> const char* {
			return reinterpret_cast<FS*>(ptr)->next_system_call(name);
		};
		return m;
	}

}

#endif // vim:filetype=cpp
