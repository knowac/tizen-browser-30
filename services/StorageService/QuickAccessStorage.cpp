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
#include "BrowserImage.h"


#include "QuickAccessStorage.h"

namespace {
// ------ Database QUICKACCESS ------
const std::string TABLE_QUICKACCESS = "QUICKACCESS";
const std::string COL_ID = "ID";
const std::string COL_URL = "URL";
const std::string COL_TITLE = "TITLE";
const std::string COL_COLOR = "COLOR";
const std::string COL_ORDER = "QA_ORDER";
const std::string COL_HAS_FAVICON = "HAS_FAVICON";
const std::string COL_FAVICON = "FAVICON";
const std::string COL_WIDTH = "WIDTH";
const std::string COL_HEIGHT = "HEIGHT";

const std::string CREATE_TABLE_QUICKACCESS
        = "CREATE TABLE " + TABLE_QUICKACCESS
        +   " ( " + COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, "
        +   COL_URL + " TEXT UNIQUE, "
        +   COL_TITLE + " TEXT, "
        +   COL_COLOR + " INTEGER, "
        +   COL_ORDER + " INTEGER NOT NULL, "
        +   COL_HAS_FAVICON + " INTEGER, "
        +   COL_FAVICON + " BLOB, "
        +   COL_WIDTH + " INTEGER, "
        +   COL_HEIGHT + " INTEGER "
        + " );";
// ------ (end) Database QUICKACCESS ------
}

namespace tizen_browser {
namespace storage {

QuickAccessStorage::QuickAccessStorage()
    : m_isInitialized(false)
    , m_dbQuickAccessInitialized(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    init();
}

void QuickAccessStorage::init()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_isInitialized)
        return;

    std::string resourceDbDir(boost::any_cast<std::string> (config::Config::getInstance().get("resourcedb/dir")));
    std::string quickaccessDb(boost::any_cast<std::string> (config::Config::getInstance().get("DB_QUICKACCESS")));
    DB_QUICKACCESS = resourceDbDir + quickaccessDb;
    BROWSER_LOGD("[%s:%d] DB_QUICKACCESS=%s", __PRETTY_FUNCTION__, __LINE__, DB_QUICKACCESS.c_str());
    try {
        initDatabaseQuickAccess(DB_QUICKACCESS);
    } catch (storage::StorageExceptionInitialization & e) {
        BROWSER_LOGE("[%s:%d] Cannot initialize database %s!", __PRETTY_FUNCTION__, __LINE__, DB_QUICKACCESS.c_str());
    }
    m_isInitialized = true;
}

void QuickAccessStorage::addQuickAccessItem(
    const std::string &url,
    const std::string &title,
    int color,
    int order,
    bool hasFavicon,
    tools::BrowserImagePtr favicon,
    int width,
    int height)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int hasFaviconInt = hasFavicon ? 1 : 0; // Convert to int bacause of SQLite doesn't have bool type.
    boost::format addQuickAccessItemQueryString(
        "INSERT OR REPLACE INTO %1% (%2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);");
    addQuickAccessItemQueryString % TABLE_QUICKACCESS % COL_URL % COL_TITLE % COL_COLOR % COL_ORDER %
        COL_HAS_FAVICON % COL_FAVICON % COL_WIDTH % COL_HEIGHT;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_QUICKACCESS));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery addQuickAccessItemQuery(db->prepare(addQuickAccessItemQueryString.str()));
        addQuickAccessItemQuery.bindText(1, url);
        addQuickAccessItemQuery.bindText(2, title);
        addQuickAccessItemQuery.bindInt(3, color);
        addQuickAccessItemQuery.bindInt(4, order);
        addQuickAccessItemQuery.bindInt(5, hasFaviconInt);
        if (hasFavicon)
            addQuickAccessItemQuery.bindBlob(6, tools::EflTools::getBlobPNG(favicon));
        addQuickAccessItemQuery.bindInt(7, width);
        addQuickAccessItemQuery.bindInt(8, height);
        addQuickAccessItemQuery.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void QuickAccessStorage::deleteQuickAccessItem(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format deleteQuickAccessItemQueryString("DELETE FROM %1% WHERE %2% = ?;");
    deleteQuickAccessItemQueryString % TABLE_QUICKACCESS % COL_ID;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_QUICKACCESS));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery deleteQuickAccessItemQuery(db->prepare(deleteQuickAccessItemQueryString.str()));
        deleteQuickAccessItemQuery.bindInt(1, id);
        deleteQuickAccessItemQuery.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

