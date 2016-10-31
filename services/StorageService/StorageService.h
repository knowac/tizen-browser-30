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

#ifndef STORAGESERVICE_H
#define STORAGESERVICE_H

#include "ServiceFactory.h"
#include "service_macros.h"
#include "SettingsStorage.h"
#include "FoldersStorage.h"
#include "CertificateStorage.h"
#include "QuickAccessStorage.h"

#define DOMAIN_STORAGE_SERVICE "org.tizen.browser.storageservice"

namespace tizen_browser {
namespace services {

class BROWSER_EXPORT StorageService : public tizen_browser::core::AbstractService
{
public:
    StorageService();
    virtual ~StorageService();
    virtual std::string getName();

    storage::SettingsStorage& getSettingsStorage() { return m_settingsStorage; }
    storage::FoldersStorage& getFoldersStorage() { return m_foldersStorage; }
    storage::CertificateStorage& getCertificateStorage() { return m_certificateStorage; }
    storage::QuickAccessStorage& getQuickAccessStorage() { return m_quickaccessStorage; }

private:
    storage::SettingsStorage m_settingsStorage;
    storage::FoldersStorage m_foldersStorage;
    storage::CertificateStorage m_certificateStorage;
    storage::QuickAccessStorage m_quickaccessStorage;
};



}
}

#endif // STORAGESERVICE_H
