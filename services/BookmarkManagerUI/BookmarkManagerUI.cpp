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

#include <Elementary.h>
#include <boost/format.hpp>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "BookmarkManagerUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"
#include "app_i18n.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarkManagerUI, "org.tizen.browser.bookmarkmanagerui")

struct ItemData
{
    tizen_browser::base_ui::BookmarkManagerUI * m_bookmarkManager;
    tizen_browser::services::BookmarkItem * h_item;
    Elm_Object_Item * e_item;
};

BookmarkManagerUI::BookmarkManagerUI()
    : m_parent(nullptr)
    , b_mm_layout(nullptr)
    , m_topContent(nullptr)
    , m_gengrid(nullptr)
    #if !PROFILE_MOBILE
    , m_bottom_content(nullptr)
    #else
    , m_folder_new_item_class(nullptr)
    #endif
    , m_folder_all_item_class(nullptr)
    , m_folder_mobile_item_class(nullptr)
    , m_folder_custom_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkManagerUI/BookmarkManagerUI.edj");
    createGengridItemClasses();
}

BookmarkManagerUI::~BookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if(m_folder_all_item_class)
        elm_gengrid_item_class_free(m_folder_all_item_class);

    if(m_folder_custom_item_class)
        elm_gengrid_item_class_free(m_folder_custom_item_class);

#if PROFILE_MOBILE
    if(m_folder_new_item_class)
        elm_gengrid_item_class_free(m_folder_new_item_class);
#endif
    if(m_folder_mobile_item_class)
        elm_gengrid_item_class_free(m_folder_mobile_item_class);
}

void BookmarkManagerUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void BookmarkManagerUI::showUI()
{
    evas_object_show(b_mm_layout);
#if !PROFILE_MOBILE
    m_focusManager.startFocusManager(m_gengrid);
#else
    orientationChanged();
#endif
}

void BookmarkManagerUI::hideUI()
{
    evas_object_hide(b_mm_layout);
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark.clear();
#if !PROFILE_MOBILE
    m_focusManager.stopFocusManager();
#endif
}

Evas_Object* BookmarkManagerUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!b_mm_layout)
      b_mm_layout = createBookmarksLayout(m_parent);
    elm_object_part_text_set(m_topContent, "title_text", "Bookmark Manager"); //TODO: add translation

    return b_mm_layout;
}

void BookmarkManagerUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_folder_all_item_class = elm_gengrid_item_class_new();
    m_folder_all_item_class->item_style = "grid_all_item";
    m_folder_all_item_class->func.text_get = _grid_all_folder_title_text_get;
    m_folder_all_item_class->func.content_get =  nullptr;
    m_folder_all_item_class->func.state_get = nullptr;
    m_folder_all_item_class->func.del = _grid_content_delete;

    m_folder_custom_item_class = elm_gengrid_item_class_new();
    m_folder_custom_item_class->item_style = "grid_custom_folder_item";
    m_folder_custom_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_custom_item_class->func.content_get =  nullptr;
    m_folder_custom_item_class->func.state_get = nullptr;
    m_folder_custom_item_class->func.del = _grid_content_delete;

#if PROFILE_MOBILE
    m_folder_new_item_class = elm_gengrid_item_class_new();
    m_folder_new_item_class->item_style = "grid_new_folder_item";
    m_folder_new_item_class->func.text_get = nullptr;
    m_folder_new_item_class->func.content_get =  nullptr;
    m_folder_new_item_class->func.state_get = nullptr;
    m_folder_new_item_class->func.del = _grid_content_delete;
#endif

    m_folder_mobile_item_class = elm_gengrid_item_class_new();
    m_folder_mobile_item_class->item_style = "grid_mobile_folder_item";
    m_folder_mobile_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_mobile_item_class->func.content_get =  nullptr;
    m_folder_mobile_item_class->func.state_get = nullptr;
    m_folder_mobile_item_class->func.del = _grid_content_delete;
}

void BookmarkManagerUI::_grid_content_delete(void *data, Evas_Object */*obj*/)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    auto itemData = static_cast<FolderData*>(data);
    if (itemData)
        delete itemData;
}

char* BookmarkManagerUI::_grid_all_folder_title_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        FolderData *folderData = static_cast<FolderData*>(data);
        const char *part_name1 = "page_title";
        const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
            return strdup((boost::format("%s  (%d)") % _("IDS_BR_BODY_ALL") % folderData->count).str().c_str());
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return strdup("");
}

char* BookmarkManagerUI::_grid_folder_title_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        FolderData *folderData = static_cast<FolderData*>(data);
        const char *part_name1 = "page_title";
        const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
            return strdup((boost::format("%s  (%d)") % folderData->name.c_str() % folderData->count).str().c_str());
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return strdup("");
}

