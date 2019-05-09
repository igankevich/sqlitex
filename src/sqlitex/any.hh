#ifndef SQLITEX_ANY_HH
#define SQLITEX_ANY_HH

#include <string>

#include <sqlitex/allocator.hh>
#include <sqlitex/blob.hh>
#include <sqlitex/forward.hh>
#include <sqlitex/named_ptr.hh>

namespace sqlite {

	template <class T, encoding enc=encoding{}> T any_cast(const any& value);

	class any_base {

	public:
		using value_type = types::value;

	protected:
		value_type* _ptr = nullptr;

	public:

		inline explicit any_base(value_type* ptr): _ptr(ptr) {}
		any_base() = default;
		~any_base() = default;
		any_base(any_base&&) = default;
		any_base& operator=(any_base&&) = default;
		any_base(const any_base&) = default;
		any_base& operator=(const any_base&) = default;

		inline value_type* get() { return this->_ptr; }
		inline const value_type* get() const { return this->_ptr; }
		inline void clear(value_type* ptr=nullptr) { this->_ptr = ptr; }
		inline data_type type() const { return data_type(::sqlite3_value_type(this->_ptr)); }
		inline bool no_change() const { return ::sqlite3_value_nochange(this->_ptr) != 0; }
		unsigned int subtype() const { return ::sqlite3_value_subtype(this->_ptr); }
		inline int size() const { return ::sqlite3_value_bytes(this->_ptr); }
		inline int size16() const { return ::sqlite3_value_bytes16(this->_ptr); }

		inline data_type
		numeric_type() const {
			return data_type(::sqlite3_value_numeric_type(this->_ptr));
		}

		template <class T, encoding> friend T any_cast(const any& value);
		friend void* any_cast(const any& value, const char* type);

	};

	class any: public any_base {

	public:

		any() = default;
		inline explicit any(value_type* ptr): any_base(::sqlite3_value_dup(ptr)) {}
		inline ~any() { ::sqlite3_value_free(this->_ptr); }

		inline any&
		operator=(const any& rhs) {
			this->_ptr = ::sqlite3_value_dup(rhs._ptr);
			return *this;
		}

	};

	template <> inline int
	any_cast<int>(const any& value) { return ::sqlite3_value_int(value._ptr); }

	template <> inline types::int64
	any_cast<types::int64>(const any& value) { return ::sqlite3_value_int64(value._ptr); }

	template <> inline float
	any_cast<float>(const any& value) { return ::sqlite3_value_double(value._ptr); }

	template <> inline double
	any_cast<double>(const any& value) { return ::sqlite3_value_double(value._ptr); }

	template <> inline string
	any_cast<string>(const any& value) {
		auto ptr = ::sqlite3_value_text(value._ptr);
		string result;
		result = reinterpret_cast<const string::value_type*>(ptr);
		return result;
	}

	template <> inline u8string
	any_cast<u8string,encoding::utf8>(const any& value) {
		auto ptr = ::sqlite3_value_text(value._ptr);
		u8string result;
		result = reinterpret_cast<const u8string::value_type*>(ptr);
		return result;
	}

	template <> inline u8string
	any_cast<u8string>(const any& value) {
		return any_cast<u8string,encoding::utf8>(value);
	}

	template <> inline u16string
	any_cast<u16string,encoding::utf16>(const any& value) {
		auto ptr = ::sqlite3_value_text16(value._ptr);
		u16string result;
		result = reinterpret_cast<const u16string::value_type*>(ptr);
		return result;
	}

	template <> inline u16string
	any_cast<u16string,encoding::utf16le>(const any& value) {
		auto ptr = ::sqlite3_value_text16le(value._ptr);
		u16string result;
		result = reinterpret_cast<const u16string::value_type*>(ptr);
		return result;
	}

	template <> inline u16string
	any_cast<u16string,encoding::utf16be>(const any& value) {
		auto ptr = ::sqlite3_value_text16be(value._ptr);
		u16string result;
		result = reinterpret_cast<const u16string::value_type*>(ptr);
		return result;
	}

	template <> inline u16string
	any_cast<u16string>(const any& value) {
		return any_cast<u16string,encoding::utf16>(value);
	}

	template <> inline blob
	any_cast<blob>(const any& value) {
		auto ptr = ::sqlite3_value_blob(value._ptr);
		blob result;
		result.assign(reinterpret_cast<const blob::value_type*>(ptr), value.size());
		return result;
	}

	inline void*
	any_cast(const any& value, const char* type) {
		return ::sqlite3_value_pointer(value._ptr, type);
	}

}

#endif // vim:filetype=cpp
