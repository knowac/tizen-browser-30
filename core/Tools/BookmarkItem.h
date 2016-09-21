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

#ifndef BOOKMARKITEM_H
#define BOOKMARKITEM_H

#include "BrowserLogger.h"
#include "BrowserImage.h"

namespace tizen_browser{
namespace services{

class BookmarkItem
{
public:
    BookmarkItem();
    BookmarkItem(
        int id,
        const std::string& url = "",
        const std::string& title = "",
        const std::string& note = "",
        int parent = 0,
        int order = 0
        );
    virtual ~BookmarkItem();

    void setAddress(const std::string & url) { m_url = url; }
    std::string getAddress() const { return m_url; }

    void setTitle(const std::string & title) { m_title = title; }
    std::string getTitle() const { return m_title; }

    void setNote(const std::string& note) { m_note = note; }
    std::string getNote() const { return m_note; }

    void setId(int id) { m_saved_id = id; }
    int getId() const { return m_saved_id; }

    //TODO: create bool with a value when database is updated
    bool getPrivate() const { return false; }

    void setThumbnail(std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail);
    std::shared_ptr<tizen_browser::tools::BrowserImage> getThumbnail() const ;

    void setFavicon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon);
    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon() const;

    void setParent(int parent) { m_parent = parent; }
    int getParent() const { return m_parent; }

    void setOrder(int order) { m_order = order; }
    int getOrder() const { return m_order; }

    void setTags(const std::vector<unsigned int>& tags) { m_tags = tags; }
    std::vector<unsigned int> getTags() const { return m_tags; }

    bool has_thumbnail() const { return m_has_thumbnail; }
    bool has_favicon() const { return m_has_favicon; }
    bool is_folder(void) const { return m_is_folder; }
    bool is_editable(void) const { return m_is_editable; }

    void set_folder_flag(bool flag) { m_is_folder = flag; }
    void set_editable_flag(bool flag) { m_is_editable = flag; }

private:
    int m_saved_id;
    std::string m_url;
    std::string m_title;
    std::string m_note;
    std::shared_ptr<tizen_browser::tools::BrowserImage> m_thumbnail;
    std::shared_ptr<tizen_browser::tools::BrowserImage> m_favicon;
    int m_parent;
    int m_order;
    std::vector<unsigned int> m_tags;
    bool m_has_thumbnail;
    bool m_has_favicon;
    bool m_is_folder;
    bool m_is_editable;
};

typedef std::shared_ptr<BookmarkItem> SharedBookmarkItem;
typedef std::vector<SharedBookmarkItem> SharedBookmarkItemList;

enum FolderIDType {
      ROOT_FOLDER_ID = 0,
      ALL_BOOKMARKS_ID = -1
};

}
}

#endif // BOOKMARKITEM_H