Evas_Object* BookmarkManagerUI::createBookmarksLayout(Evas_Object* parent)
{
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    b_mm_layout = elm_layout_add(parent);
    elm_layout_file_set(b_mm_layout, m_edjFilePath.c_str(), "bookmarkmanager-layout");
    evas_object_size_hint_weight_set(b_mm_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(b_mm_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(b_mm_layout);

    createGengrid();
    createTopContent();
#if !PROFILE_MOBILE
    createBottomContent();
    createFocusVector();
#endif
    return b_mm_layout;
}

//TODO: Make parend the argument and return created object to make code more modular.
//      (After fixing window managment)
void BookmarkManagerUI::createGengrid()
{
    m_gengrid = elm_gengrid_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
#if PROFILE_MOBILE
    elm_scroller_bounce_set(m_gengrid, EINA_FALSE, EINA_TRUE);
    elm_object_scroll_lock_x_set(m_gengrid, EINA_TRUE);
#else
    elm_object_style_set(m_gengrid, "back_ground");
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
    elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
#endif
}

void BookmarkManagerUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(b_mm_layout);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_topContent = elm_layout_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "top_content", m_topContent);
    evas_object_size_hint_weight_set(m_topContent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_topContent, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_topContent);

    elm_layout_file_set(m_topContent, m_edjFilePath.c_str(), "topContent");

    Evas_Object* close_button = elm_button_add(m_topContent);
    elm_object_style_set(close_button, "invisible_button");
    evas_object_smart_callback_add(close_button, "clicked", _close_clicked_cb, this);
    elm_object_part_content_set(m_topContent, "close_click", close_button);

    evas_object_show(close_button);
    elm_object_focus_custom_chain_append(m_topContent, close_button, nullptr);
    elm_object_focus_set(close_button, EINA_TRUE);
    elm_object_tree_focus_allow_set(b_mm_layout, EINA_TRUE);
    elm_object_focus_allow_set(close_button, EINA_TRUE);
}

#if !PROFILE_MOBILE
void BookmarkManagerUI::createBottomContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(b_mm_layout);

    m_bottom_content = elm_layout_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "bottom_content", m_bottom_content);
    evas_object_size_hint_weight_set(m_bottom_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bottom_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_bottom_content);

    elm_layout_file_set(m_bottom_content, m_edjFilePath.c_str(), "bottom-content");
}

void BookmarkManagerUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(elm_object_part_content_get(m_topContent, "close_click"));
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}
#endif

void BookmarkManagerUI::_close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkManagerUI* id = static_cast<BookmarkManagerUI*>(data);
        id->closeBookmarkManagerClicked();
    }
}

void BookmarkManagerUI::addCustomFolder(FolderData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* BookmarkView;
    if (item->folder_id == m_all_folder_id)
        BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_all_item_class,
                                                            item, _bookmarkAllFolderClicked, item);
    else if (item->folder_id == m_special_folder_id)
        BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_mobile_item_class,
                                                            item, _bookmarkMobileFolderClicked, item);
    else
        BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_custom_item_class,
                                                            item, _bookmarkCustomFolderClicked, item);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
}

void BookmarkManagerUI::setFoldersId(unsigned int all, unsigned int special)
{
    m_all_folder_id = all;
    m_special_folder_id = special;
}

#if PROFILE_MOBILE
void BookmarkManagerUI::addNewFolder()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_new_item_class,
                                                            NULL, _bookmarkNewFolderClicked, this);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
}

void BookmarkManagerUI::orientationChanged()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> portrait = isLandscape();

    if (portrait) {
        if (*portrait) {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH_LANDSCAPE),
                                      ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT_LANDSCAPE));
            elm_object_signal_emit(b_mm_layout, "switch_landscape", "ui");
        } else {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH),
                                      ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
            elm_object_signal_emit(b_mm_layout, "switch_vertical", "ui");
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}
#endif

void BookmarkManagerUI::addCustomFolders(services::SharedBookmarkFolderList folders)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = folders.begin(); it != folders.end(); ++it) {
        FolderData *folderData = new FolderData();
        folderData->name = (*it)->getName();
        folderData->folder_id = (*it)->getId();
        folderData->count = (*it)->getCount();
        folderData->bookmarkManagerUI = this;
        addCustomFolder(folderData);
    }
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_gengrid);
    evas_object_show(m_gengrid);
#if !PROFILE_MOBILE
    elm_object_part_text_set(m_bottom_content, "text", (boost::format("%d %s") % elm_gengrid_items_count(m_gengrid) % "folders").str().c_str());
#endif
}

void BookmarkManagerUI::addCustomFolders(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->is_folder()) {
            FolderData *folderData = new FolderData();
            int count = 0;

            for (auto it2 = items.begin(); it2 != items.end(); ++it2) {
                BROWSER_LOGD("%d %d %d",(*it2)->is_folder(),(*it2)->getDir(),(*it)->getId());
                if (!(*it2)->is_folder() && (*it2)->getDir()==(*it)->getId()) {
                    count++;
                }
            }
            folderData->name = (*it)->getTitle();
            folderData->count = count;
            folderData->folder_id = (*it)->getId();
            folderData->bookmarkManagerUI = this;
            addCustomFolder(folderData);
        }
    }
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid",m_gengrid);
    evas_object_show(m_gengrid);
}

void BookmarkManagerUI::_bookmarkCustomFolderClicked(void * data , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData * itemData = (FolderData*)(data);
        BROWSER_LOGD("Folder Name: %s" , itemData->name.c_str());
        itemData->bookmarkManagerUI->customFolderClicked(itemData->folder_id);
    }
}
#if PROFILE_MOBILE
void BookmarkManagerUI::_bookmarkNewFolderClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        bookmarkManagerUI->newFolderItemClicked();
    }
}
#endif
void BookmarkManagerUI::_bookmarkAllFolderClicked(void * data , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData * itemData = (FolderData*)(data);
        itemData->bookmarkManagerUI->allFolderClicked();
    }
}

void BookmarkManagerUI::_bookmarkMobileFolderClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData * itemData = (FolderData*)(data);
        itemData->bookmarkManagerUI->specialFolderClicked();
    }
}

}
}
