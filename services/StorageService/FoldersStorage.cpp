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
 *
 *
 */

/*
 * FoldersStorage.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#include <boost/format.hpp>
#include <BrowserAssert.h>
#include <string>
#include "Config.h"
#include "FoldersStorage.h"
#include "Field.h"
#include "SQLTransactionScope.h"
#include "DBTools.h"
#include "SQLDatabase.h"
#include "SQLDatabaseImpl.h"
#include "DriverManager.h"
#include "StorageException.h"
#include "StorageExceptionInitialization.h"

namespace {
    const std::string TABLE_FOLDER = "FOLDER_TABLE";
    const std::string COL_FOLDER_ID = "folder_id";
    const std::string COL_FOLDER_NAME = "name";
    const std::string COL_FOLDER_NUMBER = "number";
    const std::string CONSTRAINT_TABLE_PK = TABLE_FOLDER + "_PK";
    const std::string DDL_CREATE_TABLE_FOLDER
                            = " CREATE TABLE " + TABLE_FOLDER
                            + " ( " + COL_FOLDER_ID + " INTEGER, "
                            + "   " + COL_FOLDER_NAME + " TEXT,"
                            + "   " + COL_FOLDER_NUMBER + " INTEGER,"
                            + " CONSTRAINT " + CONSTRAINT_TABLE_PK
                            + "      PRIMARY KEY ( " + COL_FOLDER_ID + " ) "
                            + "      ON CONFLICT REPLACE "
                            + " ); ";

}

namespace tizen_browser {
namespace storage {

FoldersStorage::FoldersStorage()
    : m_isInitialized(false)
    , m_dbFoldersInitialised(false)
{
    init();
}

FoldersStorage::~FoldersStorage()
{
}


void FoldersStorage::init()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_isInitialized)
        return;

    std::string resourceDbDir(boost::any_cast < std::string > (tizen_browser::config::Config::getInstance().get("resourcedb/dir")));
    std::string sessionDb(boost::any_cast < std::string > (tizen_browser::config::Config::getInstance().get("DB_FOLDERS")));

    DB_FOLDERS = resourceDbDir + sessionDb;

    BROWSER_LOGD("[%s:%d] DB_FOLDERS=%s", __PRETTY_FUNCTION__, __LINE__, DB_FOLDERS.c_str());

    try {
        initDatabaseFolders(DB_FOLDERS);
    } catch (storage::StorageExceptionInitialization & e) {
        BROWSER_LOGE("[%s:%d] Cannot initialize database %s!", __PRETTY_FUNCTION__, __LINE__, DB_FOLDERS.c_str());
    }

    m_isInitialized = true;
}

void FoldersStorage::initDatabaseFolders(const std::string& db_str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!m_dbFoldersInitialised) {
        try {
            dbtools::checkAndCreateTable(db_str, TABLE_FOLDER, DDL_CREATE_TABLE_FOLDER);
            const std::string all = "All";
            if (ifFolderExists(all))
                AllFolder = getFolderId(all);
            else
                AllFolder = addFolder(all);

#if PROFILE_MOBILE
            const std::string special = "Mobile"; //TODO: missing translation
#else
            const std::string special = "Bookmark Bar"; //TODO: missing translation
#endif
            if (ifFolderExists(special))
                SpecialFolder = getFolderId(special);
            else
                SpecialFolder = addFolder(special);
        } catch (storage::StorageException & e) {
            throw storage::StorageExceptionInitialization(e.getMessage(),
                                                          e.getErrorCode());
        }

        m_dbFoldersInitialised = true;
    }

    M_ASSERT(m_dbFoldersInitialised);
}

services::SharedBookmarkFolder FoldersStorage::getFolder(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string name = getFolderName(id);
    unsigned int count = getFolderNumber(id);
    services::SharedBookmarkFolder folder;
    if (name != "")
        folder = std::make_shared<services::BookmarkFolder>(id, name, count);
    return folder;
}

services::SharedBookmarkFolderList FoldersStorage::getFolders()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    services::SharedBookmarkFolderList folders;
    int foldersCount = getFoldersCount();
    if (foldersCount != 0) {
        boost::format getFoldersString("SELECT %1%, %2%, %3% FROM %4% ;");
        getFoldersString % COL_FOLDER_ID % COL_FOLDER_NAME % COL_FOLDER_NUMBER % TABLE_FOLDER;
        try {
            storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
            std::shared_ptr<storage::SQLDatabase> connection = scope.database();
            storage::SQLQuery getFoldersQuery(connection->prepare(getFoldersString.str()));
            getFoldersQuery.exec();
            for (int i = 0; i < foldersCount; ++i) {
                services::SharedBookmarkFolder bookmark = std::make_shared<services::BookmarkFolder>(
                            getFoldersQuery.getInt(0), getFoldersQuery.getString(1), getFoldersQuery.getInt(2));
                folders.push_back(bookmark);
                getFoldersQuery.next();
            }
        } catch (storage::StorageException& e) {
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        }
    }
    return folders;
}

unsigned int FoldersStorage::getFoldersCount()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getCountString("SELECT COUNT (*) FROM %1% ;");
    getCountString % TABLE_FOLDER;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getCountQuery(connection->prepare(getCountString.str()));
        getCountQuery.exec();
        return getCountQuery.getInt(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

unsigned int FoldersStorage::addFolder(const std::string& name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format addFolderQueryString("INSERT OR REPLACE INTO %1% ( %2% ) VALUES ( ? );");
    addFolderQueryString % TABLE_FOLDER % COL_FOLDER_NAME;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery addFolderQuery(db->prepare(addFolderQueryString.str()));
        addFolderQuery.bindText(1, name);
        addFolderQuery.exec();
        return db->lastInsertId();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

void FoldersStorage::updateFolderName(unsigned int id, const std::string& newName)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (id == AllFolder)
        return;
    boost::format updateFolderNameString("UPDATE %1%  SET %2% = ? WHERE %3% = ?" );
    updateFolderNameString % TABLE_FOLDER % COL_FOLDER_NAME % COL_FOLDER_ID;
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
    std::shared_ptr<storage::SQLDatabase> connection = scope.database();
    try {
        storage::SQLQuery updateFolderNameQuery(connection->prepare(updateFolderNameString.str()));
        updateFolderNameQuery.bindText(1, newName);
        updateFolderNameQuery.bindInt(2, id);
        updateFolderNameQuery.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void FoldersStorage::addNumberInFolder(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (id != AllFolder)
        addNumberInFolder(AllFolder);
    boost::format updateFolderNameString("UPDATE %1% SET %2% = ? WHERE %3% = ?" );
    updateFolderNameString % TABLE_FOLDER % COL_FOLDER_NUMBER % COL_FOLDER_ID;
    int count = getFolderNumber(id);
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
    std::shared_ptr<storage::SQLDatabase> connection = scope.database();
    try {
        storage::SQLQuery updateFolderNameQuery(connection->prepare(updateFolderNameString.str()));
        updateFolderNameQuery.bindInt(1, count+1);
        updateFolderNameQuery.bindInt(2, id);
        updateFolderNameQuery.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void FoldersStorage::removeNumberInFolder(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (id != AllFolder)
        removeNumberInFolder(AllFolder);
    boost::format updateFolderNameString("UPDATE %1% SET %2% = ? WHERE %3% = ?" );
    updateFolderNameString % TABLE_FOLDER % COL_FOLDER_NUMBER % COL_FOLDER_ID;
    int count = getFolderNumber(id);
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
    std::shared_ptr<storage::SQLDatabase> connection = scope.database();
    try {
        storage::SQLQuery updateFolderNameQuery(connection->prepare(updateFolderNameString.str()));
        updateFolderNameQuery.bindInt(1, count-1);
        updateFolderNameQuery.bindInt(2, id);
        updateFolderNameQuery.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void FoldersStorage::deleteAllFolders()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format deleteFoldersString("DELETE FROM %1% WHERE %2% != ? AND %3% != ? ;");
    deleteFoldersString % TABLE_FOLDER % COL_FOLDER_ID % COL_FOLDER_ID;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();

        storage::SQLQuery deleteFoldersQuery(connection->prepare(deleteFoldersString.str()));
        deleteFoldersQuery.bindInt(1, AllFolder);
        deleteFoldersQuery.bindInt(2, SpecialFolder);
        deleteFoldersQuery.exec();
    } catch( storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }

    boost::format updateFoldersCountString("UPDATE %1% SET %2% = ? WHERE %3% = ? OR %4% = ?");
    updateFoldersCountString % TABLE_FOLDER % COL_FOLDER_NUMBER % COL_FOLDER_ID % COL_FOLDER_ID;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();

        storage::SQLQuery updateFoldersCountQuery(connection->prepare(updateFoldersCountString.str()));
        updateFoldersCountQuery.bindInt(1, 0);
        updateFoldersCountQuery.bindInt(2, AllFolder);
        updateFoldersCountQuery.bindInt(3, SpecialFolder);
        updateFoldersCountQuery.exec();
    } catch( storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void FoldersStorage::deleteFolder(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (id == AllFolder || id == SpecialFolder)
        return;
    boost::format deleteFolderString("DELETE FROM %1% WHERE %2% = ?;");
    deleteFolderString % TABLE_FOLDER % COL_FOLDER_ID;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
    std::shared_ptr<storage::SQLDatabase> connection = scope.database();
    try {
        storage::SQLQuery deleteFolderQurey(connection->prepare(deleteFolderString.str()));
        deleteFolderQurey.bindInt(1, id);
        deleteFolderQurey.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

bool FoldersStorage::ifFolderExists(const std::string& name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getCountString("SELECT COUNT (*) FROM %1% WHERE %2% = ?;");
    getCountString % TABLE_FOLDER % COL_FOLDER_NAME;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getCountQuery(connection->prepare(getCountString.str()));
        getCountQuery.bindText(1, name);
        getCountQuery.exec();
        int number = getCountQuery.getInt(0);
        return number != 0;
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return true;
}

unsigned int FoldersStorage::getFolderId(const std::string& name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getIdString("SELECT %1% FROM %2% WHERE %3% = ?;");
    getIdString % COL_FOLDER_ID % TABLE_FOLDER % COL_FOLDER_NAME;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getIdQuery(connection->prepare(getIdString.str()));
        getIdQuery.bindText(1, name);
        getIdQuery.exec();
        return getIdQuery.getInt(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

std::string FoldersStorage::getFolderName(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getNameString("SELECT %1% FROM %2% WHERE %3% = ?;");
    getNameString % COL_FOLDER_NAME % TABLE_FOLDER % COL_FOLDER_ID;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getNameQuery(connection->prepare(getNameString.str()));
        getNameQuery.bindInt(1, id);
        getNameQuery.exec();

        return getNameQuery.getString(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return std::string();
}

unsigned int FoldersStorage::getFolderNumber(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getNameString("SELECT %1% FROM %2% WHERE %3% = ?;");
    getNameString % COL_FOLDER_NUMBER % TABLE_FOLDER % COL_FOLDER_ID;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_FOLDERS));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getNameQuery(connection->prepare(getNameString.str()));
        getNameQuery.bindInt(1, id);
        getNameQuery.exec();

        return getNameQuery.getInt(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

}//end namespace storage
}//end namespace tizen_browser
