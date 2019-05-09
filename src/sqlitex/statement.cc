#include <ostream>

#include <sqlitex/database.hh>
#include <sqlitex/statement.hh>

void
sqlite::statement::dump(std::ostream& out) {
	out << this->sql() << std::endl;
	const int n = this->num_parameters();
	for (int i=1; i<=n; ++i) {
		const char* name = this->parameter_name(i);
		if (name) {
			out << "param[" << i << "]=" << name << std::endl;
		}
	}
	if (n > 0) {
		out << this->expanded_sql().get() << std::endl;
	}
}

sqlite::static_database sqlite::statement::database() {
	return static_database(::sqlite3_db_handle(this->_ptr));
}
