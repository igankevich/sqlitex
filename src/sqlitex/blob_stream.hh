#ifndef SQLITEX_BLOB_STREAM_HH
#define SQLITEX_BLOB_STREAM_HH

#include <sqlitex/errc.hh>
#include <sqlitex/forward.hh>

namespace sqlite {

	class blob_stream {

	private:
		types::blob* _ptr = nullptr;
		int _offset = 0;

	public:

		inline explicit blob_stream(types::blob* ptr): _ptr(ptr) {}
		inline ~blob_stream() { ::sqlite3_blob_close(this->_ptr); }
		blob_stream(const blob_stream&) = delete;
		blob_stream(blob_stream&&) = default;
		blob_stream& operator=(const blob_stream&) = delete;
		blob_stream& operator=(blob_stream&&) = default;

		inline int size() const { return ::sqlite3_blob_bytes(this->_ptr); }
		inline void reopen(int64 rowid) { call(::sqlite3_blob_reopen(this->_ptr, rowid)); }

		inline void
		write(const void* data, int size) {
			call(::sqlite3_blob_write(this->_ptr, data, size, this->_offset));
			this->_offset += size;
		}

		inline void
		read(void* data, int size) {
			call(::sqlite3_blob_read(this->_ptr, data, size, this->_offset));
			this->_offset += size;
		}

		friend class database;

	};

}

#endif // vim:filetype=cpp
