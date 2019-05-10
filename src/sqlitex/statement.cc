#include <ostream>

#include <sqlitex/connection.hh>
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

sqlite::connection_base sqlite::statement::connection() {
	return connection_base(::sqlite3_db_handle(this->_ptr));
}
