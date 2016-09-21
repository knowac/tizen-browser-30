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

#ifndef SQLDATABASE_H_
#define SQLDATABASE_H_

#include <memory>
#include <vector>
#include <string>

#include "Blob.h"
#include "Field.h"

namespace tizen_browser {
namespace storage {

class SQLDatabase;
class SQLQueryPrivate;
class SQLDatabasePrivate;

/*! \brief Represents query result.
 *
 * Parameter positions start from 1
 */
class SQLQuery
{
public:
	SQLQuery();
	SQLQuery(const SQLQuery&);
	~SQLQuery();
	SQLQuery& operator = (const SQLQuery&);

	/*! \brief Tell if query object is valid and its methods can still be accessed.
	 *
	 * \return True if query object is valid.
	 */
	bool isValid() const;

	/*! \brief Bind textual value (char const* C type) to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 * \param text - textual value to bind.
	 * \param length - length of textual value to bind.
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
	void bindText(int paramNo, const char * text, int length = -1);

	/*! \brief Bind textual value (C++ string) to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 * \param text - textual value to bind.
	 * \param length - length of textual value to bind.
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
	void bindText(int paramNo, const std::string& text);

	/*! \brief Bind blob to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 * \param blob - memory buffer to bind
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
	void bindBlob(int paramNo, std::unique_ptr<tizen_browser::tools::Blob> blob);

	/*! \brief Bind blob to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 * \param data - memory buffer to bind
	 * \param length - size of memory buffer to bind
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
	void bindBlob(int paramNo, const void * data, size_t length);


	/*! \brief Bind integral value (int C type) to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 * \param value - integral value to bind.
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
	void bindInt(int paramNo, int value);

	/*! \brief Bind large integral value (64 bit int C type) to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 * \param value - large integral value to bind.
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
	void bindInt64(int paramNo, long long value);

	/*! \brief Bind floating point value (double C type) to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 * \param value - floating point value to bind.
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
	void bindDouble(int paramNo, double value);

	/*! \brief Bind SQL NULL constant to positional parameter.
	 *
	 * \param paramNo - 1-based parameter number in query.
	 *
	 * \pre Query object must not be executed.
     * \throws StorageException on error
	 */
    void bindNull(int paramNo);

	/*! \brief Receive textual data from query result.
	 *
	 * \param column - 0-based index of a column to get data for.
	 * \return Textual value for given column in current row.
	 *
	 * \pre Query must be executed and valid and rows must be available to read.
	 */
	const char * getCString(int column) const;

	/*! \brief Receive textual data from query result.
	 *
	 * \param column - 0-based index of a column to get data for.
	 * \return Textual value for given column in current row.
	 *
	 * \pre Query must be executed and valid and rows must be available to read.
	 */
	std::string getString(int column) const;

	/*! \brief Receive integer data from query result.
	 *
	 * \param column - 0-based index of a column to get data for.
	 * \return Integer value for given column in current row.
	 *
	 * \pre Query must be executed and valid and rows must be available to read.
	 */
	int getInt(int column) const;

	/*! \brief Receive 64-bit integer data from query result.
	 *
	 * \param column - 0-based index of a column to get data for.
	 * \return 64-bit integer value for given column in current row.
	 *
	 * \pre Query must be executed and valid and rows must be available to read.
	 */
	long long getInt64(int column) const;

	/*! \brief Receive floating point data from query result.
	 *
	 * \param column - 0-based index of a column to get data for.
	 * \return Floating point value for given column in current row.
	 *
	 * \pre Query must be executed and valid and rows must be available to read.
	 */
	double getDouble(int column) const;

	/*! \brief Receive binary data from query result.
	 *
	 * \param column - 0-based index of a column to get data for.
	 * \return Binary data for given column in current row.
	 *
	 * \pre Query must be executed and valid and rows must be available to read.
     * \throws StorageException on error
	 */
	std::shared_ptr<tizen_browser::tools::Blob> getBlob(int column) const;

	/*! \brief Get length of data in given column for current row.
	 *
	 * \param column - 0-based index of a column to get length for.
	 * \return Size of data for given column in current row.
     * \throws StorageException on error
	 */
	size_t getDataLength(int column) const;


	/*! \brief Get field type for given column.
	 *
	 * \param column - 0-based index of a column to get type for.
	 * \return Type of given column.
	 *
	 * \pre Query must be executed and valid.
	 */
	int fieldType(int column) const;


