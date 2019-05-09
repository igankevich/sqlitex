#include <sqlitex/database.hh>

const char*
sqlite::to_string(locking_mode rhs) {
	switch (rhs) {
		case locking_mode::normal: return "NORMAL";
		case locking_mode::exclusive: return "EXCLUSIVE";
		default: return "UNKNOWN";
	}
}

void
sqlite::operator>>(const u8string& str, locking_mode& rhs) {
	if (str == "NORMAL") { rhs = locking_mode::normal; }
	else if (str == "EXCLUSIVE") { rhs = locking_mode::exclusive; }
	else { throw std::invalid_argument("bad locking mode"); }
}

const char*
sqlite::to_string(journal_mode rhs) {
	switch (rhs) {
		case journal_mode::del: return "DELETE";
		case journal_mode::truncate: return "TRUNCATE";
		case journal_mode::persist: return "PERSIST";
		case journal_mode::memory: return "MEMORY";
		case journal_mode::wal: return "WAL";
		case journal_mode::none: return "OFF";
		default: return "UNKNOWN";
	}
}

void
sqlite::operator>>(const u8string& str, journal_mode& rhs) {
	if (str == "DELETE") { rhs = journal_mode::del; }
	else if (str == "TRUNCATE") { rhs = journal_mode::truncate; }
	else if (str == "PERSIST") { rhs = journal_mode::persist; }
	else if (str == "MEMORY") { rhs = journal_mode::memory; }
	else if (str == "WAL") { rhs = journal_mode::wal; }
	else if (str == "OFF") { rhs = journal_mode::none; }
	else { throw std::invalid_argument("bad journal mode"); }
}
