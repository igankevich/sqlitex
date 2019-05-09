#ifndef SQLITE_ERRC_HH
#define SQLITE_ERRC_HH

#include <sqlite3.h>

#include <system_error>

namespace sqlite {

	enum struct errc: int {
		// primary result codes
		ok = SQLITE_OK,
		error = SQLITE_ERROR,
		internal = SQLITE_INTERNAL,
		perm = SQLITE_PERM,
		abort = SQLITE_ABORT,
		busy = SQLITE_BUSY,
		locked = SQLITE_LOCKED,
		nomem = SQLITE_NOMEM,
		readonly = SQLITE_READONLY,
		interrupt = SQLITE_INTERRUPT,
		ioerr = SQLITE_IOERR,
		corrupt = SQLITE_CORRUPT,
		notfound = SQLITE_NOTFOUND,
		full = SQLITE_FULL,
		cantopen = SQLITE_CANTOPEN,
		protocol = SQLITE_PROTOCOL,
		empty = SQLITE_EMPTY,
		schema = SQLITE_SCHEMA,
		toobig = SQLITE_TOOBIG,
		constraint = SQLITE_CONSTRAINT,
		mismatch = SQLITE_MISMATCH,
		misuse = SQLITE_MISUSE,
		nolfs = SQLITE_NOLFS,
		auth = SQLITE_AUTH,
		format = SQLITE_FORMAT,
		range = SQLITE_RANGE,
		notadb = SQLITE_NOTADB,
		notice = SQLITE_NOTICE,
		warning = SQLITE_WARNING,
		row = SQLITE_ROW,
		done = SQLITE_DONE,
		// extended result codes
		#ifdef SQLITE_ERROR_MISSING_COLLSEQ
		error_missing_collseq = SQLITE_ERROR_MISSING_COLLSEQ,
		#endif
		#ifdef SQLITE_ERROR_RETRY
		error_retry = SQLITE_ERROR_RETRY,
		#endif
		ioerr_read = SQLITE_IOERR_READ,
		ioerr_short_read = SQLITE_IOERR_SHORT_READ,
		ioerr_write = SQLITE_IOERR_WRITE,
		ioerr_fsync = SQLITE_IOERR_FSYNC,
		ioerr_dir_fsync = SQLITE_IOERR_DIR_FSYNC,
		ioerr_truncate = SQLITE_IOERR_TRUNCATE,
		ioerr_fstat = SQLITE_IOERR_FSTAT,
		ioerr_unlock = SQLITE_IOERR_UNLOCK,
		ioerr_rdlock = SQLITE_IOERR_RDLOCK,
		ioerr_delete = SQLITE_IOERR_DELETE,
		ioerr_blocked = SQLITE_IOERR_BLOCKED,
		ioerr_nomem = SQLITE_IOERR_NOMEM,
		ioerr_access = SQLITE_IOERR_ACCESS,
		ioerr_checkreservedlock = SQLITE_IOERR_CHECKRESERVEDLOCK,
		ioerr_lock = SQLITE_IOERR_LOCK,
		ioerr_close = SQLITE_IOERR_CLOSE,
		ioerr_dir_close = SQLITE_IOERR_DIR_CLOSE,
		ioerr_shmopen = SQLITE_IOERR_SHMOPEN,
		ioerr_shmsize = SQLITE_IOERR_SHMSIZE,
		ioerr_shmlock = SQLITE_IOERR_SHMLOCK,
		ioerr_shmmap = SQLITE_IOERR_SHMMAP,
		ioerr_seek = SQLITE_IOERR_SEEK,
		ioerr_delete_noent = SQLITE_IOERR_DELETE_NOENT,
		ioerr_mmap = SQLITE_IOERR_MMAP,
		ioerr_gettemppath = SQLITE_IOERR_GETTEMPPATH,
		ioerr_convpath = SQLITE_IOERR_CONVPATH,
		ioerr_vnode = SQLITE_IOERR_VNODE,
		ioerr_auth = SQLITE_IOERR_AUTH,
		#ifdef SQLITE_IOERR_BEGIN_ATOMIC
		ioerr_begin_atomic = SQLITE_IOERR_BEGIN_ATOMIC,
		#endif
		#ifdef SQLITE_IOERR_COMMIT_ATOMIC
		ioerr_commit_atomic = SQLITE_IOERR_COMMIT_ATOMIC,
		#endif
		#ifdef SQLITE_IOERR_ROLLBACK_ATOMIC
		ioerr_rollback_atomic = SQLITE_IOERR_ROLLBACK_ATOMIC,
		#endif
		locked_sharedcache = SQLITE_LOCKED_SHAREDCACHE,
		busy_recovery = SQLITE_BUSY_RECOVERY,
		busy_snapshot = SQLITE_BUSY_SNAPSHOT,
		cantopen_notempdir = SQLITE_CANTOPEN_NOTEMPDIR,
		cantopen_isdir = SQLITE_CANTOPEN_ISDIR,
		cantopen_fullpath = SQLITE_CANTOPEN_FULLPATH,
		cantopen_convpath = SQLITE_CANTOPEN_CONVPATH,
		corrupt_vtab = SQLITE_CORRUPT_VTAB,
		readonly_recovery = SQLITE_READONLY_RECOVERY,
		readonly_cantlock = SQLITE_READONLY_CANTLOCK,
		readonly_rollback = SQLITE_READONLY_ROLLBACK,
		readonly_dbmoved = SQLITE_READONLY_DBMOVED,
		#ifdef SQLITE_READONLY_CANTINIT
		readonly_cantinit = SQLITE_READONLY_CANTINIT,
		#endif
		#ifdef SQLITE_READONLY_DIRECTORY
		readonly_directory = SQLITE_READONLY_DIRECTORY,
		#endif
		abort_rollback = SQLITE_ABORT_ROLLBACK,
		constraint_check = SQLITE_CONSTRAINT_CHECK,
		constraint_commithook = SQLITE_CONSTRAINT_COMMITHOOK,
		constraint_foreignkey = SQLITE_CONSTRAINT_FOREIGNKEY,
		constraint_function = SQLITE_CONSTRAINT_FUNCTION,
		constraint_notnull = SQLITE_CONSTRAINT_NOTNULL,
		constraint_primarykey = SQLITE_CONSTRAINT_PRIMARYKEY,
		constraint_trigger = SQLITE_CONSTRAINT_TRIGGER,
		constraint_unique = SQLITE_CONSTRAINT_UNIQUE,
		constraint_vtab = SQLITE_CONSTRAINT_VTAB,
		constraint_rowid = SQLITE_CONSTRAINT_ROWID,
		notice_recover_wal = SQLITE_NOTICE_RECOVER_WAL,
		notice_recover_rollback = SQLITE_NOTICE_RECOVER_ROLLBACK,
		warning_autoindex = SQLITE_WARNING_AUTOINDEX,
		auth_user = SQLITE_AUTH_USER,
		ok_load_permanently = SQLITE_OK_LOAD_PERMANENTLY,
	};

	class error_category: public std::error_category {
	public:
		const char* name() const noexcept override;
		std::string message(int ev) const noexcept override;
	};

	extern error_category sqlite_category;

	inline std::error_condition
	make_error_condition(errc e) noexcept {
		return std::error_condition(static_cast<int>(e), sqlite_category);
	}

	inline errc
	call(int ret) {
		if (ret != int(errc::ok)) { throw std::system_error(ret, sqlite_category); }
		return errc(ret);
	}

	inline void
	throw_error(errc err) {
		throw std::system_error(int(err), sqlite_category);
	}

}

namespace std { template<> struct is_error_condition_enum<sqlite::errc>: true_type {}; }


#endif // vim:filetype=cpp
