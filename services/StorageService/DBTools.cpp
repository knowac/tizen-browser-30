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

#include "DBTools.h"
#include "DriverManager.h"
#include "BrowserLogger.h"
#include "SQLTransactionScope.h"

namespace tizen_browser {
namespace dbtools {


void checkAndCreateTable(const std::string& db_str, const std::string& tablename, const std::string& ddl)
{
    auto conCheckExistance = storage::DriverManager::getDatabase(db_str);

    bool isTabPresent = conCheckExistance->tableExists(tablename);
    if (!isTabPresent) {
        BROWSER_LOGI("A table %s can't be found. It will be recreated.", tablename.c_str());

        conCheckExistance->exec(ddl);
    }
}

void checkAndCreateTable(storage::SQLTransactionScope& transactionScope, const std::string& tablename, const std::string& ddl)
{
    auto conCheckExistance = transactionScope.database();

    bool isTabPresent = conCheckExistance->tableExists(tablename);

    if (!isTabPresent) {
        BROWSER_LOGI("A table %s can't be found. It will be recreated.", tablename.c_str());

        conCheckExistance->exec(ddl);
    }
}


}
}