#ifndef SQLITEX_TRANSACTION_HH
#define SQLITEX_TRANSACTION_HH

#include <sqlitex/connection.hh>

namespace sqlite {

	#define SQLITEX_TRANSACTION(NAME, type) \
		class NAME { \
		private: \
			connection& _db; \
			bool _success = false; \
		public: \
			inline explicit \
			NAME(connection& db): _db(db) { \
				this->_db.execute("BEGIN " type " TRANSACTION"); \
			} \
			inline ~NAME() { \
				if (!this->_success) { \
					this->_db.execute("ROLLBACK"); \
				} \
			} \
			inline void commit() { \
				this->_db.execute("COMMIT"); \
				this->_success = true; \
			} \
		}

	SQLITEX_TRANSACTION(deferred_transaction, "DEFERRED");
	SQLITEX_TRANSACTION(immediate_transaction, "IMMEDIATE");
	SQLITEX_TRANSACTION(exclusive_transaction, "EXCLUSIVE");

}

#endif // vim:filetype=cpp
