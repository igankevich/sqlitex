#ifndef SQLITEX_SNAPSHOT_HH
#define SQLITEX_SNAPSHOT_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class snapshot {

	private:
		types::snapshot* _ptr = nullptr;

	public:

		inline explicit snapshot(types::snapshot* ptr) noexcept: _ptr(ptr) {}
		inline ~snapshot() noexcept { ::sqlite3_snapshot_free(this->_ptr); }
		snapshot(const snapshot&) = delete;
		snapshot& operator=(const snapshot&) = delete;

		inline snapshot(snapshot&& rhs) noexcept: _ptr(rhs._ptr) { rhs._ptr = nullptr; }
		inline snapshot& operator=(snapshot&& rhs) noexcept { this->swap(rhs); return *this; }
		inline void swap(snapshot& rhs) noexcept { std::swap(this->_ptr, rhs._ptr); }

		inline types::snapshot* get() noexcept { return this->_ptr; }
		inline const types::snapshot* get() const noexcept { return this->_ptr; }

		inline int compare(const snapshot& rhs) const noexcept{
			return ::sqlite3_snapshot_cmp(this->_ptr, rhs._ptr);
		}

	};

	inline void swap(snapshot& lhs, snapshot& rhs) noexcept { lhs.swap(rhs); }

	inline bool
	operator==(const snapshot& lhs, const snapshot& rhs) noexcept {
		return lhs.compare(rhs) == 0;
	}

	inline bool
	operator!=(const snapshot& lhs, const snapshot& rhs) noexcept {
		return !operator==(lhs, rhs);
	}

	inline bool
	operator<(const snapshot& lhs, const snapshot& rhs) noexcept {
		return lhs.compare(rhs) < 0;
	}

	inline bool
	operator<=(const snapshot& lhs, const snapshot& rhs) noexcept {
		return lhs.compare(rhs) <= 0;
	}

	inline bool
	operator>(const snapshot& lhs, const snapshot& rhs) noexcept {
		return lhs.compare(rhs) > 0;
	}

	inline bool
	operator>=(const snapshot& lhs, const snapshot& rhs) noexcept {
		return lhs.compare(rhs) >= 0;
	}

}

#endif // vim:filetype=cpp
