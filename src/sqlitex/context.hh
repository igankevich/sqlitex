#ifndef SQLITEX_CONTEXT_HH
#define SQLITEX_CONTEXT_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class context {

	private:
		types::context* _ptr = nullptr;

	public:
		inline context(types::context* ptr): _ptr(ptr) {}
		inline types::context* get() { return this->_ptr; }
		inline connection_base connection();
		inline void* data() { return ::sqlite3_user_data(this->_ptr); }

		template <class T> inline T*
		data() { return static_cast<T*>(::sqlite3_user_data(this->_ptr)); }

		template <class T>
		inline T*
		metadata(int argument) {
			return static_cast<T*>(::sqlite3_get_auxdata(this->_ptr, argument));
		}

		inline void
		metadata(int argument, void* data, types::destructor destructor=nullptr) {
			return ::sqlite3_set_auxdata(this->_ptr, argument, data, destructor);
		}

		inline void*
		temporary_buffer(int size) {
			return ::sqlite3_aggregate_context(this->_ptr, size);
		}

		inline void result(float rhs) { ::sqlite3_result_double(this->_ptr, rhs); }
		inline void result(double rhs) { ::sqlite3_result_double(this->_ptr, rhs); }
		inline void result(int rhs) { ::sqlite3_result_int(this->_ptr, rhs); }
		inline void result(int64 rhs) { ::sqlite3_result_int64(this->_ptr, rhs); }
		inline void result(std::nullptr_t) { ::sqlite3_result_null(this->_ptr); }
		inline void result(any_base rhs) { ::sqlite3_result_value(this->_ptr, rhs.get()); }

		template <class Clock, class Duration>
		inline void
		result(const std::chrono::time_point<Clock,Duration>& rhs) {
			this->result(static_cast<int64>(Clock::to_time_t(rhs)));
		}

		inline void
		result(const named_ptr& rhs) {
			::sqlite3_result_pointer(this->_ptr, rhs.get(), rhs.name(), nullptr);
		}

		inline void
		result(const zeroes& rhs) {
			::sqlite3_result_zeroblob64(this->_ptr, rhs.size());
		}

		inline void
		result(const char* rhs, destructor destr=pass_by_copy) {
			::sqlite3_result_text(this->_ptr, rhs, -1, destr);
		}

		template <class Alloc>
		inline void
		result(const basic_u8string<Alloc>& value, destructor destr=pass_by_copy) {
			::sqlite3_result_text64(
				this->_ptr,
				value.data(),
				value.size(),
				destr,
				downcast(encoding::utf8)
			);
		}

		template <class Alloc>
		inline void
		result(const basic_u16string<Alloc>& value,
               encoding enc=encoding::utf16, destructor destr=pass_by_copy) {
			::sqlite3_result_text64(
				this->_ptr,
				reinterpret_cast<const char*>(value.data()),
				value.size()*sizeof(char16_t),
				destr,
				downcast(enc)
			);
		}

		template <class Alloc>
		inline void
		result(const basic_u16string<Alloc>& value, destructor destr=pass_by_copy,
               encoding enc=encoding::utf16) {
			::sqlite3_result_text64(
				this->_ptr,
				reinterpret_cast<const char*>(value.data()),
				value.size()*sizeof(char16_t),
				destr,
				downcast(enc)
			);
		}

		inline void
		result(const blob& value, destructor destr=pass_by_copy) {
			::sqlite3_result_blob64(this->_ptr, value.get(), value.size(), destr);
		}

		inline void
		result_subtype(unsigned int tp) {
			::sqlite3_result_subtype(this->_ptr, tp);
		}

		inline void error(const char* msg) { ::sqlite3_result_error(this->_ptr, msg, -1); }

		inline void
		error(const u8string& msg) {
			::sqlite3_result_error(this->_ptr, msg.data(), msg.size());
		}

		inline void
		error(const u16string& msg) {
			::sqlite3_result_error16(
				this->_ptr,
				msg.data(),
				msg.size()*sizeof(u16string::value_type)
			);
		}

		inline void error_length() { ::sqlite3_result_error_toobig(this->_ptr); }
		inline void error_bad_alloc() { ::sqlite3_result_error_nomem(this->_ptr); }
		inline void error(errc code) { ::sqlite3_result_error_code(this->_ptr, int(code)); }

	};

}

#endif // vim:filetype=cpp
