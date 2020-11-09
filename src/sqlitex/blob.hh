#ifndef SQLITEX_BLOB_HH
#define SQLITEX_BLOB_HH

#include <streambuf>
#include <string>

#include <sqlitex/errc.hh>
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

		inline void* get() noexcept { return &this->std::string::operator[](0); }
		inline const void* get() const noexcept { return this->std::string::data(); }

	};

	class zeroes {

	private:
		uint64 _size = 0;

	public:
		inline explicit zeroes(uint64 size) noexcept: _size(size) {}
		inline uint64 size() const noexcept { return this->_size; }

	};

	class blob_buffer {

	private:
		types::blob* _ptr = nullptr;

	public:

		blob_buffer() = default;
		inline explicit blob_buffer(types::blob* ptr) noexcept: _ptr(ptr) {}
		inline ~blob_buffer() noexcept { ::sqlite3_blob_close(this->_ptr); }
		blob_buffer(const blob_buffer&) = delete;
		blob_buffer& operator=(const blob_buffer&) = delete;
		inline blob_buffer(blob_buffer&& rhs) noexcept: _ptr(rhs._ptr) { rhs._ptr = nullptr; }
		inline blob_buffer& operator=(blob_buffer&& rhs) noexcept { this->swap(rhs); return *this; }
		inline void swap(blob_buffer& rhs) noexcept { std::swap(this->_ptr, rhs._ptr); }
		inline int size() const noexcept { return ::sqlite3_blob_bytes(this->_ptr); }
		inline void reopen(int64 rowid) { call(::sqlite3_blob_reopen(this->_ptr, rowid)); }

		inline void
		write(const void* data, int size, int offset) {
			call(::sqlite3_blob_write(this->_ptr, data, size, offset));
		}

		inline void
		read(void* data, int size, int offset) {
			call(::sqlite3_blob_read(this->_ptr, data, size, offset));
		}

		friend class connection;

	};

	inline void swap(blob_buffer& lhs, blob_buffer& rhs) noexcept { lhs.swap(rhs); }

	class blob_streambuf: public std::streambuf {

	private:
		using base_type = std::streambuf;
		using seekdir = std::ios_base::seekdir;
		using openmode = std::ios_base::openmode;
		using size_type = std::streamsize;

	public:
		using base_type::char_type;
		using base_type::traits_type;
		using base_type::int_type;
		using base_type::pos_type;
		using base_type::off_type;

	private:
		blob_buffer _buffer;
		int _poffset = 0;
		int _goffset = 0;
		int _size = 0;

	public:
		blob_streambuf() = default;
		blob_streambuf(const blob_streambuf&) = delete;
		blob_streambuf& operator=(const blob_streambuf&) = delete;
		inline explicit blob_streambuf(blob_buffer&& rhs): _buffer(std::move(rhs)) {}

	protected:
		int_type overflow(int_type c) override;
		int_type underflow() override;
		size_type xsputn(const char_type* s, size_type n) override;
		size_type xsgetn(char_type* s, size_type n) override;
		size_type showmanyc() override;
		int_type pbackfail(int_type c) override;
		pos_type seekoff(off_type off, seekdir dir, openmode which) override;
		pos_type seekpos(pos_type pos, openmode which) override;

	};

}

#endif // vim:filetype=cpp
