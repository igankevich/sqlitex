#ifndef SQLITEX_VIRTUAL_TABLE_HH
#define SQLITEX_VIRTUAL_TABLE_HH

#include <sqlitex/any.hh>
#include <sqlitex/context.hh>
#include <sqlitex/forward.hh>

namespace sqlite {

	class virtual_table_index: public types::index_info {
	public:
		inline const char* collation(int n) { return ::sqlite3_vtab_collation(this, n); }
	};

	class virtual_table_context: public context {

	public:
		using context::context;

		inline bool no_change() { return ::sqlite3_vtab_nochange(get()); }

	};

	class virtual_table_cursor {

	public:

		inline int close() { return 0; }
		inline int filter(int idxNum, const char* idxStr, int argc, any_base* argv) { return 0; }
		inline int next() { return 0; }
		inline bool eof() { return false; }
		inline int column(virtual_table_context* c, int n) { return 0; }
		inline int rowid(int64& out);

	};

	class virtual_table: public types::virtual_table {

	public:
		enum class key {
			constraint_support=SQLITE_VTAB_CONSTRAINT_SUPPORT,
		};

	public:

		inline int best_index(virtual_table_index* ptr) { return 0; }
		inline int disconnect() { return 0; }
		inline int destroy() { return 0; }
		inline int begin() { return 0; }
		inline int sync() { return 0; }
		inline int commit() { return 0; }
		inline int rollback() { return 0; }
		inline int name(const char*) { return 0; }
		inline int savepoint(int n) { return 0; }
		inline int release(int n) { return 0; }
		inline int rollback(int n) { return 0; }
		inline int update(int argc, any_base* argv, int64& rowid);
		inline int find_function(
			int nargs,
			const char* name,
			types::virtual_table_function* func,
			void** user_data
		) { return 0; }

	};

}

#endif // vim:filetype=cpp
