#ifndef SQLITEX_SNAPSHOT_HH
#define SQLITEX_SNAPSHOT_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class snapshot {

	private:
		types::snapshot* _ptr = nullptr;

	public:

		inline explicit snapshot(types::snapshot* ptr): _ptr(ptr) {}
		inline ~snapshot() { ::sqlite3_snapshot_free(this->_ptr); }
		snapshot(const snapshot&) = delete;
		snapshot& operator=(const snapshot&) = delete;

		inline snapshot(snapshot&& rhs): _ptr(rhs._ptr) { rhs._ptr = nullptr; }
		inline snapshot& operator=(snapshot&& rhs) { this->swap(rhs); return *this; }
		inline void swap(snapshot& rhs) { std::swap(this->_ptr, rhs._ptr); }

		inline types::snapshot* get() { return this->_ptr; }
		inline const types::snapshot* get() const { return this->_ptr; }

		inline int
		compare(const snapshot& rhs) const {
			return ::sqlite3_snapshot_cmp(this->_ptr, rhs._ptr);
		}

	};

	inline void swap(snapshot& lhs, snapshot& rhs) { lhs.swap(rhs); }

	inline bool
	operator==(const snapshot& lhs, const snapshot& rhs) {
		return lhs.compare(rhs) == 0;
	}

	inline bool
	operator!=(const snapshot& lhs, const snapshot& rhs) {
		return !operator==(lhs, rhs);
	}

	inline bool
	operator<(const snapshot& lhs, const snapshot& rhs) {
		return lhs.compare(rhs) < 0;
	}

	inline bool
	operator<=(const snapshot& lhs, const snapshot& rhs) {
		return lhs.compare(rhs) <= 0;
	}

	inline bool
	operator>(const snapshot& lhs, const snapshot& rhs) {
		return lhs.compare(rhs) > 0;
	}

	inline bool
	operator>=(const snapshot& lhs, const snapshot& rhs) {
		return lhs.compare(rhs) >= 0;
	}

}

#endif // vim:filetype=cpp
