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

#include "SQLDatabase.h"
#include "SQLDatabaseImpl.h"
#include "StorageException.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"

#include <string.h>
#include <stdlib.h>
#include <chrono>
#include <thread>

#include <Eina.h>

namespace tizen_browser {
namespace storage {

#define SQL_RETRY_TIME_US	    100000
#define SQL_RETRY_COUNT    200

static FieldPtr _null_field(new Field());

static inline int sql_prepare(sqlite3 * db, sqlite3_stmt ** stmt, const char * query)
{
    int retry = 0;
    int rc;

    do {
        rc = sqlite3_prepare_v2(db, query, -1, stmt, NULL);
        if(rc == SQLITE_BUSY || rc == SQLITE_LOCKED) {
            ++retry;
            std::this_thread::sleep_for(std::chrono::milliseconds(SQL_RETRY_TIME_US));
        }
    } while(retry < SQL_RETRY_COUNT && (rc == SQLITE_BUSY || rc == SQLITE_LOCKED));

    if(rc != SQLITE_OK) {
        BROWSER_LOGE("[sql_db] Can't prepare query from string '%s' with result %d (%s)",
                query,
                rc,
                sqlite3_errmsg(db));

        return rc;
    }

    return rc;
}

static inline int sql_step(sqlite3_stmt * stmt)
{
    int rc;
    int retry = 0;

    do {
        rc = sqlite3_step(stmt);

        if(rc == SQLITE_LOCKED) {
            rc = sqlite3_reset(stmt);
            ++retry;
            std::this_thread::sleep_for(std::chrono::milliseconds(SQL_RETRY_TIME_US));
        } else if(rc == SQLITE_BUSY) {
            ++retry;
            std::this_thread::sleep_for(std::chrono::milliseconds(SQL_RETRY_TIME_US));
        }
    } while(retry < SQL_RETRY_COUNT && (rc == SQLITE_BUSY || rc == SQLITE_LOCKED));

    if(retry == SQL_RETRY_COUNT) {
        BROWSER_LOGE("[sql_db] Database timeout");
    }

    if(rc == SQLITE_MISUSE) {
        BROWSER_LOGE("[sql_db] Sqlite misuse");
    }

    return rc;
}

static inline bool sql_begin(sqlite3 * db)
{
    sqlite3_stmt * stmt  = 0;
    if(sql_prepare(db, &stmt, "BEGIN EXCLUSIVE TRANSACTION")) {
        BROWSER_LOGE("[sql_db] Can't begin SQL transaction");
        return false;
    }

    int result = sql_step(stmt);
    sqlite3_finalize(stmt);

    if(result && result != SQLITE_DONE) {
        BROWSER_LOGE("[sql_db] Error while starting transaction");
        return false;
    }

    return true;
}

static inline bool sql_commit(sqlite3 * db)
{
    sqlite3_stmt * stmt  = 0;
    if(sql_prepare(db, &stmt, "COMMIT")) {
        BROWSER_LOGE("[sql_db] Can't commit SQL transaction");
        return false;
    }

    int result = sql_step(stmt);
    sqlite3_finalize(stmt);

    if(result && result != SQLITE_DONE) {
        BROWSER_LOGD("[sql_db] Error while commiting transaction");
        return false;
    }

    return true;
}

static inline bool sql_rollback(sqlite3 * db)
{
    sqlite3_stmt * stmt  = 0;
    if(sql_prepare(db, &stmt, "ROLLBACK")) {
        BROWSER_LOGE("[sql_db] Can't rollback SQL transaction");
        return false;
    }

    int result = sql_step(stmt);
    sqlite3_finalize(stmt);

    if(result && result != SQLITE_DONE) {
        BROWSER_LOGD("[sql_db] Error while rolling transaction back");
        return false;
    }

    return true;
}

SQLQueryPrivate::SQLQueryPrivate(std::shared_ptr<SQLDatabase> db_ref,
        sqlite3 * db,
        sqlite3_stmt * stmt,
        const std::string& query) :
    _db_ref(db_ref),
    _db(db),
    _stmt(stmt),
    _hasNext(false),
    _query(query)
{
}

SQLQueryPrivate::SQLQueryPrivate(const SQLQueryPrivate& other) :
    _db_ref(other._db_ref),
    _db(other._db),
    _stmt(other._stmt),
    _hasNext(other._hasNext),
    _query(other._query)
{
    const_cast<SQLQueryPrivate &>(other)._stmt = NULL;
}

SQLQueryPrivate::~SQLQueryPrivate()
{
    if(_stmt)
        sqlite3_finalize(_stmt);
}

SQLDatabasePrivate::SQLDatabasePrivate() :
    _db(NULL)
{
}

SQLDatabasePrivate::~SQLDatabasePrivate()
{
    close();
}

void SQLDatabasePrivate::close()
{
    if(_db) {
        sqlite3_close(_db);
        _db = NULL;
    }
}

SQLQuery::SQLQuery() :
    d(NULL)
{
}

SQLQuery::SQLQuery(const SQLQuery& other)
{
    if(other.d)
        d = new SQLQueryPrivate(*other.d);
    else
        d = NULL;
}

SQLQuery::~SQLQuery()
{
    delete d;
}

SQLQuery& SQLQuery::operator = (const SQLQuery& other)
{
    if(d != other.d) {
        if(other.d) {
            SQLQueryPrivate * new_d = new SQLQueryPrivate(*other.d);
            delete d;
            d = new_d;
        } else {
            delete d;
            d = NULL;
        }
    }
    return *this;
}

bool SQLQuery::isValid() const
{
    return d && d->_stmt && !d->_db_ref.expired();
}

void SQLQuery::bindText(int paramNo, const char * text, int length)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_text(d->_stmt, paramNo, text, length, NULL);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

void SQLQuery::bindText(int paramNo, const std::string& text)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_text(d->_stmt, paramNo, text.c_str(), text.length(), NULL);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

void SQLQuery::bindBlob(int paramNo, std::unique_ptr<tizen_browser::tools::Blob> blob)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_blob(d->_stmt, paramNo, blob->getData(), blob->getLength(),
                                  SQLITE_TRANSIENT);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

void SQLQuery::bindBlob(int paramNo, const void * data, size_t length)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_blob(d->_stmt, paramNo, data, length, SQLITE_TRANSIENT);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

void SQLQuery::bindInt(int paramNo, int value)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_int(d->_stmt, paramNo, value);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

void SQLQuery::bindInt64(int paramNo, long long value)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_int64(d->_stmt, paramNo, value);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

void SQLQuery::bindDouble(int paramNo, double value)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_double(d->_stmt, paramNo, value);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

void SQLQuery::bindNull(int paramNo)
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int error = sqlite3_bind_null(d->_stmt, paramNo);

