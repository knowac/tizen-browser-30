/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <string>
#include <BrowserAssert.h>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include "EflTools.h"

#include "Field.h"
#include "BrowserLogger.h"
#include "DriverManager.h"
#include "DBTools.h"
#include "Config.h"
#include "StorageExceptionInitialization.h"
#include "SQLTransactionScope.h"

#include "PWAStorage.h"

namespace {
// ------ Database PWA ------
const std::string TABLE_PWA = "PWA";
const std::string COL_ID = "ID";
const std::string COL_URL = "URL";
const std::string COL_EXIST = "EXIST";
const std::string COL_NEVER = "NEVER";

const std::string CREATE_TABLE_PWA
        = "CREATE TABLE " + TABLE_PWA
        +   " ( " + COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, "
        +   COL_URL + " TEXT UNIQUE, "
        +   COL_EXIST + " INTEGER, "
        +   COL_NEVER + " INTEGER "
        + " );";
// ------ (end) Database PWA ------
}

namespace tizen_browser {
namespace storage {

PWAStorage::PWAStorage()
    : m_isInitialized(false)
    , m_dbPWAInitialized(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    init();
}

void PWAStorage::init()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_isInitialized)
        return;

    std::string resourceDbDir(boost::any_cast<std::string>(config::Config::getInstance().get("resourcedb/dir")));
    std::string pwaDb(boost::any_cast<std::string>(config::Config::getInstance().get("DB_PWA")));
    DB_PWA = resourceDbDir + pwaDb;
    BROWSER_LOGD("[%s:%d] DB_PWA=%s", __PRETTY_FUNCTION__, __LINE__, DB_PWA.c_str());
    try {
        initDatabasePWA(DB_PWA);
    } catch (StorageExceptionInitialization & e) {
        BROWSER_LOGE("[%s:%d] Cannot initialize database %s!", __PRETTY_FUNCTION__, __LINE__, DB_PWA.c_str());
    }
    m_isInitialized = true;
}

void PWAStorage::addPWAItem(const std::string & url, const int & exist, const int & never)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format addPWAItemQueryString("INSERT OR REPLACE INTO %1% (%2%, %3%, %4%) VALUES (?, ?, ?);");
    addPWAItemQueryString % TABLE_PWA % COL_URL % COL_EXIST % COL_NEVER;
    try {
        SQLTransactionScope scope(DriverManager::getDatabase(DB_PWA));
        std::shared_ptr<SQLDatabase> db = scope.database();
        SQLQuery addPWAItemQuery(db->prepare(addPWAItemQueryString.str()));
        addPWAItemQuery.bindText(1, url);
        addPWAItemQuery.bindInt(2, exist);
        addPWAItemQuery.bindInt(3, never);
        addPWAItemQuery.exec();
    } catch (const StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void PWAStorage::deletePWAItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format deletePWAItemQueryString("DELETE FROM %1% ;");
    deletePWAItemQueryString % TABLE_PWA;
    try {
        SQLTransactionScope scope(DriverManager::getDatabase(DB_PWA));
        std::shared_ptr<SQLDatabase> db = scope.database();
        SQLQuery deletePWAItemQuery(db->prepare(deletePWAItemQueryString.str()));
        deletePWAItemQuery.exec();
    } catch (const StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

unsigned int PWAStorage::getPWACount()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getCountString("SELECT COUNT (*) FROM " + TABLE_PWA + " ;");
    try {
        SQLTransactionScope scope(DriverManager::getDatabase(DB_PWA));
        std::shared_ptr<SQLDatabase> db = scope.database();
        SQLQuery getCountQuery(db->prepare(getCountString.str()));
        getCountQuery.exec();
        return getCountQuery.getInt(0);
    } catch (const StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

int PWAStorage::getPWACheck(const std::string & url)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int pwaCount = getPWACount();

    if (pwaCount) {
        boost::format getPWAListString("SELECT %1%, %2%, %3% FROM %4% ;");
        getPWAListString % COL_URL % COL_EXIST % COL_NEVER % TABLE_PWA;
        try {
            SQLTransactionScope scope(DriverManager::getDatabase(DB_PWA));
            std::shared_ptr<SQLDatabase> db = scope.database();
            SQLQuery getPWAListQuery(db->prepare(getPWAListString.str()));
            getPWAListQuery.exec();

            for (int i = 0; i < pwaCount; ++i) {
                if (!strcmp(url.c_str(), getPWAListQuery.getString(0).c_str())) {
                    if (getPWAListQuery.getInt(2))
                        return getPWAListQuery.getInt(2);
                    else
                        return getPWAListQuery.getInt(1);
                }
                getPWAListQuery.next();
            }
        } catch (const StorageException& e) {
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        }
    }
    BROWSER_LOGD("[%s:%d] pwaCount is 0 or string not equal !", __PRETTY_FUNCTION__, __LINE__);
    return 0;
}

void PWAStorage::initDatabasePWA(const std::string &db_str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_dbPWAInitialized) {
        try {
            dbtools::checkAndCreateTable(db_str, TABLE_PWA, CREATE_TABLE_PWA);
        } catch (const StorageException& e) {
            throw StorageExceptionInitialization(e.getMessage(), e.getErrorCode());
        }
        m_dbPWAInitialized = true;
    }
}

}//end namespace storage
}//end namespace tizen_browser