unsigned int QuickAccessStorage::getQuickAccessCount()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getCountString("SELECT COUNT (*) FROM " + TABLE_QUICKACCESS + " ;");
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_QUICKACCESS));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery getCountQuery(db->prepare(getCountString.str()));
        getCountQuery.exec();
        return getCountQuery.getInt(0);
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

bool QuickAccessStorage::quickAccessItemExist(const std::string &url)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format isItemExistString("SELECT COUNT(*) FROM %1% WHERE %2% = ?;");
    isItemExistString % TABLE_QUICKACCESS % COL_URL;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_QUICKACCESS));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery isItemExistQuery(db->prepare(isItemExistString.str()));
        isItemExistQuery.bindText(1, url);
        isItemExistQuery.exec();
        return static_cast<bool>(isItemExistQuery.getInt(0));
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return false;
}

services::SharedQuickAccessItemVector QuickAccessStorage::getQuickAccessList()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    services::SharedQuickAccessItemVector QAList;
    int QACount = getQuickAccessCount();
    if (QACount > 0) {
        boost::format getQuickAccessListString("SELECT %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9% FROM %10% ;");
        getQuickAccessListString % COL_ID % COL_URL % COL_TITLE % COL_COLOR % COL_ORDER %
            COL_HAS_FAVICON % COL_FAVICON % COL_WIDTH % COL_HEIGHT % TABLE_QUICKACCESS;
        try {
            storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_QUICKACCESS));
            std::shared_ptr<storage::SQLDatabase> db = scope.database();
            storage::SQLQuery getQuickAccesListQuery(db->prepare(getQuickAccessListString.str()));
            getQuickAccesListQuery.exec();

            for (int i = 0; i < QACount; i++) {
                services::SharedQuickAccessItem QuickAccesItem = std::make_shared<services::QuickAccessItem>(
                    getQuickAccesListQuery.getInt(0),
                    getQuickAccesListQuery.getString(1),
                    getQuickAccesListQuery.getString(2),
                    getQuickAccesListQuery.getInt(3),
                    getQuickAccesListQuery.getInt(4),
                    static_cast<bool>(getQuickAccesListQuery.getInt(5)));

                if (static_cast<bool>(getQuickAccesListQuery.getInt(5))) {
                    tools::BrowserImagePtr favicon = std::make_shared<tools::BrowserImage>(
                        getQuickAccesListQuery.getInt(7),
                        getQuickAccesListQuery.getInt(8),
                        getQuickAccesListQuery.getBlob(6)->getLength());
                    favicon->setData(const_cast<void *>(getQuickAccesListQuery.getBlob(6)->getData()),
                        false, tools::ImageType::ImageTypePNG);
                    QuickAccesItem->setFavicon(favicon);
                }

                QAList.push_back(QuickAccesItem);
                getQuickAccesListQuery.next();
            }
        } catch (storage::StorageException& e){
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        }
    }
    return QAList;
}

void QuickAccessStorage::initDatabaseQuickAccess(const std::string &db_str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_dbQuickAccessInitialized) {
        try {
            dbtools::checkAndCreateTable(db_str, TABLE_QUICKACCESS, CREATE_TABLE_QUICKACCESS);
        } catch (storage::StorageException &e) {
            throw storage::StorageExceptionInitialization(e.getMessage(), e.getErrorCode());
        }

        m_dbQuickAccessInitialized = true;
    }

    M_ASSERT(m_dbQuickAccessInitialized);
}


}//end namespace storage
}//end namespace tizen_browser

