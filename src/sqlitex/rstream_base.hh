#ifndef SQLITE_RSTREAM_BASE_HH
#define SQLITE_RSTREAM_BASE_HH

namespace sqlite {

	class rstream_base {

	public:
		enum state {
			goodbit = 0,
			failbit = 1,
			badbit = 2,
			eofbit = 4
		};

	private:
		state _state = goodbit;

	protected:

		rstream_base() = default;
		rstream_base(const rstream_base&) = default;
		rstream_base(rstream_base&&) = default;
		~rstream_base() = default;

	public:

		inline explicit
		operator bool() const noexcept {
			return this->_state == goodbit;
		}

		inline bool
		operator!() const noexcept {
			return !operator bool();
		}

		inline void
		clear(state rhs) noexcept {
			this->_state = rhs;
		}

		inline bool
		good() const noexcept {
			return !this->_state;
		}

		inline bool
		bad() const noexcept {
			return this->_state & badbit;
		}

		inline bool
		fail() const noexcept {
			return this->_state & failbit;
		}

		inline bool
		eof() const noexcept {
			return this->_state & eofbit;
		}

		inline state
		rdstate() const noexcept {
			return this->_state;
		}

		inline void
		setstate(state rhs) noexcept {
			this->_state = state(this->_state | rhs);
		}

	};

}

#endif // vim:filetype=cpp
