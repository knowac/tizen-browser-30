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
 * FoldersStorage.h
 *
 *  Created on: Dec 14, 2015
 *      Author: m.kawonczyk@samsung.com
 */


#ifndef FOLDERSSTORAGE_H
#define FOLDERSSTORAGE_H

#include <memory>
#include <boost/signals2/signal.hpp>

#include "SQLDatabase.h"
#include "WebEngineSettings.h"
#include "BookmarkFolder.h"
#include "app_i18n.h"

namespace tizen_browser {
namespace storage {


class  FoldersStorage
{
public:
    /**
     * Singleton, protect against being created in wrong place.
     */
    FoldersStorage();

    /**
     * Initialise FoldersStorage.
     *
     * Checks if all needed tables are created and if not creates them.
     *
     * Returns true if there is no error.
     */
    void init();

    unsigned int AllFolder;
    unsigned int SpecialFolder;

    ~FoldersStorage();

    /**
     * Returns created custom folder.
     */
    services::SharedBookmarkFolder getFolder(unsigned int id);

    /**
     * Returns all created custom folders.
     */
    services::SharedBookmarkFolderList getFolders();

    /**
     * Returns all created custom folders.
     */
    unsigned int getFoldersCount();

    /**
     * Adds folder with a specified name.
     * Returns created folder id.
     */
    unsigned int addFolder(const std::string& name);

    /**
     * Updates folder with a specified name.
     */
    void updateFolderName(unsigned int id, const std::string& newName);

    /**
     * Add additional bookmark to a folder.
     */
    void addNumberInFolder(unsigned int id);

    /**
     * Remove additional bookmark to a folder.
     */
    void removeNumberInFolder(unsigned int id);

    /**
     * Delete all folders.
     */
    void deleteAllFolders();

    /**
     * Delete folder with a specified id.
     */
    void deleteFolder(unsigned int id);

    /**
     * Answers if folder of a specified name exists.
     */
    bool ifFolderExists(const std::string& name);
    /**
     * Get id of a folder.
     */
    unsigned int getFolderId(const std::string &name);

    /**
     * Get name of a folder.
     */
    std::string getFolderName(unsigned int id);

    /**
     * Get number of bookmarks in a folder.
     */
    unsigned int getFolderNumber(unsigned int id);

private:
    void initDatabaseFolders(const std::string& db_str);
    bool m_isInitialized;
    bool m_dbFoldersInitialised;

    std::string DB_FOLDERS;
};

}//end namespace storage
}//end namespace tizen_browser

#endif // FOLDERSSTORAGE_H
