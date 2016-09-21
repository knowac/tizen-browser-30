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

#ifndef CERTIFICATESTORAGE_H
#define CERTIFICATESTORAGE_H

#include <memory>
#include <boost/signals2/signal.hpp>

#include "SQLDatabase.h"
#include "WebEngineSettings.h"
#include "app_i18n.h"

namespace tizen_browser {
namespace storage {

typedef std::vector<std::pair<std::string, int>> HostCertList;

class  CertificateStorage
{
public:
    /**
     * Singleton, protect against being created in wrong place.
     */
    CertificateStorage();

    /**
     * Initialise CertificateStorage.
     *
     * Checks if all needed tables are created and if not creates them.
     *
     * Returns true if there is no error.
     */
    void init();

    ~CertificateStorage();

    /**
     * Get all host, allow pair from db.
     */
    std::shared_ptr<HostCertList> getHostCertList();

    /**
     * Get number of certificate entries in db.
     */
    unsigned int getCertificateEntriesCount();

    /**
     * Add new certificate data into db.
     */
    unsigned int addOrUpdateCertificateEntry(const std::string& pem, const std::string& host, int allow);

    /**
     * Get certificate pem data for uri.
     */
    std::string getPemForURI(const std::string& uri);

    /**
     * Delete all entries in cert db.
     */
    void deleteAllEntries();

private:
    void initDatabaseCertificate(const std::string& db_str);
    bool m_isInitialized;
    bool m_dbCertificateInitialised;

    std::string DB_CERTIFICATE;
};

}//end namespace storage
}//end namespace tizen_browser

#endif // CERTIFICATESTORAGE_H
