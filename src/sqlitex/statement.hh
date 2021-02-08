#ifndef SQLITEX_STATEMENT_HH
#define SQLITEX_STATEMENT_HH

#include <cstdint>
#include <iosfwd>
#include <iterator>
#include <string>
#include <type_traits>

#include <sqlitex/allocator.hh>
#include <sqlitex/any.hh>
#include <sqlitex/blob.hh>
#include <sqlitex/errc.hh>
#include <sqlitex/forward.hh>
#include <sqlitex/named_ptr.hh>

namespace sqlite {

	template <class Iterator>
	class iterator_view {

	private:
		Iterator _first, _last;

	public:
		inline
		iterator_view(Iterator first, Iterator last):
		_first(first), _last(last) {}

		inline Iterator begin() { return this->_first; }
		inline Iterator end() { return this->_last; }

	};

	class statement_counters {

	private:
		types::statement* _ptr = nullptr;

	public:

		inline explicit statement_counters(types::statement* ptr): _ptr(ptr) {}
		inline void reset() { ::sqlite3_stmt_scanstatus_reset(this->_ptr); }

		inline void
		get(int i, scan_status s, void* out) const {
			call(::sqlite3_stmt_scanstatus(this->_ptr, i, downcast(s), out));
		}

		inline int
		nloop(int i) const {
			int64 result = 0;
			this->get(i, scan_status::nloop, &result);
			return result;
		}

		inline int
		nvisit(int i) const {
			int64 result = 0;
			this->get(i, scan_status::nvisit, &result);
			return result;
		}

		inline double
		estimate(int i) const {
			double result = 0;
			this->get(i, scan_status::estimate, &result);
			return result;
		}

		inline const char*
		name(int i) const {
			const char* result = 0;
			this->get(i, scan_status::name, &result);
			return result;
		}

		inline const char*
		explain(int i) const {
			const char* result = 0;
			this->get(i, scan_status::explain, &result);
			return result;
		}

		inline int
		select_id(int i) const {
			int result = 0;
			this->get(i, scan_status::select_id, &result);
			return result;
		}

	};

	/**
	\brief SQL statement.
	\date 2018-10-08
	\author Ivan Gankevich
	\details
	Example usage:
	\code{.cpp}
	Publication pub;
	statement s = db.prepare("SELECT * FROM publications");
	s.step();
	cstream cstr(s);
	cstr >> pub._author >> pub._title >> pub._year;
	\endcode
	*/
	class statement {

	public:
		enum class status: int {
			fullscan_step=SQLITE_STMTSTATUS_FULLSCAN_STEP,
			sort=SQLITE_STMTSTATUS_SORT,
			autoindex=SQLITE_STMTSTATUS_AUTOINDEX,
			vm_step=SQLITE_STMTSTATUS_VM_STEP,
			reprepare=SQLITE_STMTSTATUS_REPREPARE,
			run=SQLITE_STMTSTATUS_RUN,
			memory_used=SQLITE_STMTSTATUS_MEMUSED,
		};

	private:
		types::statement* _ptr = nullptr;

	public:

		inline explicit
		statement(types::statement* stmt) noexcept:
		_ptr(stmt)
		{}

		statement() = default;
		statement(const statement&) = delete;
		statement& operator=(const statement&) = delete;

		inline statement(statement&& rhs) noexcept: _ptr(rhs._ptr) { rhs._ptr = nullptr; }

		inline statement&
		operator=(statement&& rhs) noexcept {
			this->swap(rhs);
			return *this;
		}

		inline void
		swap(statement& rhs) noexcept {
			std::swap(this->_ptr, rhs._ptr);
		}

		inline
		~statement() noexcept {
			::sqlite3_finalize(this->_ptr);
			this->_ptr = nullptr;
		}

		inline void
		close() {
			call(::sqlite3_finalize(this->_ptr));
			this->_ptr = nullptr;
		}

		inline types::statement* get() noexcept { return this->_ptr; }
		inline const types::statement* get() const noexcept { return this->_ptr; }
		inline bool read_only() const noexcept { return ::sqlite3_stmt_readonly(this->_ptr); }
		inline bool busy() const noexcept { return ::sqlite3_stmt_busy(this->_ptr); }
		inline bool is_open() const noexcept { return this->_ptr != nullptr; }

		inline errc
		step() {
			errc ret = errc(::sqlite3_step(this->_ptr));
			if (ret != errc::row && ret != errc::done) { throw_error(ret); }
			return ret;
		}

