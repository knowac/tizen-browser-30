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

/*
 *
 * Created on: Apr, 2014
 *     Author: m.kielak
 */

#ifndef ABSTRACTFAVORITESERVICE_H
#define ABSTRACTFAVORITESERVICE_H

#include "browser_config.h"

#include "AbstractService.h"
#include "service_macros.h"
#include "BookmarkItem.h"
#include "BrowserImage.h"
#include "StorageService.h"

namespace tizen_browser{
namespace interfaces{

class AbstractFavoriteService
        : public tizen_browser::core::AbstractService
{
public:

    /**
     * @brief Add page to bookmarks
     *
     * @param address Webpage url.
     * @param title Title of bookmark.
     * @param note Bookmark note, default is empty .
     * @param dirId Directory numeric ID, default is 0.
     * @param thumbnail Page thumbnail, default is empty image.
     * @param favicon Page favicon image, default is empty image.
     *
     * @return BookmarkItem class
     */
    virtual std::shared_ptr<tizen_browser::services::BookmarkItem> addBookmark(const std::string & address,
        const std::string & title,
        const std::string & note = std::string(),
        std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail=std::shared_ptr<tizen_browser::tools::BrowserImage>(),
        std::shared_ptr<tizen_browser::tools::BrowserImage> favicon = std::shared_ptr<tizen_browser::tools::BrowserImage>(),
        unsigned int dirId = 0) = 0;

    /**
     * @brief Update bookmark snapshot by given url
     *
     * @param url of bookmark to delete
     */
    virtual void updateBookmarkItemSnapshot(const std::string & url,
        std::shared_ptr<tizen_browser::tools::BrowserImage> snapshot) = 0;

    /** \todo Need to change this callback function for finding stored bookmark, check getBookmarkId function
     * @brief Check if bookmark exists
     *
     * @param url url to find
     * @return true if exists, false if not
     */
     virtual bool bookmarkExists(const std::string & url)= 0;

    /**
     * @brief Get bookmarks in a folder from platform service and store it in private m_bookmarksList
     *
     * @return list of bookmark items in folder with id, folder_id
     */
    virtual std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>> getBookmarks(int folder_id = -1) = 0;

    /**
     * @brief Delete all bookmarks
     *
     * @return true if success, false on error
     */
    virtual bool deleteAllBookmarks() = 0;

    /**
      * @brief Edit bookmark title and folder by given url
      *
      * @return true if success, false on error
      */
    virtual bool editBookmark(const std::string & url, const std::string & title, unsigned int folder_id = 0) = 0;

    /**
     * @brief Delete bookmark by given url
     *
     * @param url of bookmark to delete
     * @return true if success, false on error of not found bookmark
     */
    virtual bool deleteBookmark(const std::string & url) = 0;

    virtual std::shared_ptr<tizen_browser::services::BookmarkItem> addFolder(const std::string & title,
        int parent = tizen_browser::services::ROOT_FOLDER_ID) = 0;

    virtual std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>> getFolders(
        int parent = tizen_browser::services::ROOT_FOLDER_ID) = 0;
    virtual bool folderExists(const std::string & title, int parent = tizen_browser::services::ROOT_FOLDER_ID) = 0;
    virtual std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>> getAllBookmarkItems(
        int parent = tizen_browser::services::ROOT_FOLDER_ID) = 0;
    virtual std::shared_ptr<tizen_browser::services::BookmarkItem> getRoot() = 0;
    virtual std::shared_ptr<tizen_browser::services::BookmarkItem> getBookmarkItem(int id) = 0;
    virtual void editBookmark(int id, const std::string & url, const std::string & title, int parent = -1,
        int order = -1) = 0;
    virtual bool deleteBookmark(int id) = 0;
    /**
     * @brief Gets bookmark item
     *
     * @param url of bookmark, pointer to item
     * @return true if success, false on error of not found bookmark
     */
    virtual bool getItem(const std::string & url, tizen_browser::services::BookmarkItem *item) = 0;
    virtual int getFolderId(const std::string & title, int parent = tizen_browser::services::ROOT_FOLDER_ID) = 0;

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkAdded;
    boost::signals2::signal<void (const std::string& uri)> bookmarkDeleted;
    boost::signals2::signal<void ()> bookmarksDeleted;

};

}
}

#endif // FAVORITESERVICE_H
