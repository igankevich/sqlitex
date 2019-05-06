#ifndef SQLITE_RSTREAM_HH
#define SQLITE_RSTREAM_HH

#include <cstdint>
#include <iosfwd>
#include <string>

#include <unistdx/it/basic_istream_iterator>

#include <sqlitex/allocator.hh>
#include <sqlitex/any.hh>
#include <sqlitex/call.hh>
#include <sqlitex/forward.hh>
#include <sqlitex/named_ptr.hh>
#include <sqlitex/rstream_base.hh>
#include <sqlitex/types.hh>

namespace sqlite {

	/**
	\brief Stream of rows.
	\date 2018-10-08
	\author Ivan Gankevich
	\details
	Example usage:
	\code{.cpp}
	Publication pub;
	rstream rstr = db.prepare("SELECT * FROM publications");
	cstream cstr(rstr);
	rstr >> cstr;
	cstr >> pub._author >> pub._title >> pub._year;
	\endcode
	*/
	class rstream: public rstream_base {

	private:
		types::statement* _stmt = nullptr;

	public:

		rstream() = default;

		inline explicit
		rstream(types::statement* stmt):
		_stmt(stmt)
		{}

		inline
		rstream(rstream&& rhs):
		_stmt(rhs._stmt) {
			rhs._stmt = nullptr;
		}

		rstream(const rstream&) = delete;
		rstream& operator=(const rstream&) = delete;

		inline rstream&
		operator=(rstream&& rhs) {
			this->rstream_base::clear(goodbit);
			this->swap(rhs);
			return *this;
		}

		inline void
		swap(rstream& rhs) {
			std::swap(this->_stmt, rhs._stmt);
		}

		inline
		~rstream() {
			::sqlite3_finalize(this->_stmt);
			this->_stmt = nullptr;
		}

		inline void
		open(types::statement* stmt) {
			this->close();
			this->_stmt = stmt;
		}

		inline bool
		is_open() const noexcept {
			return this->_stmt != nullptr;
		}

		inline void
		close() {
			if (this->_stmt) {
				call(::sqlite3_finalize(this->_stmt));
				this->_stmt = nullptr;
			}
		}

		inline bool read_only() const { return ::sqlite3_stmt_readonly(this->_stmt); }
		inline bool busy() const { return ::sqlite3_stmt_busy(this->_stmt); }

		inline void
		step() {
			errc ret = errc(::sqlite3_step(this->_stmt));
			if (ret == errc::row) {
				return;
			}
			if (ret == errc::done) {
				this->setstate(eofbit);
			} else {
				this->setstate(failbit);
				throw_error(ret);
			}
		}

		inline int num_columns() const { return ::sqlite3_column_count(this->_stmt); }
		inline void reset() { call(::sqlite3_reset(this->_stmt)); }

		inline int
		num_parameters() const noexcept {
			return ::sqlite3_bind_parameter_count(this->_stmt);
		}

		inline const char*
		parameter_name(int index) {
			return ::sqlite3_bind_parameter_name(this->_stmt, index);
		}

		inline int
		parameter_index(const char* name) {
			return ::sqlite3_bind_parameter_index(this->_stmt, name);
		}

		inline void
		bind(int index, double value) {
			call(::sqlite3_bind_double(this->_stmt, index, value));
		}

		inline void
		bind(int index, int value) {
			call(::sqlite3_bind_int(this->_stmt, index, value));
		}

		inline void
		bind(int index, unsigned int value) {
			this->bind(index, static_cast<int>(value));
		}

		inline void
		bind(int index, int64_t value) {
			call(::sqlite3_bind_int64(this->_stmt, index, value));
		}

		inline void
		bind(int index, uint64_t value) {
			this->bind(index, static_cast<int64_t>(value));
		}

		inline void
		bind(int index, const char* value) {
			call(::sqlite3_bind_text(this->_stmt, index, value, -1, SQLITE_STATIC));
		}

		template <class Alloc>
		inline void
		bind(int index, const basic_u8string<Alloc>& value) {
			call(::sqlite3_bind_text64(
				this->_stmt,
				index,
				value.data(),
				value.size(),
				SQLITE_TRANSIENT,
				downcast(encoding::utf8)
			));
		}

		template <class Alloc>
		inline void
		bind(int index, const basic_u16string<Alloc>& value, encoding enc=encoding::utf16) {
			call(::sqlite3_bind_text64(
				this->_stmt,
				index,
				reinterpret_cast<const char*>(value.data()),
				value.size()*sizeof(char16_t),
				SQLITE_TRANSIENT,
				downcast(enc)
			));
		}

		inline void
		bind(int index, any_base value) {
			call(::sqlite3_bind_value(this->_stmt, index, value.get()));
		}

