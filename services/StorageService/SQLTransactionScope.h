/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef SQLTRANSACTIONSCOPE_H_
#define SQLTRANSACTIONSCOPE_H_

#include <boost/noncopyable.hpp>
#include <memory>


namespace tizen_browser {
namespace storage {

class SQLDatabase;

/*! \brief Holds SQL transaction.
 *
 * On exception rolls the transaction back. On normal destruction
 * commits the transaction
 */
class SQLTransactionScope : boost::noncopyable
{
    std::shared_ptr<SQLDatabase> _db;
    bool _inTransaction;
public:
    SQLTransactionScope(std::shared_ptr<SQLDatabase> db);
    ~SQLTransactionScope();
    void commit();
    void rollback();

    inline std::shared_ptr<SQLDatabase> database() const { return _db; }
};


}
}

#endif // SQLTRANSACTIONSCOPE_H_
