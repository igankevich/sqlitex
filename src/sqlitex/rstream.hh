#ifndef SQLITE_RSTREAM_HH
#define SQLITE_RSTREAM_HH

#include <cstdint>
#include <iosfwd>
#include <string>

#include <unistdx/it/basic_istream_iterator>

#include <sqlitex/call.hh>
#include <sqlitex/rstream_base.hh>
#include <sqlitex/types.hh>

namespace sqlite {

	typedef ::sqlite3_stmt statement_type;

	class cstream;

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
		statement_type* _stmt = nullptr;

	public:

		rstream() = default;

		inline explicit
		rstream(statement_type* stmt):
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
		open(statement_type* stmt) {
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

		inline void
		step() {
			sqlite_errc ret = sqlite_errc(::sqlite3_step(this->_stmt));
			if (ret == sqlite_errc::row) {
				return;
			}
			if (ret == sqlite_errc::done) {
				this->setstate(eofbit);
			} else {
				this->setstate(failbit);
				throw_error(ret);
			}
		}

		inline void
		reset() {
			call(::sqlite3_reset(this->_stmt));
		}

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

		template <class Ch, class Tr, class Alloc>
		inline void
		bind(int index, const std::basic_string<Ch,Tr,Alloc>& value) {
			call(::sqlite3_bind_text(
				this->_stmt,
				index,
				value.data(),
				value.size(),
				SQLITE_TRANSIENT
			));
		}

		inline void
		bind(int index, const blob& value) {
			call(::sqlite3_bind_blob(
				this->_stmt,
				index,
				value.ptr(),
				value.size(),
				SQLITE_TRANSIENT
			));
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

		inline statement_type*
		statement() noexcept {
			return this->_stmt;
		}

		inline const statement_type*
		statement() const noexcept {
			return this->_stmt;
		}

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
		int _col = 0;
		int _ncols = 0;

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
			this->_col = 0;
			this->_ncols = ::sqlite3_data_count(this->_rstr.statement());
		}

		inline void
		reset() {
			this->_col = 0;
			this->_ncols = 0;
		}

		inline bool
		eof() const noexcept {
			return this->_col == this->_ncols;
		}

		inline bool
		good() const noexcept {
			return !this->eof();
		}

		inline cstream&
		operator>>(float& rhs) {
			if (this->good()) {
				auto d = ::sqlite3_column_double(this->_rstr.statement(), this->_col);
				rhs = static_cast<float>(d);
				++this->_col;
			}
			return *this;
		}

		inline cstream&
		operator>>(double& rhs) {
			if (this->good()) {
				rhs = ::sqlite3_column_double(this->_rstr.statement(), this->_col);
				++this->_col;
			}
			return *this;
		}

		inline cstream&
		operator>>(int& rhs) {
			if (this->good()) {
				rhs = ::sqlite3_column_int(this->_rstr.statement(), this->_col);
				++this->_col;
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
				rhs = ::sqlite3_column_int64(this->_rstr.statement(), this->_col);
				++this->_col;
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

		template <class Ch, class Tr, class Alloc>
		inline cstream&
		operator>>(std::basic_string<Ch,Tr,Alloc>& rhs) {
			if (this->good()) {
				const unsigned char* result =
					::sqlite3_column_text(this->_rstr.statement(), this->_col);
				if (!result) {
					rhs.clear();
				} else {
					rhs = reinterpret_cast<const char*>(result);
				}
				++this->_col;
			}
			return *this;
		}

		inline cstream&
		operator>>(blob& rhs) {
			if (this->good()) {
				const void* result =
					::sqlite3_column_blob(this->_rstr.statement(), this->_col);
				const int n =
					::sqlite3_column_bytes(this->_rstr.statement(), this->_col);
				if (!result) {
					rhs.clear();
				} else {
					rhs.assign(reinterpret_cast<const char*>(result), n);
				}
				++this->_col;
			}
			return *this;
		}

		template <class Clock, class Duration>
		inline cstream&
		operator>>(std::chrono::time_point<Clock,Duration>& rhs) {
			if (this->good()) {
				int64_t value =
					::sqlite3_column_int64(
						this->_rstr.statement(),
						this->_col
					);
				rhs = Clock::from_time_t(static_cast<std::time_t>(value));
				++this->_col;
			}
			return *this;
		}

		inline int
		num_columns() const noexcept {
			return this->_ncols;
		}

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

}

#endif // vim:filetype=cpp