		inline void
		bind(int index, const named_ptr& value) {
			call(::sqlite3_bind_pointer(
				this->_stmt,
				index,
				value.get(),
				value.name(),
				nullptr
			));
		}

		inline void
		bind(int index, const blob& value) {
			call(::sqlite3_bind_blob64(
				this->_stmt,
				index,
				value.ptr(),
				value.size(),
				SQLITE_TRANSIENT
			));
		}

		inline void
		bind(int index, const zeroes& value) {
			call(::sqlite3_bind_zeroblob64(this->_stmt, index, value.size()));
		}

		template <class Clock, class Duration>
		inline void
		bind(int index, const std::chrono::time_point<Clock,Duration>& value) {
			call(::sqlite3_bind_int64(
				this->_stmt,
				index,
				static_cast<int64_t>(Clock::to_time_t(value))
			));
		}

		inline void
		bind(int index, std::nullptr_t) {
			call(::sqlite3_bind_null(this->_stmt, index));
		}

		inline void
		clear() {
			call(::sqlite3_clear_bindings(this->_stmt));
		}

		void
		dump(std::ostream& out);

		inline const char*
		sql() const noexcept {
			return ::sqlite3_sql(this->_stmt);
		}

		inline unique_ptr<char>
		expanded_sql() const noexcept {
			return unique_ptr<char>(::sqlite3_expanded_sql(this->_stmt));
		}

		inline rstream&
		operator>>(cstream& rhs);

		inline types::statement*
		statement() noexcept {
			return this->_stmt;
		}

		inline const types::statement*
		statement() const noexcept {
			return this->_stmt;
		}

		static_database database();

