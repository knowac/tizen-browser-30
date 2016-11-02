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
 *     Author: k.dobkowski
 */

#include "browser_config.h"
#include "BookmarkService.h"
#include <Elementary.h>

#include <boost/any.hpp>
#include <BrowserAssert.h>

#include "app_i18n.h"
#include "ServiceManager.h"
#include "service_macros.h"
#include "BrowserLogger.h"
#include "AbstractWebEngine.h"
#include "AbstractMainWindow.h"
#include "EflTools.h"
#include "GeneralTools.h"

#include <web/web_bookmark.h>
#include "Tools/CapiWebErrorCodes.h"

namespace tizen_browser{
namespace services{

EXPORT_SERVICE(BookmarkService, "org.tizen.browser.favoriteservice")

BookmarkService::BookmarkService()
{
    if(bp_bookmark_adaptor_initialize() < 0) {
        errorPrint("bp_bookmark_adaptor_initialize");
        return;
    }

    m_root = std::make_shared<BookmarkItem>(ROOT_FOLDER_ID, "", _("IDS_BR_BODY_BOOKMARKS"), "", -1, 1);
    m_root->set_folder_flag(true);
}

BookmarkService::~BookmarkService()
{
    bp_bookmark_adaptor_deinitialize();
}

void BookmarkService::errorPrint(std::string method) const
{
    int error_code = bp_bookmark_adaptor_get_errorcode();
    BROWSER_LOGE("%s error: %d (%s)", method.c_str(), error_code,
            tools::capiWebError::bookmarkErrorToString(error_code).c_str());
}

std::shared_ptr<BookmarkItem> BookmarkService::addBookmark(
                                                const std::string & address,
                                                const std::string & title,
                                                const std::string & note,
                                                std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail,
                                                std::shared_ptr<tizen_browser::tools::BrowserImage> favicon,
                                                unsigned int dirId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = 1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int id = -1;
    int *ids = nullptr;
    int ids_count = -1;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, address.c_str(), 0);
    free(ids);
    if (ret < 0){
        BROWSER_LOGE("Error! Could not get ids!");
        return std::make_shared<BookmarkItem>();
    }

    bp_bookmark_info_fmt info;

    std::memset(&info, 0, sizeof(bp_bookmark_info_fmt));
    info.type = 0;
    info.parent = dirId;
    info.sequence = -1;
    info.access_count = -1;
    info.editable = 1;

    if (!address.empty()) {
        info.url = (char*) address.c_str();
    }
    if (!title.empty())
        info.title = (char*) title.c_str();

    if (bp_bookmark_adaptor_easy_create(&id, &info) < 0) {
        errorPrint("bp_bookmark_adaptor_easy_create");
        bp_bookmark_adaptor_easy_free(&info);
        return std::make_shared<BookmarkItem>();
    }
    // max sequence
    int order;
    if ((order = bp_bookmark_adaptor_set_sequence(id, -1)) < 0){
        BROWSER_LOGE("Error! Could not set sequence!");
        return std::make_shared<BookmarkItem>();
    }

    std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(id, address, title, note, dirId, order);
    if (thumbnail && thumbnail->getSize() > 0){
        std::unique_ptr<tizen_browser::tools::Blob> thumb_blob = tizen_browser::tools::EflTools::getBlobPNG(thumbnail);
        if (thumb_blob){
            unsigned char * thumb = std::move((unsigned char*)thumb_blob->getData());
            bp_bookmark_adaptor_set_snapshot(id, thumbnail->getWidth(), thumbnail->getHeight(), thumb, thumb_blob->getLength());
            bookmark->setThumbnail(thumbnail);
        } else
            BROWSER_LOGW("Could not create thumbnail!");
    } else
        BROWSER_LOGW("Thumbnail object does not exist!");