    if (error != SQLITE_OK) {
        throw StorageException(sqlite3_errmsg(d->_db), error);
    }
}

const char * SQLQuery::getCString(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    return (const char *)sqlite3_column_text(d->_stmt, column);
}

std::string SQLQuery::getString(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    const char * str = (const char *)sqlite3_column_text(d->_stmt, column);

    if(!str)
        return std::string();

    return str;
}

int SQLQuery::getInt(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    return sqlite3_column_int(d->_stmt, column);
}

long long SQLQuery::getInt64(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    return sqlite3_column_int64(d->_stmt, column);
}

double SQLQuery::getDouble(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    return sqlite3_column_double(d->_stmt, column);
}

std::shared_ptr<tizen_browser::tools::Blob> SQLQuery::getBlob(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    const void * blob = sqlite3_column_blob(d->_stmt, column);

    if(blob == NULL) {
        return std::shared_ptr<tizen_browser::tools::Blob>();
    }

    int length = sqlite3_column_bytes(d->_stmt, column);

    return std::make_shared<tizen_browser::tools::Blob>(blob, length);
}

size_t SQLQuery::getDataLength(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int result = sqlite3_column_bytes(d->_stmt, column);

    if(result <= 0) {
        if(sqlite3_errcode(d->_db) != SQLITE_OK) {
            throw StorageException(sqlite3_errmsg(d->_db), sqlite3_errcode(d->_db));
        }
    }

    return result;
}

int SQLQuery::fieldType(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    return sqlite3_column_type(d->_stmt, column);
}

FieldPtr SQLQuery::getField(int column) const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    int col_type = sqlite3_column_type(d->_stmt, column);

