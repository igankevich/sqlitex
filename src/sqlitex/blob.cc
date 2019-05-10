#include <sqlitex/blob.hh>

auto
sqlite::blob_streambuf::overflow(int_type c) -> int_type {
	if (traits_type::eq_int_type(c, traits_type::eof())) { return traits_type::eof(); }
	if (this->_poffset == this->_size) { return traits_type::eof(); }
	char_type ch = traits_type::to_char_type(c);
	this->_buffer.write(&ch, sizeof(char_type), this->_poffset);
	++this->_poffset;
	return traits_type::to_int_type(c);
}

auto
sqlite::blob_streambuf::xsputn(const char_type* s, size_type n) -> size_type {
	auto m = std::min(n, size_type(this->_size - this->_poffset));
	if (m == 0) { return 0; }
	this->_buffer.write(s, m, this->_poffset);
	this->_poffset += m;
	return m;
}

auto
sqlite::blob_streambuf::underflow() -> int_type {
	if (this->_goffset == this->_size) { return traits_type::eof(); }
	char_type ch = 0;
	this->_buffer.read(&ch, sizeof(char_type), this->_goffset);
	++this->_goffset;
	return traits_type::to_int_type(ch);
}

auto
sqlite::blob_streambuf::xsgetn(char_type* s, size_type n) -> size_type {
	auto m = std::min(n, size_type(this->_size - this->_goffset));
	if (m == 0) { return 0; }
	this->_buffer.read(s, m, this->_goffset);
	this->_goffset += m;
	return m;
}

auto
sqlite::blob_streambuf::showmanyc() -> size_type {
	return this->_size - this->_goffset;
}

auto
sqlite::blob_streambuf::pbackfail(int_type c) -> int_type {
	if (this->_goffset == 0) { return traits_type::eof(); }
	--this->_goffset;
	if (!traits_type::eq_int_type(c, traits_type::eof())) {
		char_type ch = traits_type::to_char_type(c);
		this->_buffer.write(&ch, sizeof(char_type), this->_goffset);
	}
	return traits_type::not_eof(c);
}

auto
sqlite::blob_streambuf::seekoff(off_type off, seekdir dir, openmode which) -> pos_type {
	if (which & std::ios_base::in) {
		switch (dir) {
			case std::ios_base::beg: this->_goffset = static_cast<int>(off); break;
			case std::ios_base::cur: this->_goffset += static_cast<int>(off); break;
			case std::ios_base::end: this->_goffset = this->_size + static_cast<int>(off);
									 break;
			default: break;
		}
		return pos_type(this->_goffset);
	}
	if (which & std::ios_base::out) {
		switch (dir) {
			case std::ios_base::beg: this->_poffset = static_cast<int>(off); break;
			case std::ios_base::cur: this->_poffset += static_cast<int>(off); break;
			case std::ios_base::end: this->_poffset = this->_size + static_cast<int>(off);
									 break;
			default: break;
		}
		return pos_type(this->_poffset);
	}
	return pos_type(off_type(-1));
}

auto
sqlite::blob_streambuf::seekpos(pos_type pos, openmode which) -> pos_type {
	if (which & std::ios_base::in) { this->_goffset = static_cast<int>(pos); }
	if (which & std::ios_base::out) { this->_poffset = static_cast<int>(pos); }
	return pos;
}