    if (favicon && favicon->getSize() > 0){
        std::unique_ptr<tizen_browser::tools::Blob> favicon_blob = tizen_browser::tools::EflTools::getBlobPNG(favicon);
        if (favicon_blob){
            unsigned char * fav = std::move((unsigned char*)favicon_blob->getData());
            bp_bookmark_adaptor_set_icon(id, favicon->getWidth(), favicon->getHeight(), fav, favicon_blob->getLength());
            bookmark->setFavicon(favicon);
        } else
            BROWSER_LOGW("Could not create favicon!");
    } else
        BROWSER_LOGW("Favicon object does not exist!");

#if PROFILE_MOBILE
    bookmark->set_folder_flag(EINA_FALSE);
#endif
    bookmarkAdded(bookmark);
    return bookmark;
}

void BookmarkService::updateBookmarkItemSnapshot(const std::string & url, tools::BrowserImagePtr snapshot)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int id = getBookmarkId(url);
    if (id != 0 && snapshot) {
        std::unique_ptr<tools::Blob> snapshot_blob = tools::EflTools::getBlobPNG(snapshot);
        unsigned char * snap = std::move((unsigned char*)snapshot_blob->getData());
        if (bp_bookmark_adaptor_set_snapshot(id, snapshot->getWidth(), snapshot->getHeight(), snap,
                snapshot_blob->getLength()) < 0)
            errorPrint("bp_history_adaptor_set_snapshot");
    } else
        BROWSER_LOGW("Snapshot update not handled properly");
}

bool BookmarkService::editBookmark(const std::string & url, const std::string & title, unsigned int folder_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return (update_bookmark_notify(getBookmarkId(url), title.c_str(), url.c_str(), folder_id, -1) == 1);
}

bool BookmarkService::deleteBookmark(const std::string & url)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int id(getBookmarkId(url));
    int ret(0);

    if (id != 0)
        ret = bp_bookmark_adaptor_delete(id);

    return static_cast<bool>(ret);
}

std::shared_ptr<BookmarkItem> BookmarkService::addFolder(const std::string &title, int parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = parent;
    properties.type = 1;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = 1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int id = -1;
    int *ids = nullptr;
    int ids_count = -1;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_TITLE, title.c_str(), 0);
    free(ids);
    if (ret < 0){
        BROWSER_LOGE("Error! Could not get ids!");
        return std::make_shared<BookmarkItem>();
    }

    bp_bookmark_info_fmt info;

    std::memset(&info, 0, sizeof(bp_bookmark_info_fmt));
    info.type = 1;
    info.parent = parent;
    info.sequence = -1;
    info.access_count = -1;
    info.editable = 1;

    if (!title.empty())
        info.title = (char*) title.c_str();

    if (bp_bookmark_adaptor_easy_create(&id, &info) < 0) {
        errorPrint("bp_bookmark_adaptor_easy_create");
        bp_bookmark_adaptor_easy_free(&info);
        return std::make_shared<BookmarkItem>();
    }
    // max sequence
    int order;
    if ((order = bp_bookmark_adaptor_set_sequence(id, -1)) < 0){
        BROWSER_LOGE("Error! Could not set sequence!");
        return std::make_shared<BookmarkItem>();
    }

    std::shared_ptr<BookmarkItem> folder = std::make_shared<BookmarkItem>(id, std::string(""), title, std::string(""), parent, order);
    folder->set_folder_flag(true);
    return folder;
}

std::vector<std::shared_ptr<BookmarkItem>> BookmarkService::getFolders(int parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::vector<std::shared_ptr<BookmarkItem> > folders;
    int *ids = nullptr;
    int ids_count = 0;
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, parent,
            FOLDER_TYPE, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0) < 0) {
        errorPrint("bp_bookmark_adaptor_get_ids_p");
        return std::vector<std::shared_ptr<BookmarkItem>>();
    }

    for(int i = 0; i<ids_count; i++)
    {
        bp_bookmark_info_fmt folder_info;
        if (bp_bookmark_adaptor_get_easy_all(ids[i], &folder_info) == 0) {
            std::string title = (folder_info.title ? folder_info.title : "");
            std::shared_ptr<BookmarkItem> folder = std::make_shared<BookmarkItem>(ids[i], std::string(""),
                    title, std::string(""), folder_info.parent, folder_info.sequence);
            folders.push_back(folder);
        } else {
            BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all error");
        }
        bp_bookmark_adaptor_easy_free(&folder_info);
    }
    free(ids);
    return folders;
}