		inline const char*
		column_name(int i) const {
			return ::sqlite3_column_name(this->_stmt, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		column_name(int i) const;

		inline data_type
		column_type(int i) const {
			return static_cast<data_type>(
				::sqlite3_column_type(this->_stmt, i)
			);
		}

		inline const char*
		database_name(int i) const {
			return ::sqlite3_column_database_name(this->_stmt, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		database_name(int i) const;

		inline const char*
		table_name(int i) const {
			return ::sqlite3_column_table_name(this->_stmt, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		table_name(int i) const;

		inline const char*
		origin_name(int i) const {
			return ::sqlite3_column_origin_name(this->_stmt, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		origin_name(int i) const;

		inline const char*
		column_type_name(int i) const {
			return ::sqlite3_column_decltype(this->_stmt, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		column_type_name(int i) const;

		inline int
		column_size(int i) const {
			return ::sqlite3_column_bytes(this->_stmt, i);
		}

		template <encoding enc>
		inline int
		column_size(int i) const;

	};

	inline void
	swap(rstream& lhs, rstream& rhs) {
		lhs.swap(rhs);
	}

	template<class T>
	using rstream_iterator = sys::basic_istream_iterator<rstream, T>;

	/**
	\brief Stream of columns.
	\date 2018-10-08
	\author Ivan Gankevich
	*/
	class cstream {

	private:
		rstream& _rstr;
		int _column = 0;
		int _ncolumns = 0;

	public:

		inline explicit
		cstream(rstream& rhs):
		_rstr(rhs)
		{}

		cstream() = delete;
		cstream(const cstream&) = delete;
		cstream& operator=(const cstream&) = delete;
		cstream(cstream&&) = delete;
		cstream& operator=(cstream&&) = delete;

		inline void
		init(rstream& rhs) {
			this->_column = 0;
			this->_ncolumns = ::sqlite3_data_count(this->_rstr.statement());
		}

		inline void
		reset() {
			this->_column = 0;
			this->_ncolumns = 0;
		}

		inline bool eof() const { return this->_column == this->_ncolumns; }
		inline bool good() const { return !this->eof(); }
		inline const rstream& parent() const { return this->_rstr; }
		inline data_type column_type() const { return parent().column_type(this->_column); }

		inline cstream&
		operator>>(float& rhs) {
			if (this->good()) {
				auto d = ::sqlite3_column_double(this->_rstr.statement(), this->_column);
				rhs = static_cast<float>(d);
				++this->_column;
			}
			return *this;
		}

		inline cstream&
		operator>>(double& rhs) {
			if (this->good()) {
				rhs = ::sqlite3_column_double(this->_rstr.statement(), this->_column);
				++this->_column;
			}
			return *this;
		}

		inline cstream&
		operator>>(int& rhs) {
			if (this->good()) {
				rhs = ::sqlite3_column_int(this->_rstr.statement(), this->_column);
				++this->_column;
			}
			return *this;
		}

		inline cstream&
		operator>>(unsigned int& rhs) {
			int tmp;
			this->operator>>(tmp);
			rhs = tmp;
			return *this;
		}

		inline cstream&
		operator>>(int64_t& rhs) {
			if (this->good()) {
				rhs = ::sqlite3_column_int64(this->_rstr.statement(), this->_column);
				++this->_column;
			}
			return *this;
		}

		inline cstream&
		operator>>(uint64_t& rhs) {
			int64_t tmp;
			this->operator>>(tmp);
			rhs = tmp;
			return *this;
		}

		template <class Alloc>
		inline cstream&
		operator>>(basic_u8string<Alloc>& rhs) {
			using char_type = typename basic_u8string<Alloc>::value_type;
			if (this->good()) {
				auto result = ::sqlite3_column_text(this->_rstr.statement(), this->_column);
				if (!result) { rhs.clear(); }
				else { rhs = reinterpret_cast<const char_type*>(result); }
				++this->_column;
			}
			return *this;
		}

		template <class Alloc>
		inline cstream&
		operator>>(basic_u16string<Alloc>& rhs) {
			using char_type = typename basic_u16string<Alloc>::value_type;
			if (this->good()) {
				auto result = ::sqlite3_column_text16(this->_rstr.statement(), this->_column);
				if (!result) { rhs.clear(); }
				else { rhs = static_cast<const char_type*>(result); }
				++this->_column;
			}
			return *this;
		}

		inline cstream&
		operator>>(blob& rhs) {
			if (this->good()) {
				const void* result =
					::sqlite3_column_blob(this->_rstr.statement(), this->_column);
				const int n =
					::sqlite3_column_bytes(this->_rstr.statement(), this->_column);
				if (!result) {
					rhs.clear();
				} else {
					rhs.assign(reinterpret_cast<const char*>(result), n);
				}
				++this->_column;
			}
			return *this;
		}

		inline cstream&
		operator>>(any_base& rhs) {
			if (this->good()) {
				rhs.clear(::sqlite3_column_value(this->_rstr.statement(), this->_column));
				++this->_column;
			}
			return *this;
		}

		template <class Clock, class Duration>
		inline cstream&
		operator>>(std::chrono::time_point<Clock,Duration>& rhs) {
			if (this->good()) {
				auto value = ::sqlite3_column_int64(this->_rstr.statement(), this->_column);
				rhs = Clock::from_time_t(static_cast<std::time_t>(value));
				++this->_column;
			}
			return *this;
		}

		inline int num_columns() const { return this->_ncolumns; }

	};

	rstream&
	rstream::operator>>(cstream& rhs) {
		this->step();
		if (this->good()) {
			rhs.init(*this);
		} else {
			rhs.reset();
		}
		return *this;
	}

	inline void
	bind(rstream&,int) {}

	template <class Head, class ... Tail>
	inline void
	bind(rstream& rstr, int index, const Head& head, const Tail& ... tail) {
		rstr.bind(index, head);
		bind(rstr, index+1, tail...);
	}

	inline void
	bind(rstream&) {}

	template <class Head, class ... Tail>
	inline void
	bind(rstream& rstr, const Head& head, const Tail& ... tail) {
		bind(rstr, 1, tail...);
	}

	template <>
	inline u8string
	rstream::column_name<encoding::utf8>(int i) const {
		return ::sqlite3_column_name(this->_stmt, i);
	}

	template <>
	inline u16string
	rstream::column_name<encoding::utf16>(int i) const {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_name16(this->_stmt, i)
		);
	}

	template <>
	inline u8string
	rstream::database_name<encoding::utf8>(int i) const {
		return ::sqlite3_column_database_name(this->_stmt, i);
	}

	template <>
	inline u16string
	rstream::database_name<encoding::utf16>(int i) const {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_database_name16(this->_stmt, i)
		);
	}

	template <>
	inline u8string
	rstream::table_name<encoding::utf8>(int i) const {
		return ::sqlite3_column_table_name(this->_stmt, i);
	}

	template <>
	inline u16string
	rstream::table_name<encoding::utf16>(int i) const {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_table_name16(this->_stmt, i)
		);
	}

	template <>
	inline u8string
	rstream::origin_name<encoding::utf8>(int i) const {
		return ::sqlite3_column_origin_name(this->_stmt, i);
	}

	template <>
	inline u16string
	rstream::origin_name<encoding::utf16>(int i) const {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_origin_name16(this->_stmt, i)
		);
	}

	template <>
	inline u8string
	rstream::column_type_name<encoding::utf8>(int i) const {
		return ::sqlite3_column_decltype(this->_stmt, i);
	}

	template <>
	inline u16string
	rstream::column_type_name<encoding::utf16>(int i) const {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_decltype16(this->_stmt, i)
		);
	}

	template <>
	inline int
	rstream::column_size<encoding::utf8>(int i) const {
		return ::sqlite3_column_bytes(this->_stmt, i);
	}

	template <>
	inline int
	rstream::column_size<encoding::utf16>(int i) const {
		return ::sqlite3_column_bytes16(this->_stmt, i);
	}

}

#endif // vim:filetype=cpp
