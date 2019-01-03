#ifndef VTESTBED_SQLITE_RSTREAM_CC
#define VTESTBED_SQLITE_RSTREAM_CC

#include "rstream.hh"

#include <ostream>

void
sqlite::rstream::dump(std::ostream& out) {
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


#endif // vim:filetype=cpp
