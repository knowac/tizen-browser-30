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

#include "SQLTransactionScope.h"
#include "SQLDatabase.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"

namespace tizen_browser {
namespace storage {

SQLTransactionScope::SQLTransactionScope(std::shared_ptr<SQLDatabase> db) :
    _db(db),
    _inTransaction(false)
{
    M_ASSERT(db);

    db->begin();
    _inTransaction = true;
}

SQLTransactionScope::~SQLTransactionScope()
{
    if(std::uncaught_exception()) {
        if(_inTransaction)
            _db->rollback();
    } else {
        if(_inTransaction)
            _db->commit();
    }
}

void SQLTransactionScope::commit()
{
    M_ASSERT(_inTransaction);
    _inTransaction = false;
    _db->commit();
}

void SQLTransactionScope::rollback()
{
    _inTransaction = false;
    _db->rollback();
}


}
}
