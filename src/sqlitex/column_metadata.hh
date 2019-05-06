#ifndef SQLITEX_COLUMN_METADATA_HH
#define SQLITEX_COLUMN_METADATA_HH

#include <sqlitex/forward.hh>

namespace sqlite {

	class column_metadata {

	private:
		unique_ptr<char> _type;
		unique_ptr<char> _collation;
		bool _notnull = false;
		bool _primarykey = false;
		bool _autoincrement = false;

	public:

		inline const char* type() const { return this->_type.get(); }
		inline const char* collation() const { return this->_collation.get(); }
		inline bool not_null() const { return this->_notnull; }
		inline bool primary_key() const { return this->_primarykey; }
		inline bool auto_increment() const { return this->_autoincrement; }

		friend class database;

	};

}

#endif // vim:filetype=cpp