bool BookmarkService::folderExists(const std::string & title, int parent)
{
    return getFolderId(title, parent);
}

void BookmarkService::editBookmark(int id, const std::string & url, const std::string & title, int parent, int order)
{
    bool is_valid_url = url.size() > 0;
    bool is_valid_title = title.size() > 0;
    bool is_valid_parent = parent != -1;
    bool is_valid_order = order != -1;
    if (is_valid_url)
        bp_bookmark_adaptor_set_url(id, url.c_str());
    if (is_valid_title)
        bp_bookmark_adaptor_set_title(id, title.c_str());
    if (is_valid_parent)
        bp_bookmark_adaptor_set_parent_id(id, parent);
    if (is_valid_order)
        bp_bookmark_adaptor_set_sequence(id, order);
    bp_bookmark_adaptor_publish_notification();
}

std::vector<std::shared_ptr<BookmarkItem>> BookmarkService::getAllBookmarkItems(int parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::vector<std::shared_ptr<BookmarkItem> > bookmarkItems;
    int *ids = nullptr;
    int ids_count = 0;
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, parent, ALL_TYPE, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0) < 0) {
        errorPrint("bp_bookmark_adaptor_get_ids_p");
        return std::vector<std::shared_ptr<BookmarkItem>>();
    }

    for(int i = 0; i < ids_count; i++)
    {
        bp_bookmark_info_fmt bookmarkItemInfo;
        if (bp_bookmark_adaptor_get_easy_all(ids[i], &bookmarkItemInfo) == 0) {
            std::string url = (bookmarkItemInfo.url ? bookmarkItemInfo.url : std::string(""));
            std::string title = (bookmarkItemInfo.title ? bookmarkItemInfo.title : std::string(""));
            std::shared_ptr<BookmarkItem> bookmarkItem = std::make_shared<BookmarkItem>(ids[i], url,
                    title, std::string(""), bookmarkItemInfo.parent, bookmarkItemInfo.sequence);
            bookmarkItem->set_folder_flag(bookmarkItemInfo.type == FOLDER_TYPE);
            if (bookmarkItemInfo.favicon_length > 0) {
                tools::BrowserImagePtr fav = std::make_shared<tools::BrowserImage>(
                        bookmarkItemInfo.favicon_width,
                        bookmarkItemInfo.favicon_height,
                        bookmarkItemInfo.favicon_length);
                fav->setData((void*)bookmarkItemInfo.favicon, false, tools::ImageType::ImageTypePNG);
                bookmarkItem->setFavicon(fav);
            } else
                BROWSER_LOGD("bookmark favicon size is 0");

            bookmarkItems.push_back(bookmarkItem);
        } else {
            BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all error");
        }
        bp_bookmark_adaptor_easy_free(&bookmarkItemInfo);
    }
    free(ids);
    return bookmarkItems;
}

std::shared_ptr<services::BookmarkItem> BookmarkService::getRoot()
{
    return m_root;
}

std::shared_ptr<services::BookmarkItem> BookmarkService::getBookmarkItem(int id)
{
    //TODO: think about extending service delivering options such as get bookmarkitem title/address/bookmark/etc instead
    //of full bookmark item. It might speed up (needs checking) browser - database transactions.

    if (id == ROOT_FOLDER_ID)
        return getRoot();

    std::shared_ptr<BookmarkItem> bookmarkItem = nullptr;
    bp_bookmark_info_fmt info;
    if (bp_bookmark_adaptor_get_easy_all(id, &info) == 0) {
        std::string url = (info.url ? info.url : std::string(""));
        std::string title = (info.title ? info.title : std::string(""));
        bookmarkItem = std::make_shared<BookmarkItem>(id, url, title, std::string(""), info.parent, info.sequence);
        bookmarkItem->set_folder_flag(info.type == FOLDER_TYPE);

        bp_bookmark_adaptor_easy_free(&info);
    } else
        BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all error");

    return bookmarkItem;
}

