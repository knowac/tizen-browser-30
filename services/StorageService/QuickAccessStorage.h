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

#ifndef QUICKACCESSSTORAGE_H
#define QUICKACCESSSTORAGE_H

#include "QuickAccessItem.h"
#include "SQLDatabase.h"

namespace tizen_browser {
namespace storage {

class QuickAccessStorage
{
public:

    QuickAccessStorage();
    void init();
    void addQuickAccessItem(
        const std::string &url,
        const std::string &title,
        int color,
        int order,
        bool hasFavicon,
        tools::BrowserImagePtr favicon,
        int widht,
        int height);
    void deleteQuickAccessItem(unsigned int id);
    unsigned int getQuickAccessCount();
    bool quickAccessItemExist(const std::string &url);
    services::SharedQuickAccessItemVector getQuickAccessList();

private:

    void initDatabaseQuickAccess(const std::string &db_str);
    bool m_isInitialized;
    bool m_dbQuickAccessInitialized;
    std::string DB_QUICKACCESS;
};

}//end namespace storage
}//end namespace tizen_browser

#endif // QUICKACCESSSTORAGE_H