    if(col_type == 0)
        throw StorageException(sqlite3_errmsg(d->_db), sqlite3_errcode(d->_db));

    switch(col_type)
    {
    case SQLITE_INTEGER:
        return std::make_shared<Field>(sqlite3_column_int(d->_stmt, column));
    case SQLITE_BLOB:
        return std::make_shared<Field>(getBlob(column));
    case SQLITE_NULL:
        return _null_field;
    case SQLITE_TEXT:
        return std::make_shared<Field>(getString(column));
    case SQLITE_FLOAT:
        return std::make_shared<Field>(getDouble(column));
    default:
        throw _null_field;
    }
}

int SQLQuery::columnCount() const
{
    M_ASSERT(d);
    M_ASSERT(d->_stmt);
    M_ASSERT(!d->_db_ref.expired());

    return sqlite3_column_count(d->_stmt);
}

bool SQLQuery::hasNext() const
{
    M_ASSERT(d);

    return d->_hasNext;
}

bool SQLQuery::next()
{
    M_ASSERT(d);
    M_ASSERT(!d->_db_ref.expired());

    if(d->_hasNext) {
        int error = sql_step(d->_stmt);

        if (error == SQLITE_DONE) {
            d->_hasNext = false;
            return true;
        } else if (error == SQLITE_ROW) {
            d->_hasNext = true;
            return true;
        } else {
            d->_hasNext = false;
            BROWSER_LOGE("[SQLQuery] Unknown result received while executing query - %d - %s", error, d->_query.c_str());
            return false;
        }
    }

    return false;
}

void SQLQuery::reset()
{
    M_ASSERT(d);

    if(d->_stmt) {
        M_ASSERT(!d->_db_ref.expired());
        sqlite3_reset(d->_stmt);
    }
}

void SQLQuery::clearBindings()
{
    M_ASSERT(d);

    if(d->_stmt) {
        M_ASSERT(!d->_db_ref.expired());
        sqlite3_clear_bindings(d->_stmt);
    }
}

void SQLQuery::exec()
{
    M_ASSERT(d);
    M_ASSERT(!d->_db_ref.expired());

    if(!d->_stmt) {
        throw StorageException("[SQLQuery] Statement not active", 0);
    }

    int error = sql_step(d->_stmt);

    if (error == SQLITE_DONE) {
        // No more data available
        d->_hasNext= false;
    } else if (error == SQLITE_ROW) {
        // Data is available
        d->_hasNext = true;
    } else if (error == SQLITE_ERROR) {
        BROWSER_LOGE("[SQLQuery] Can't execute SELECT query because of error '%s' - %s", sqlite3_errmsg(d->_db), d->_query.c_str());
        d->_hasNext = false;
        throw StorageException(sqlite3_errmsg(d->_db), sqlite3_errcode(d->_db));
    } else {
        BROWSER_LOGE("[SQLQuery] Can't execute SELECT query because of result %d - %s", error, d->_query.c_str());
        throw StorageException(sqlite3_errmsg(d->_db), sqlite3_errcode(d->_db));
    }
}

std::vector<std::string> SQLQuery::columnNames() const
{
    M_ASSERT(d);
    M_ASSERT(!d->_db_ref.expired());
    M_ASSERT(d->_stmt);

    std::vector<std::string> result;

    int cols = sqlite3_column_count(d->_stmt);
    result.reserve(cols);

    for(int i = 0 ; i < cols ; ++i) {
        result.push_back(std::string(sqlite3_column_name(d->_stmt, i)));
    }

    return result;
}

std::shared_ptr<SQLDatabase> SQLDatabase::make_shared()
{
    return std::shared_ptr<SQLDatabase>(new SQLDatabase());
}

SQLDatabase::SQLDatabase() :
    d(new SQLDatabasePrivate())
{
}

SQLDatabase::~SQLDatabase()
{
    delete d;
}

std::shared_ptr<SQLDatabase> SQLDatabase::newInstance()
{
    std::shared_ptr<SQLDatabase> db = make_shared();
    db->d->_db_self_weak = db;
    return db;
}