int BookmarkService::getFolderId(const std::string & title, int parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = parent;
    properties.type = 1;
    properties.is_operator = 0;
    properties.is_editable = -1;
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = -1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;
    int *ids = nullptr;
    int ids_count = 0;
    int i = 0;
    bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_TITLE, title.c_str(), 0);
    if (ids_count > 0){
        i = *ids;
    }
    free(ids);
    return i;
}

bool BookmarkService::getItem(const std::string &url, BookmarkItem *item)
{
    int id;
    if ((id = getBookmarkId(url)) == 0)
        return false;
    return get_item_by_id(id, item);
}

bool BookmarkService::bookmarkExists(const std::string & url)
{
    return 0 != getBookmarkId(url);
}

int BookmarkService::getBookmarkId(const std::string & url)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = 0;
    properties.is_editable = -1;
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = -1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;
    int *ids = nullptr;
    int ids_count = 0;
    int i = 0;
    bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, url.c_str(), 0);
    if (ids_count > 0){
        i = *ids;
    }
    free(ids);
    return i;
}

std::vector<std::shared_ptr<BookmarkItem> > BookmarkService::getBookmarks(int folder_id)
{
    BROWSER_LOGD("[%s:%d] folder_id = %d", __func__, __LINE__, folder_id);
    std::vector<std::shared_ptr<BookmarkItem> > bookmarks;
    int *ids = nullptr;
    int ids_count = 0;
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id,
            ALL_TYPE, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0) < 0 || !ids_count) {
        errorPrint("bp_bookmark_adaptor_get_ids_p");
        return std::vector<std::shared_ptr<BookmarkItem>>();
    }
    BROWSER_LOGD("Bookmark items: %d", ids_count);

    for(int i = 0; i<ids_count; i++)
    {
        bp_bookmark_info_fmt bookmark_info;
        if (bp_bookmark_adaptor_get_easy_all(ids[i], &bookmark_info) == 0) {
            std::string url = (bookmark_info.url ? bookmark_info.url : "");
            std::string title = (bookmark_info.title ? bookmark_info.title : "");

            std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(ids[i], url,
                    title, std::string(""), bookmark_info.parent, bookmark_info.sequence);

            if (bookmark_info.thumbnail_length > 0) {
                tools::BrowserImagePtr bi =
                    std::make_shared<tizen_browser::tools::BrowserImage>(
                        bookmark_info.thumbnail_width,
                        bookmark_info.thumbnail_height,
                        bookmark_info.thumbnail_length);
                bi->setData((void*)bookmark_info.thumbnail, false, tools::ImageType::ImageTypePNG);
                bookmark->setThumbnail(bi);
            } else {
                BROWSER_LOGD("bookmark thumbnail size is 0");
            }

            if (bookmark_info.favicon_length > 0) {
                tools::BrowserImagePtr fav = std::make_shared<tools::BrowserImage>(
                        bookmark_info.favicon_width,
                        bookmark_info.favicon_height,
                        bookmark_info.favicon_length);
                fav->setData((void*)bookmark_info.favicon, false, tools::ImageType::ImageTypePNG);
                bookmark->setFavicon(fav);
            } else {
                BROWSER_LOGD("bookmark favicon size is 0");
            }
            bookmarks.push_back(bookmark);
        } else {
            BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all error");
        }
        bp_bookmark_adaptor_easy_free(&bookmark_info);
    }
    free(ids);
    return bookmarks;
}

bool BookmarkService::deleteAllBookmarks()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_adaptor_reset();
    bookmarksDeleted();
    return true;
}

bool BookmarkService::deleteBookmark(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id[%d]", id);
    if (bp_bookmark_adaptor_delete(id) < 0)
        return false;
    else {
        bp_bookmark_adaptor_publish_notification();
        return true;
    }
}

