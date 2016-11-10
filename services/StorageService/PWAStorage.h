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

#ifndef PWASTORAGE_H
#define PWASTORAGE_H

#include "SQLDatabase.h"

namespace tizen_browser {
namespace storage {

class PWAStorage
{
public:

    PWAStorage();
    void init();
    void addPWAItem(const std::string & url, const int & exist, const int & never);
    void deletePWAItems();
    unsigned int getPWACount();
    int getPWACheck(const std::string & url);

private:

    void initDatabasePWA(const std::string & db_str);
    bool m_isInitialized;
    bool m_dbPWAInitialized;
    std::string DB_PWA;
};

}//end namespace storage
}//end namespace tizen_browser

#endif // PWASTORAGE_H