void SQLDatabase::open(const std::string& path)
{
    M_ASSERT(!d->_db);

    int error = sqlite3_open_v2(path.c_str(), &d->_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    if(error != SQLITE_OK) {
        BROWSER_LOGE("[SQLDatabase] Can't open sqlite database from file '%s' with error %d",
                path.c_str(),
                error);

        throw StorageException(sqlite3_errmsg(d->_db), error);
    }

    SQLQuery query(prepare("PRAGMA foreign_keys = ON"));
    query.exec();
}

void SQLDatabase::close()
{
    d->close();
}

SQLQuery SQLDatabase::prepare(const std::string& query) const
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(d->_db, SQLQuery());

    sqlite3_stmt * stmt = NULL;
    int rc = sql_prepare(d->_db, &stmt, query.c_str());

    if(rc != SQLITE_OK) {
        BROWSER_LOGE("[SQLDatabase] Can't prepare query from string '%s' with result %d (%s)",
                query.c_str(),
                rc,
                sqlite3_errmsg(d->_db));

        return SQLQuery();
    }

    SQLQuery result;

    try {
        result.d = new SQLQueryPrivate(d->_db_self_weak.lock(), d->_db, stmt, query);
    } catch(...) {
        sqlite3_finalize(stmt);
        throw;
    }

    return result;
}

bool SQLDatabase::tableExists(const std::string& name) const
{
    if(!d->_db)
        return false;

    SQLQuery query(prepare("select count(*) from sqlite_master where type='table' and name=?"));
    query.bindText(1, name);
    query.exec();

    return query.getInt(0) > 0;
}

long long SQLDatabase::lastInsertId() const
{
    BROWSER_LOGD("lastInsertId");
    if(!d->_db)
        return -1;

    long long id = sqlite3_last_insert_rowid(d->_db);

    BROWSER_LOGD("lastInsertId: %d", id);
    return id;
}

bool SQLDatabase::exec(const std::string& command) const
{
    if(!d->_db)
        return false;

    sqlite3_stmt * stmt = NULL;
    int rc = sql_prepare(d->_db, &stmt, command.c_str());

    if(rc != SQLITE_OK) {
        const char * errorMessage = sqlite3_errmsg(d->_db);
        int errorCode = sqlite3_errcode(d->_db);
        std::string error( errorMessage ? errorMessage : "" );
        BROWSER_LOGE("[SQLDatabase] Can't prepare query from string '%s' with result %d (%s)",
                command.c_str(),
                rc,
                error.c_str());
        throw StorageException(error, errorCode);
//         return false;
    }

    rc = sql_step(stmt);
    sqlite3_finalize(stmt);

    if(rc != SQLITE_DONE && rc != SQLITE_ROW) {
        BROWSER_LOGE("[SQLDatabase] Can't execute query from string '%s' with result %d (%s)",
                command.c_str(),
                rc,
                sqlite3_errmsg(d->_db));
        throw StorageException(sqlite3_errmsg(d->_db), sqlite3_errcode(d->_db));
    }

    return true;
}

std::vector<std::string> SQLDatabase::tableColumnNames(const std::string& table) const
{
    std::string query_str("PRAGMA table_info(");
    query_str.append(table);
    query_str.append(")");

    std::vector<std::string> result;

    SQLQuery query(prepare(query_str));
    query.exec();

    while(query.hasNext()) {
        result.push_back(query.getString(1));
        query.next();
    }

    return result;
}

void SQLDatabase::begin()
{
    if(!sql_begin(d->_db)) {
        throw StorageException(sqlite3_errmsg(d->_db), sqlite3_errcode(d->_db));
    }
}

void SQLDatabase::commit()
{
    if(!sql_commit(d->_db)) {
        throw StorageException(sqlite3_errmsg(d->_db), sqlite3_errcode(d->_db));
    }
}

void SQLDatabase::rollback()
{
    sql_rollback(d->_db);
}

std::shared_ptr<SQLDatabase> SQLDatabase::cloneForThread(void)
{
    std::shared_ptr<SQLDatabase> result(newInstance());
    result->open(d->_path);
    return result;
}

} /* namespace storage */
} /* namespace tizen_browser */
