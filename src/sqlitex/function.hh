#ifndef SQLITEX_FUNCTION_HH
#define SQLITEX_FUNCTION_HH

#include <sqlitex/any.hh>
#include <sqlitex/context.hh>
#include <sqlitex/forward.hh>

namespace sqlite {

	class function {
	public:
		inline void func(context* ctx, int nargs, any_base* args) {}
		inline void step(context* ctx, int nargs, any_base* args) {}
		inline void end(context* ctx) {}
	};

}

#endif // vim:filetype=cpp
