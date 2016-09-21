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
 * BookmarkFlowUI.cpp
 *
 *  Created on: Nov 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef BOOKMARKFLOWUI_H
#define BOOKMARKFLOWUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "BookmarkFolder.h"
#include "app_i18n.h"

#define BOOKMARK_FLOW_SERVICE "org.tizen.browser.bookmarkflowui"
#define M_UNUSED(x) (void)(x)

namespace tizen_browser{
namespace base_ui{

struct BookmarkUpdate {
    unsigned int folder_id;
    std::string title;
    std::string old_url;
    std::string url;
};

class BROWSER_EXPORT BookmarkFlowUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
        , public tizen_browser::interfaces::AbstractRotatable
{
public:
    BookmarkFlowUI();
    ~BookmarkFlowUI();
    //AbstractUIComponent interface methods
    void init(Evas_Object *parent);
    Evas_Object *getContent();
    void showUI();
    void hideUI();
    void hide();
    void orientationChanged() {};
    virtual std::string getName();

    void setState(bool state);
    void setTitle(const std::string& title);
    void setURL(const std::string& title);
    void setFolder(services::SharedBookmarkItem bookmarkItem);

    boost::signals2::signal<void ()> closeBookmarkFlowClicked;
    boost::signals2::signal<void (BookmarkUpdate)> saveBookmark;
    boost::signals2::signal<void (BookmarkUpdate)> editBookmark;
    boost::signals2::signal<void (services::SharedBookmarkItem)> showSelectFolderUI;

private:
    struct FolderData {
        std::string name;
        unsigned int folder_id;
        std::shared_ptr<tizen_browser::base_ui::BookmarkFlowUI> bookmarkFlowUI;
    };

    struct EntryData {
        std::string category;
        std::string entry;
        std::shared_ptr<tizen_browser::base_ui::BookmarkFlowUI> bookmarkFlowUI;
    };

    struct ObjectData {
        Evas_Object* object;
        std::shared_ptr<tizen_browser::base_ui::BookmarkFlowUI> bookmarkFlowUI;
    };

    Evas_Object* createBookmarkFlowLayout();
    void createGenlistItemClasses();
    Elm_Genlist_Item_Class* createGenlistItemClass(const char* style,
        Elm_Gen_Item_Text_Get_Cb text_cb = nullptr, Elm_Gen_Item_Content_Get_Cb content_cb = nullptr);
    void createTopContent();
    void createGenlist();
    void fillGenlist();
    void updateTopContent();
    void updateDoneButton();

    //Genlist items create callbacks
    static Evas_Object *_genlist_entry_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object *_genlist_folder_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object *_genlist_add_to_qa_content_get(void *data, Evas_Object *obj, const char *part);
    static char* _genlist_entry_text_get(void* data, Evas_Object*, const char* part);
    static char* _genlist_text_get(void* data, Evas_Object*, const char* part);
    static char* _genlist_folder_text_get(void* data, Evas_Object*, const char* part);
    static char* _genlist_add_to_qa_text_get(void* data, Evas_Object*, const char* part);

    static void _done_clicked(void* data, Evas_Object*, void*);
    static void _cancel_clicked(void* data, Evas_Object*, void*);
    static void _entry_focused(void* data, Evas_Object*, void*);
    static void _entry_unfocused(void* data, Evas_Object*, void*);
    static void _entry_changed(void* data, Evas_Object*, void*);
    static void _input_cancel_clicked(void* data, Evas_Object*, void*);
    static void _folder_selector_clicked(void* data, Evas_Object*, void*);
    static void _add_to_qa_state_changed(void* data, Evas_Object*, void*);
    static void _qa_clicked(void* data, Evas_Object*, void*);

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_done_button;
    Evas_Object *m_cancel_button;
    Evas_Object *m_genlist;
    Evas_Object *m_qa_checkbox;

    Elm_Object_Item *m_title_entry;
    Elm_Object_Item *m_url_entry;

    Elm_Genlist_Item_Class *m_entry_item_class;
    Elm_Genlist_Item_Class *m_group_item_class;
    Elm_Genlist_Item_Class *m_folder_item_class;
    Elm_Genlist_Item_Class *m_add_to_qa_item_class;

    services::SharedBookmarkItem m_bookmarkItem;
    std::string m_edjFilePath;
    std::string m_title;
    std::string m_url;
    bool m_add_to_qa;
    bool m_state;
};

}
}

#endif // BOOKMARKFLOWUI_H
