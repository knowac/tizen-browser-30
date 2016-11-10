/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SQLDATABASEIMPL_H_
#define SQLDATABASEIMPL_H_

#include <sqlite3.h>
#include "SQLDatabase.h"

namespace tizen_browser {
namespace storage {

class SQLQueryPrivate
{
public:
	std::weak_ptr<SQLDatabase> _db_ref;
	sqlite3 * _db;
	sqlite3_stmt * _stmt;
	bool _hasNext;
	std::string _query;

	SQLQueryPrivate(std::shared_ptr<SQLDatabase> db_ref, sqlite3 * db, sqlite3_stmt * stmt, const std::string& query);
	SQLQueryPrivate(const SQLQueryPrivate& other);
	~SQLQueryPrivate();
};

class SQLDatabasePrivate
{
public:
	SQLDatabasePrivate();
	~SQLDatabasePrivate();

	void close();

	std::string _path;
	sqlite3 * _db;
	std::weak_ptr<SQLDatabase> _db_self_weak;
};

}
}

#endif /* SQLDATABASEIMPL_H_ */