		inline int num_columns() const noexcept { return ::sqlite3_column_count(this->_ptr); }
		inline void reset() { call(::sqlite3_reset(this->_ptr)); }

		inline statement_counters counters() { return statement_counters(this->_ptr); }

		inline int
		get(status key, bool reset=false) noexcept {
			return ::sqlite3_stmt_status(this->_ptr, int(key), reset);
		}

		inline int
		num_parameters() const noexcept {
			return ::sqlite3_bind_parameter_count(this->_ptr);
		}

		inline const char*
		parameter_name(int i) noexcept {
			return ::sqlite3_bind_parameter_name(this->_ptr, i);
		}

		inline int
		parameter_index(const char* name) noexcept {
			return ::sqlite3_bind_parameter_index(this->_ptr, name);
		}

		template <class Float>
		inline auto
		column(int i, Float& value) const noexcept ->
		typename std::enable_if<std::is_floating_point<Float>::value>::type {
			value = static_cast<Float>(::sqlite3_column_double(this->_ptr, i));
		}

		template <class Integer>
		inline auto
		column(int i, Integer& value) const noexcept ->
		typename std::enable_if<std::is_integral<Integer>::value &&
		(sizeof(Integer) <= sizeof(int))>::type {
			value = static_cast<Integer>(::sqlite3_column_int(this->_ptr, i));
		}

		template <class Integer>
		inline auto
		column(int i, Integer& value) const noexcept ->
		typename std::enable_if<std::is_integral<Integer>::value &&
		(sizeof(Integer) > sizeof(int))>::type {
			value = static_cast<Integer>(::sqlite3_column_int64(this->_ptr, i));
		}

		template <class Clock, class Duration>
		inline void
		column(int i, std::chrono::time_point<Clock,Duration>& rhs) const noexcept {
			std::time_t value{};
			this->column(i, value);
			rhs = Clock::from_time_t(value);
		}

		template <class Alloc>
		inline void
		column(int i, basic_u8string<Alloc>& value) const {
			using char_type = typename basic_u8string<Alloc>::value_type;
			auto* ptr = ::sqlite3_column_text(this->_ptr, i);
			if (!ptr) { value.clear(); }
			else { value = reinterpret_cast<const char_type*>(ptr); }
		}

		template <class Alloc>
		inline void
		column(int i, basic_u16string<Alloc>& value) const {
			using char_type = typename basic_u16string<Alloc>::value_type;
			auto* ptr = ::sqlite3_column_text16(this->_ptr, i);
			if (!ptr) { value.clear(); }
			else { value = reinterpret_cast<const char_type*>(ptr); }
		}

		inline void
		column(int i, blob& value) const {
			auto* ptr = ::sqlite3_column_blob(this->_ptr, i);
			auto n = ::sqlite3_column_bytes(this->_ptr, i);
			if (!ptr) { value.clear(); }
			else { value.assign(reinterpret_cast<const blob::value_type*>(ptr), n); }
		}

		inline void
		column(int i, any_base& value) const noexcept {
			value.clear(::sqlite3_column_value(this->_ptr, i));
		}

		template <class Float>
		inline auto
		bind(int i, Float value) ->
		typename std::enable_if<std::is_floating_point<Float>::value>::type {
			call(::sqlite3_bind_double(this->_ptr, i, static_cast<double>(value)));
		}

		template <class Integer>
		inline auto
		bind(int i, Integer value) ->
		typename std::enable_if<std::is_integral<Integer>::value &&
		(sizeof(Integer) <= sizeof(int))>::type {
			call(::sqlite3_bind_int(this->_ptr, i, static_cast<int>(value)));
		}

		template <class Integer>
		inline auto
		bind(int i, Integer value) ->
		typename std::enable_if<std::is_integral<Integer>::value &&
		(sizeof(Integer) > sizeof(int))>::type {
			call(::sqlite3_bind_int64(this->_ptr, i, static_cast<int64>(value)));
		}

		inline void
		bind(int i, const char* value, destructor destr=pass_by_copy) {
			call(::sqlite3_bind_text(this->_ptr, i, value, -1, destr));
		}

		template <class Alloc>
		inline void
		bind(int i, const basic_u8string<Alloc>& value, destructor destr=pass_by_copy) {
			call(::sqlite3_bind_text64(
				this->_ptr,
				i,
				value.data(),
				value.size(),
				destr,
				downcast(encoding::utf8)
			));
		}

		template <class Alloc>
		inline void
		bind(int i, const basic_u16string<Alloc>& value,
             encoding enc=encoding::utf16, destructor destr=pass_by_copy) {
			call(::sqlite3_bind_text64(
				this->_ptr,
				i,
				reinterpret_cast<const char*>(value.data()),
				value.size()*sizeof(char16_t),
				destr,
				downcast(enc)
			));
		}

