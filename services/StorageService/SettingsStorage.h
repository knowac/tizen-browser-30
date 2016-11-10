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

#ifndef __STORAGESERVICE_H
#define __STORAGESERVICE_H

#include <memory>
#include <boost/signals2/signal.hpp>

#include "SQLDatabase.h"
#include "WebEngineSettings.h"


namespace tizen_browser {
namespace storage {

class SettingsStorage
{
public:
    SettingsStorage();
    virtual ~SettingsStorage();

    void resetSettings();
    void setParam(basic_webengine::WebEngineSettings param, bool value) const;
    void setParamString(basic_webengine::WebEngineSettings param, std::string value) const;
    bool getParamVal(basic_webengine::WebEngineSettings param) const;
    bool isDBParamPresent(const std::string& key) const;
    /**
     * @throws StorageException on error
     */
    std::string getParamString(basic_webengine::WebEngineSettings param) const;
    /**
     * @throws StorageException on error
     */
    int getSettingsInt(const std::string & key, const int defaultValue) const;

    /**
     * @throws StorageException on error
     */
    double getSettingsDouble(const std::string & key, const double defaultValue) const;

    /**
     * @throws StorageException on error
     */
    const std::string getSettingsText(const std::string & key, const std::string & defaultValue) const;

    /**
     * @throws StorageException on error
     */
    bool getSettingsBool(const std::string & key, const bool defaultValue) const;

    /**
     * @throws StorageException on error
     */
    void setSettingsInt(const std::string & key, int value) const;

    /**
     * @throws StorageException on error
     */
    void setSettingsDouble(const std::string & key, double value) const;

    /**
     * @throws StorageException on error
     */
    void setSettingsString(const std::string & key, std::string value) const;

    /**
     * @throws StorageException on error
     */
    void setSettingsBool(const std::string & key, bool value) const;

    void init(bool testmode = false);

    void initWebEngineSettingsFromDB();
    boost::signals2::signal<void (basic_webengine::WebEngineSettings, bool)> setWebEngineSettingsParam;
private:
    /**
     * @throws StorageExceptionInitialization on error
     */
    void initDatabaseSettings(const std::string & db_str);

    /**
     * @throws StorageExceptionInitialization on error
     */
    void setSettingsValue(const std::string & key, storage::FieldPtr field) const;

    bool isParamPresent(basic_webengine::WebEngineSettings param) const;

    bool m_dbSettingsInitialised;
    std::string DB_SETTINGS;

    bool m_isInitialized;
};

}
}

#endif
