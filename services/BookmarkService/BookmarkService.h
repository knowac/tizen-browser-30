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
 * Created on: Dec, 2014
 *     Author: m.kielak
 */

#ifndef BOOKMARKSERVICE_H
#define BOOKMARKSERVICE_H

#include "browser_config.h"
#include <vector>
#include <boost/signals2/signal.hpp>
#include <Evas.h>

#include "AbstractService.h"
#include "service_macros.h"
#include "BookmarkItem.h"
#include "BrowserImage.h"
#include "AbstractFavoriteService.h"

namespace tizen_browser{
namespace services{

class BROWSER_EXPORT BookmarkService
        : public tizen_browser::interfaces::AbstractFavoriteService
{
public:
    BookmarkService();
    virtual ~BookmarkService();
    virtual std::string getName();

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
    std::shared_ptr<BookmarkItem> addBookmark(const std::string & address,
                                                 const std::string & title,
                                                 const std::string & note = std::string(),
                                                 std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail=std::shared_ptr<tizen_browser::tools::BrowserImage>(),
                                                 std::shared_ptr<tizen_browser::tools::BrowserImage> favicon = std::shared_ptr<tizen_browser::tools::BrowserImage>(),
                                                 unsigned int dirId = 0);

    /**
     * @brief Update bookmark snapshot by given url
     *
     * @param url of bookmark to delete
     */
    void updateBookmarkItemSnapshot(const std::string & url, std::shared_ptr<tizen_browser::tools::BrowserImage> snapshot);

    /** \todo Need to change this callback function for finding stored bookmark, check getBookmarkId function
     * @brief Check if bookmark exists
     *
     * @param url url to find
     * @return true if exists, false if not
     */
     bool bookmarkExists(const std::string & url);

    /**
     * @brief Get bookmarks from platform service and store it in private m_bookmarksList
     *
     * @return list of bookmark items, bookmark items in a folder & bookmark folders
     */
    std::vector<std::shared_ptr<BookmarkItem> > getBookmarks(int folder_id = -1);

   /**
     * @brief Delete all bookmarks
     *
     * @return true if success, false on error
     */
    bool deleteAllBookmarks();

    /**
      * @brief Edit bookmark title and folder by given url
      *
      * @return true if success, false on error
      */
    bool editBookmark(const std::string & url, const std::string & title, unsigned int folder_id = 0);

    /**
     * @brief Delete bookmark by given url
     *
     * @param url of bookmark to delete
     * @return true if success, false on error of not found bookmark
     */
    bool deleteBookmark(const std::string & url);

    bool deleteBookmark(int id);

    std::shared_ptr<BookmarkItem> addFolder(const std::string & title, int parent = ROOT_FOLDER_ID);
    std::vector<std::shared_ptr<BookmarkItem>> getFolders(int parent = ROOT_FOLDER_ID);
    bool folderExists(const std::string & title, int parent = ROOT_FOLDER_ID);

    std::vector<std::shared_ptr<BookmarkItem>> getAllBookmarkItems(int parent = ROOT_FOLDER_ID);

    std::shared_ptr<services::BookmarkItem> getRoot();
    int getQuickAccessRoot() {return m_quickAccess_root;}

    std::shared_ptr<services::BookmarkItem> getBookmarkItem(int id);

    void editBookmark(int id, const std::string & url, const std::string & title,
        int parent = -1, int order = -1);

    /**
     * @brief Gets bookmark item
     *
     * @param url of bookmark, pointer to item
     * @return true if success, false on error of not found bookmark
     */
    bool getItem(const std::string & url, BookmarkItem *item);

    bool delete_by_id_notify(int id);
    bool delete_by_uri(const char *uri);
    int update_bookmark(int id, const char *title, const char *uri, int parent_id, int order,
                        bool is_duplicate_check_needed = false, bool is_URI_check_needed = false);
    int update_bookmark_notify(int id, const char *title, const char *uri, int parent_id, int order,
                               bool is_duplicate_check_needed = false, bool is_URI_check_needed = false);
    bool delete_all(void);
    bool get_item_by_id(int id, BookmarkItem *item);
    int get_count(void);
    bool get_id(const char *uri, int *bookmark_id);
    bool is_in_bookmark(const char *uri);
    int getFolderId(const std::string & title, int parent);

private:
    /**
     * Help method printing last bp_bookmark_error_defs error.
     */
    void errorPrint(std::string method) const;

    enum ItemType{
          FOLDER_TYPE = 1
        , BOOKMARK_TYPE = 0
        , ALL_TYPE = -1
    };
    std::vector<BookmarkItem *> m_bookmark_list;
    std::shared_ptr<tizen_browser::services::StorageService> m_storageManager;
///    \todo Need to change getBookmarkId function for finding stored bookmark - check getBookmarkExists function
    int getBookmarkId(const std::string & url);

    std::shared_ptr<services::BookmarkItem> m_root;
    int m_quickAccess_root;
};

}
}

#endif // FAVORITESERVICE_H