bool BookmarkService::delete_by_id_notify(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id[%d]", id);

    BookmarkItem item;
    get_item_by_id(id, &item);
    return deleteBookmark(id);
}

bool BookmarkService::delete_by_uri(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("uri[%s]", uri);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = 1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int *ids = nullptr;
    int ids_count = -1;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
    bool result = false;
    if (ret >= 0 && ids_count > 0)
    {
        result = delete_by_id_notify(ids[0]);
        free(ids);
    }

    return result;
}

int BookmarkService::update_bookmark(int id, const char *title, const char *uri, int parent_id, int order,
                                     bool is_duplicate_check_needed, bool is_URI_check_needed)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bool is_URI_exist = (uri != nullptr && strlen(uri) > 0);
    bool is_title_exist = (title != nullptr && strlen(title) > 0);
    int ret = -1;
    if (is_duplicate_check_needed) {
        bp_bookmark_property_cond_fmt properties;
        properties.parent = -1;
        properties.type = 0;
        properties.is_operator = -1;
        properties.is_editable = -1;
        //conditions for querying
        bp_bookmark_rows_cond_fmt conds;
        conds.limit = 1;
        conds.offset = 0;
        conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
        conds.ordering = 0;
        conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
        conds.period_type = BP_BOOKMARK_DATE_ALL;
        int *ids = nullptr;
        int ids_count = -1;
        //This is a bookmark item so check for any such URL already exists
        if (is_URI_exist && is_URI_check_needed) {
            ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
            free(ids);
            if (ret < 0) {
                errorPrint("bp_bookmark_adaptor_get_cond_ids_p");
                return -1;
            }
        }

        if (ids_count > 0) {
            BROWSER_LOGD("same bookmark already exist");
            return 0;
        }
    }

    bp_bookmark_adaptor_set_parent_id(id, parent_id);
    bp_bookmark_adaptor_set_sequence(id, order);
    if (is_URI_exist)
        bp_bookmark_adaptor_set_url(id, uri);
    if (is_title_exist)
        bp_bookmark_adaptor_set_title(id, title);
    bp_bookmark_adaptor_publish_notification();

    return 1;
}

int BookmarkService::update_bookmark_notify(int id, const char *title, const char *uri, int parent_id, int order,
                                            bool is_duplicate_check_needed, bool is_URI_check_needed)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    int ret = update_bookmark(id, title, uri, parent_id, order, is_duplicate_check_needed, is_URI_check_needed);
    return ret;
}

bool BookmarkService::is_in_bookmark(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("uri[%s]", uri);

    int id = 0;
    return get_id(uri, &id);
}

bool BookmarkService::get_id(const char *uri, int *bookmark_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = 1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int *ids = nullptr;
    int ids_count = -1;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
    bool result = ((ret >= 0) && (ids_count > 0));
    if (result) {
        *bookmark_id = ids[0];
    }
    free(ids);
    return result;
}

bool BookmarkService::get_item_by_id(int id, BookmarkItem *item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("ID: %d", id);
    if (!item) {
        BROWSER_LOGE("item is nullptr");
        return false;
    }

    if (id == 0) {
        item->setTitle(_("IDS_BR_BODY_BOOKMARKS"));
        item->setAddress("");
        item->setId(id);
        item->setParent(-1);
        return true;
    }
    bp_bookmark_info_fmt info;
    if (bp_bookmark_adaptor_get_info(id, (BP_BOOKMARK_O_TYPE | BP_BOOKMARK_O_PARENT | BP_BOOKMARK_O_SEQUENCE |
                                     BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |BP_BOOKMARK_O_TITLE), &info) == 0) {
        item->setId(id);
        item->setParent(info.parent);

        if (info.url != nullptr && strlen(info.url) > 0)
            item->setAddress(info.url);

        if (info.title != nullptr && strlen(info.title) > 0)
            item->setTitle(info.title);

        bp_bookmark_adaptor_easy_free(&info);
        return true;
    }
    BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all is failed");
    return false;
}

} /* end of namespace services*/
} /* end of namespace tizen_browser */
