#ifndef SQLITEX_RANDOM_DEVICE_HH
#define SQLITEX_RANDOM_DEVICE_HH

#include <sqlitex/forward.hh>

namespace sqlitex {

	class random_device {

	public:
		using result_type = int;

		inline result_type operator()() const {
			union { result_type n; char bytes[sizeof(result_type)] } u;
			::sqlite3_randomness(sizeof(u), u.bytes);
			return u.n;
		}

		inline void operator()(void* buffer, int n) const {
			::sqlite3_randomness(n, buffer);
		}

	};

}

#endif // vim:filetype=cpp
