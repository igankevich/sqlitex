#ifndef SQLITEX_BLOB_HH
#define SQLITEX_BLOB_HH

#include <string>

#include <sqlitex/forward.hh>

namespace sqlite {

	class blob: public std::string {

	public:

		using std::string::string;

		blob() = default;
		~blob() = default;
		blob(blob&&) = default;
		blob(const blob&) = default;
		blob& operator=(blob&&) = default;
		blob& operator=(const blob&) = default;

		inline
		blob(std::string&& rhs):
		std::string(std::forward<std::string>(rhs)) {}

		inline void* get() { return &this->std::string::operator[](0); }
		inline const void* get() const { return this->std::string::data(); }

	};

	class zeroes {

	private:
		uint64 _size = 0;

	public:
		inline explicit zeroes(uint64 size): _size(size) {}
		inline uint64 size() const { return this->_size; }

	};

}

#endif // vim:filetype=cpp
