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
 *
 *
 */

#include <boost/format.hpp>
#include <BrowserAssert.h>
#include <string>
#include "Config.h"
#include "CertificateStorage.h"
#include "Field.h"
#include "SQLTransactionScope.h"
#include "DBTools.h"
#include "SQLDatabase.h"
#include "SQLDatabaseImpl.h"
#include "DriverManager.h"
#include "StorageException.h"
#include "StorageExceptionInitialization.h"

namespace {
    const std::string TABLE_CERTIFICATE = "CERTIFICATE_TABLE";
    const std::string COL_PEM = "pem";
    const std::string COL_HOST = "host";
    const std::string COL_ALLOW = "allow";
    const std::string CONSTRAINT_TABLE_PK = TABLE_CERTIFICATE + "_PK";
    const std::string DDL_CREATE_TABLE_CERTIFICATE
                            = " CREATE TABLE " + TABLE_CERTIFICATE
                            + " ( " + COL_HOST + " TEXT,"
                            + "   " + COL_PEM + " TEXT,"
                            + "   " + COL_ALLOW + " INTEGER,"
                            + " CONSTRAINT " + CONSTRAINT_TABLE_PK
                            + "      PRIMARY KEY ( " + COL_HOST + " ) "
                            + "      ON CONFLICT REPLACE "
                            + " ); ";

}

namespace tizen_browser {
namespace storage {

CertificateStorage::CertificateStorage()
    : m_isInitialized(false)
    , m_dbCertificateInitialised(false)
{
    init();
}

CertificateStorage::~CertificateStorage()
{
}


void CertificateStorage::init()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_isInitialized)
        return;

    std::string resourceDbDir(boost::any_cast <std::string> (tizen_browser::config::Config::getInstance().get("resourcedb/dir")));
    std::string sessionDb(boost::any_cast <std::string> (tizen_browser::config::Config::getInstance().get("DB_CERTIFICATE")));

    DB_CERTIFICATE = resourceDbDir + sessionDb;

    BROWSER_LOGD("[%s:%d] DB_CERTIFICATE=%s", __PRETTY_FUNCTION__, __LINE__, DB_CERTIFICATE.c_str());

    try {
        initDatabaseCertificate(DB_CERTIFICATE);
    } catch (storage::StorageExceptionInitialization & e) {
        BROWSER_LOGE("[%s:%d] Cannot initialize database %s!", __PRETTY_FUNCTION__, __LINE__, DB_CERTIFICATE.c_str());
    }

    m_isInitialized = true;
}

void CertificateStorage::initDatabaseCertificate(const std::string& db_str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!m_dbCertificateInitialised) {
        try {
            dbtools::checkAndCreateTable(db_str, TABLE_CERTIFICATE, DDL_CREATE_TABLE_CERTIFICATE);
        } catch (storage::StorageException & e) {
            throw storage::StorageExceptionInitialization(e.getMessage(),
                                                          e.getErrorCode());
        }

        m_dbCertificateInitialised = true;
    }

    M_ASSERT(m_dbCertificateInitialised);
}

std::shared_ptr<HostCertList> CertificateStorage::getHostCertList()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto hcList = std::make_shared<HostCertList>();
    int itemsCount = getCertificateEntriesCount();
    BROWSER_LOGD("Items count = %d", itemsCount);
    if (itemsCount != 0) {
        boost::format getCertificateString("SELECT %1%, %2% FROM %3% ;");
        getCertificateString % COL_HOST % COL_ALLOW % TABLE_CERTIFICATE;
        try {
            storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_CERTIFICATE));
            std::shared_ptr<storage::SQLDatabase> connection = scope.database();
            storage::SQLQuery getCertificateQuery(connection->prepare(getCertificateString.str()));
            getCertificateQuery.exec();
            for (int i = 0; i < itemsCount; ++i) {
                std::pair<std::string, int> hostCert = std::make_pair<std::string, int>(
                            getCertificateQuery.getString(0), getCertificateQuery.getInt(1));
                hcList->push_back(hostCert);
                getCertificateQuery.next();
            }
        } catch (storage::StorageException& e) {
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        }
    }
    return hcList;
}

unsigned int CertificateStorage::getCertificateEntriesCount()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getCountString("SELECT COUNT (*) FROM %1% ;");
    getCountString % TABLE_CERTIFICATE;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_CERTIFICATE));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getCountQuery(connection->prepare(getCountString.str()));
        getCountQuery.exec();
        return getCountQuery.getInt(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

unsigned int CertificateStorage::addOrUpdateCertificateEntry(const std::string& pem, const std::string& host, int allow)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format addCertificateQueryString("REPLACE INTO %1% ( %2%, %3%, %4% ) VALUES ( ?, ?, ? );");
    addCertificateQueryString % TABLE_CERTIFICATE % COL_HOST % COL_PEM % COL_ALLOW;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_CERTIFICATE));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery addCertificateQuery(db->prepare(addCertificateQueryString.str()));
        addCertificateQuery.bindText(1, host);
        addCertificateQuery.bindText(2, pem);
        addCertificateQuery.bindInt(3, allow);
        addCertificateQuery.exec();
        return db->lastInsertId();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

std::string CertificateStorage::getPemForURI(const std::string& uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getPemString("SELECT %1% FROM %2% WHERE %3% = ?;");
    getPemString % COL_PEM % TABLE_CERTIFICATE % COL_HOST;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_CERTIFICATE));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getPemQuery(connection->prepare(getPemString.str()));
        getPemQuery.bindText(1, uri);
        getPemQuery.exec();

        return getPemQuery.getString(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return std::string();
}

void CertificateStorage::deleteAllEntries()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format deleteCertificateString("DELETE FROM %1%;");
    deleteCertificateString % TABLE_CERTIFICATE;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_CERTIFICATE));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();

        storage::SQLQuery deleteCertificatesQuery(connection->prepare(deleteCertificateString.str()));
        deleteCertificatesQuery.exec();
    } catch( storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

}//end namespace storage
}//end namespace tizen_browser