		template <class Alloc>
		inline void
		bind(int i, const basic_u16string<Alloc>& value, destructor destr=pass_by_copy,
             encoding enc=encoding::utf16) {
			call(::sqlite3_bind_text64(
				this->_ptr,
				i,
				reinterpret_cast<const char*>(value.data()),
				value.size()*sizeof(char16_t),
				destr,
				downcast(enc)
			));
		}

		inline void
		bind(int i, any_base value) {
			call(::sqlite3_bind_value(this->_ptr, i, value.get()));
		}

		inline void
		bind(int i, const named_ptr& value) {
			call(::sqlite3_bind_pointer(
				this->_ptr,
				i,
				value.get(),
				value.name(),
				nullptr
			));
		}

		inline void
		bind(int i, const blob& value, destructor destr=pass_by_copy) {
			call(::sqlite3_bind_blob64(this->_ptr, i, value.get(), value.size(), destr));
		}

		inline void
		bind(int i, const zeroes& value) {
			call(::sqlite3_bind_zeroblob64(this->_ptr, i, value.size()));
		}

		template <class Clock, class Duration>
		inline void
		bind(int i, const std::chrono::time_point<Clock,Duration>& value) {
			this->bind(i, Clock::to_time_t(value));
		}

		inline void
		bind(int i, std::nullptr_t) {
			call(::sqlite3_bind_null(this->_ptr, i));
		}

		inline void clear() { call(::sqlite3_clear_bindings(this->_ptr)); }
		void dump(std::ostream& out);
		inline const char* sql() const noexcept { return ::sqlite3_sql(this->_ptr); }

		inline unique_ptr<char>
		expanded_sql() const {
			return unique_ptr<char>(::sqlite3_expanded_sql(this->_ptr));
		}

		connection_base connection();