	/*! \brief Get column as variant type
	 *
	 * @param column - 0-based index of column to get value of
	 * @return Value of field
     * @throws StorageException on error
	 */
	FieldPtr getField(int column) const;

	/*! \brief Get number of columns returned by this query.
	 *
	 * \return Number of columns returned by this query.
	 *
	 * \pre Query must be executed and valid.
	 */
	int columnCount() const;
	/*! \brief Check if there are more rows to be read for this query object.
	 *
	 * \return True if there are more rows available in result set.
	 *
	 * \pre Query must be executed and valid.
	 */
	bool hasNext() const;

	/*! \brief Move internal cursor pointer to next row in result set.
	 *
	 * \return True if moving cursor pointer to next row in result set was successful.
	 */
	bool next();

	/*! \brief Reset query object to its initial state and prepare it for another execution.
	 */
	void reset();

	/*! \brief Remove positional parameter bindings.
	 */
	void clearBindings();

	/*! \brief Execute prepared query.
	 *
	 */
	void exec();

	/*! \brief Return column names of executed query
	 * @return Vector of strings with column names
	 * @throws StorageException on error
	 */
	std::vector<std::string> columnNames() const;

private:
	friend class SQLDatabase;
	SQLQueryPrivate * d;
};

/*! \brief Provides access to sql database.
 *
 * Always use SQLDatabase through shared_ptr<SQLDatabase>.
 * Do not use it directly as value on the stack!
 */
class SQLDatabase {
public:
	~SQLDatabase();

	/*! \brief Create new instance of SQLDatabase object
	 *
	 * @return Shared pointer of database instance
	 */
	static std::shared_ptr<SQLDatabase> newInstance();

	/*! \brief Open/create sqlite database file.
	 *
	 * \param path - path to sqlite database file to open/create.
	 */
	void open(const std::string& path);

	/*! \brief Clone opened sqlite database file.
	 *
	 * \pre Database must be opened.
	 */
	void close();

	/*! \brief Execute SQL on database.
	 *
	 * \param query - query to be executed on database.
	 * \return SQLQuery object representing query result.
	 *
	 * \pre Database must be opened.
	 */
	SQLQuery prepare(const std::string& query) const;

	/*! \brief Find out if particular table exists in database.
	 *
	 * \param name - name of the table to check for existence.
	 * \return True if table of given name exists in database.
	 *
	 * \pre Database must be opened.
	 */
	bool tableExists(const std::string& name) const;

	/*! \brief Get id number generated by last INSERT statement on a table with auto-number column.
	 *
	 * \return id generated in auto-number column by last INSERT statement.
	 *
	 * \pre Database must be opened.
	 */
	long long lastInsertId() const;

	/*! \brief Execute SQL on database.
	 *
	 * Execute database query (do not use for SELECT)
	 *
	 * \param command - query to be executed on database.
	 * \return True if query has been executed successfully.
	 *
	 * \pre Database must be opened.
	 */
	bool exec(const std::string& command) const;

	/*! \brief Get names of a columns in particular table.
	 *
	 * \param table - name of a table to fetch column names for.
	 * \return Collection of names of columns in requested table.
	 *
	 * \pre Database must be opened.
	 */
	std::vector<std::string> tableColumnNames(const std::string& table) const;

	/*! \brief Start transaction on database.
	 *
	 *  \return True if transaction has been started successfully.
	 *
	 *  \pre Database must be opened.
     *  \throws StorageException on error
	 */
	void begin();

	/*! \brief Commit transaction on database.
	 *
	 *  \pre Database must be opened and transaction must be started.
     *  \throws StorageException on error
	 */
	void commit();

	/*! \brief Roll transaction back on database.
	 *
	 *  \pre Database must be opened and transaction must be started.
	 */
	void rollback();

	/*! \brief Create copy of database object to be used in another thread
	 *
	 */
	std::shared_ptr<SQLDatabase> cloneForThread(void);

protected:
	SQLDatabase();

	static std::shared_ptr<SQLDatabase> make_shared();

private:
	SQLDatabase(const SQLDatabase&);
	SQLDatabase& operator = (const SQLDatabase&);

private:
	SQLDatabasePrivate * d;
};

} /* namespace storage */
} /* namespace tizen_browser */
#endif /* SQLDATABASE_H_ */
