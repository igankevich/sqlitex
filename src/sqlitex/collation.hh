#ifndef SQLITEX_COLLATION_HH
#define SQLITEX_COLLATION_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	template <encoding enc>
	class collation_base {

	public:
		static constexpr const ::sqlite::encoding encoding() { return enc; }
		using string = typename encoding_traits<enc>::string;

		inline int
		operator()(const string& lhs, const string& rhs) {
			return lhs.compare(rhs);
		}

	};

}

#endif // vim:filetype=cpp