		inline const char*
		column_name(int i) const noexcept {
			return ::sqlite3_column_name(this->_ptr, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		column_name(int i) const noexcept;

		inline data_type
		column_type(int i) const noexcept {
			return static_cast<data_type>(
				::sqlite3_column_type(this->_ptr, i)
			);
		}

		inline const char*
		database_name(int i) const noexcept {
			return ::sqlite3_column_database_name(this->_ptr, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		database_name(int i) const noexcept;

		inline const char*
		table_name(int i) const noexcept {
			return ::sqlite3_column_table_name(this->_ptr, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		table_name(int i) const noexcept;

		inline const char*
		origin_name(int i) const noexcept {
			return ::sqlite3_column_origin_name(this->_ptr, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		origin_name(int i) const noexcept;

		inline const char*
		column_type_name(int i) const noexcept {
			return ::sqlite3_column_decltype(this->_ptr, i);
		}

		template <encoding enc>
		inline typename encoding_traits<enc>::string
		column_type_name(int i) const noexcept;

		inline int
		column_size(int i) const noexcept {
			return ::sqlite3_column_bytes(this->_ptr, i);
		}

		template <encoding enc>
		inline int
		column_size(int i) const noexcept;

		template <class T> inline row_iterator<T> begin() { return row_iterator<T>(this); }
		template <class T> inline row_iterator<T> end() noexcept { return row_iterator<T>(); }

		template <class T>
		inline iterator_view<row_iterator<T>> rows() { return {begin<T>(),end<T>()}; }

	};

	inline void swap(statement& lhs, statement& rhs) noexcept { lhs.swap(rhs); }

	template <class T>
	class row_iterator {

	public:
		using value_type = T;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = value_type&;
		using difference_type = std::ptrdiff_t;
		using size_type = std::size_t;
		using iterator_category = std::input_iterator_tag;

	private:
		statement* _ptr = nullptr;
		value_type _value;

	public:

		inline explicit
		row_iterator(statement& ptr): _ptr(&ptr) {
			if (!this->_ptr->busy()) { advance(); }
		}

		inline explicit
		row_iterator(statement* ptr): _ptr(ptr) {
			if (!this->_ptr->busy()) { advance(); }
		}

		row_iterator() = default;
		row_iterator(const row_iterator&) = default;
		row_iterator& operator=(const row_iterator&) = default;
		row_iterator(row_iterator&&) = default;
		row_iterator& operator=(row_iterator&&) = default;

		inline bool
		operator==(const row_iterator& rhs) const noexcept {
			return this->_ptr == rhs._ptr;
		}

		inline bool
		operator!=(const row_iterator& rhs) const noexcept {
			return !operator==(rhs);
		}

		inline reference operator*() noexcept { return this->_value; }
		inline pointer operator->() noexcept { return &this->_value; }
		inline const_reference operator*() const noexcept { return this->_value; }
		inline const_pointer operator->() const noexcept { return &this->_value; }

		inline row_iterator& operator++() { this->advance(); return *this; }

		inline row_iterator
		operator++(int) {
			row_iterator tmp(*this);
			this->advance();
			return tmp;
		}

	private:

		inline void
		advance() {
			if (this->_ptr->step() == errc::done) {
				this->_ptr = nullptr;
			} else {
				static_cast<const statement&>(*this->_ptr) >> this->_value;
			}
		}

	};

	/**
	\brief Stream of columns.
	\date 2018-10-08
	\author Ivan Gankevich
	*/
	class cstream {

	private:
		const statement& _statement;
		int _column = 0;

	public:

		inline explicit cstream(const statement& rhs) noexcept: _statement(rhs) {}

		cstream() = delete;
		cstream(const cstream&) = delete;
		cstream& operator=(const cstream&) = delete;
		cstream(cstream&&) = delete;
		cstream& operator=(cstream&&) = delete;

		inline void clear() noexcept { this->_column = 0; }
		inline const statement& parent() const noexcept { return this->_statement; }
		inline data_type column_type() const noexcept { return parent().column_type(this->_column); }
		inline int num_columns() const noexcept { return parent().num_columns(); }
		inline int column() const noexcept { return this->_column; }

		template <class T>
		inline cstream&
		operator>>(T& rhs) {
			this->_statement.column(this->_column, rhs);
			++this->_column;
			return *this;
		}

	};

	inline void
	bind(statement&,int) {}

	template <class Head, class ... Tail>
	inline void
	bind(statement& rstr, int i, const Head& head, const Tail& ... tail) {
		rstr.bind(i, head);
		bind(rstr, i+1, tail...);
	}

	inline void
	bind(statement&) {}

	template <class Head, class ... Tail>
	inline void
	bind(statement& rstr, const Head& head, const Tail& ... tail) {
		bind(rstr, 1, tail...);
	}

	template <>
	inline u8string
	statement::column_name<encoding::utf8>(int i) const noexcept {
		return ::sqlite3_column_name(this->_ptr, i);
	}

	template <>
	inline u16string
	statement::column_name<encoding::utf16>(int i) const noexcept {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_name16(this->_ptr, i)
		);
	}

	template <>
	inline u8string
	statement::database_name<encoding::utf8>(int i) const noexcept {
		return ::sqlite3_column_database_name(this->_ptr, i);
	}

	template <>
	inline u16string
	statement::database_name<encoding::utf16>(int i) const noexcept {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_database_name16(this->_ptr, i)
		);
	}

	template <>
	inline u8string
	statement::table_name<encoding::utf8>(int i) const noexcept {
		return ::sqlite3_column_table_name(this->_ptr, i);
	}

	template <>
	inline u16string
	statement::table_name<encoding::utf16>(int i) const noexcept {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_table_name16(this->_ptr, i)
		);
	}

	template <>
	inline u8string
	statement::origin_name<encoding::utf8>(int i) const noexcept {
		return ::sqlite3_column_origin_name(this->_ptr, i);
	}

	template <>
	inline u16string
	statement::origin_name<encoding::utf16>(int i) const noexcept {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_origin_name16(this->_ptr, i)
		);
	}

	template <>
	inline u8string
	statement::column_type_name<encoding::utf8>(int i) const noexcept {
		return ::sqlite3_column_decltype(this->_ptr, i);
	}

	template <>
	inline u16string
	statement::column_type_name<encoding::utf16>(int i) const noexcept {
		return reinterpret_cast<const char16_t*>(
			::sqlite3_column_decltype16(this->_ptr, i)
		);
	}

	template <>
	inline int
	statement::column_size<encoding::utf8>(int i) const noexcept {
		return ::sqlite3_column_bytes(this->_ptr, i);
	}

	template <>
	inline int
	statement::column_size<encoding::utf16>(int i) const noexcept {
		return ::sqlite3_column_bytes16(this->_ptr, i);
	}

}

namespace std {
    template <class T>
    struct iterator_traits<sqlite::row_iterator<T>> {
		using value_type = T;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = value_type&;
		using difference_type = ptrdiff_t;
		using size_type = size_t;
		using iterator_category = std::input_iterator_tag;
    };
}

#endif // vim:filetype=cpp
