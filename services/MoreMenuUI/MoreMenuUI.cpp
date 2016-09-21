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
 */

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "app_i18n.h"
#include "MoreMenuUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "BrowserImage.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(MoreMenuUI, "org.tizen.browser.moremenuui")

struct ItemData{
        tizen_browser::base_ui::MoreMenuUI * m_moreMenu;
        std::shared_ptr<tizen_browser::services::HistoryItem> h_item;
        Elm_Object_Item * e_item;
    };

typedef struct _MoreItemData
{
    ItemType item;
    std::shared_ptr<tizen_browser::base_ui::MoreMenuUI> moreMenuUI;
} MoreMenuItemData;

MoreMenuUI::MoreMenuUI()
    : m_current_tab_bar(nullptr)
    , m_mm_layout(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
    , m_toastPopup(nullptr)
    , m_icon(nullptr)
    , m_bookmarkIcon(nullptr)
    , m_item_class(nullptr)
    , m_desktopMode(true)
#if PROFILE_MOBILE
    , m_shouldShowFindOnPage(false)
    , m_blockThumbnails(false)
#endif
    , m_isVisible(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("MoreMenuUI/MoreMenu.edj");
    m_item_class = createItemClass();
}

MoreMenuUI::~MoreMenuUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_item_class)
        elm_gengrid_item_class_free(m_item_class);
    evas_object_del(m_gengrid);
}

Elm_Gengrid_Item_Class* MoreMenuUI::createItemClass()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Gengrid_Item_Class* item_class = elm_gengrid_item_class_new();
    item_class->item_style = "menu_item";
    item_class->func.text_get = _grid_text_get;
    item_class->func.content_get =  _grid_content_get;
    item_class->func.state_get = NULL;
    item_class->func.del = NULL;
    return item_class;
}

void MoreMenuUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

#if PROFILE_MOBILE
void MoreMenuUI::updateBookmarkButton()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_item_update(m_map_menu_views[ADD_TO_BOOKMARK]);
}
#endif

void MoreMenuUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_mm_layout)
        createMoreMenuLayout();
    createGengrid();    // recreate gengrid because icons could have changed
    addItems();
#if !PROFILE_MOBILE
    m_focusManager.startFocusManager(m_gengrid);
    setFocus(EINA_TRUE);
#endif
    evas_object_show(m_mm_layout);
    evas_object_show(elm_object_part_content_get(m_mm_layout,"current_tab_bar"));
#if PROFILE_MOBILE
    resetContent();
#endif
    m_isVisible = true;
}

void MoreMenuUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_mm_layout);

    evas_object_hide(m_mm_layout);
    evas_object_hide(elm_object_part_content_get(m_mm_layout,"current_tab_bar"));
    clearItems();
    evas_object_del(m_gengrid);

#if PROFILE_MOBILE
    elm_object_signal_emit(m_parent, "hide_moremenu", "ui");
    deleteMoreMenuLayout();
#else
    setFocus(EINA_FALSE);
    m_focusManager.stopFocusManager();
#endif
    m_isVisible = false;
}


Evas_Object* MoreMenuUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if(!m_mm_layout)
        createMoreMenuLayout();
    return m_mm_layout;
}

#if PROFILE_MOBILE
void MoreMenuUI::resetContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> rotated = isRotated();
    if (rotated) {
        if (*rotated) {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH_LANDSCAPE),
                                      ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT_LANDSCAPE));
            elm_object_signal_emit(m_mm_layout, "switch_landscape", "ui");
            if (evas_object_visible_get(m_mm_layout))
                elm_object_signal_emit(m_parent, "show_moremenu_landscape", "ui");
        } else {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH),
                                      ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
            elm_object_signal_emit(m_mm_layout, "switch_vertical", "ui");
            if (evas_object_visible_get(m_mm_layout))
                elm_object_signal_emit(m_parent, "show_moremenu_vertical", "ui");
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}
#endif

void MoreMenuUI::createMoreMenuLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    elm_theme_extension_add(NULL, m_edjFilePath.c_str());
    m_mm_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_mm_layout, m_edjFilePath.c_str(), "moremenu-layout");
    evas_object_size_hint_weight_set(m_mm_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_mm_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

#if PROFILE_MOBILE
    elm_object_part_content_set(m_parent, "moremenu", m_mm_layout);
    elm_object_tree_focus_allow_set(m_mm_layout, EINA_FALSE);
#endif
}

#if PROFILE_MOBILE
void MoreMenuUI::deleteMoreMenuLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_parent);
    M_ASSERT(m_mm_layout);

    evas_object_del(m_gengrid);
    evas_object_hide(m_mm_layout);
    elm_object_signal_emit(m_parent, "hide_moremenu", "ui");
    evas_object_del(m_mm_layout);

    m_mm_layout = nullptr;
}
#endif

void MoreMenuUI::createGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_gengrid = elm_gengrid_add(m_mm_layout);
    elm_object_part_content_set(m_mm_layout, "elm.swallow.grid", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
#if PROFILE_MOBILE
    elm_scroller_bounce_set(m_gengrid, EINA_FALSE, EINA_FALSE);
    elm_object_scroll_lock_x_set(m_gengrid, EINA_TRUE);
#else
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
#endif
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void MoreMenuUI::showCurrentTab()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_current_tab_bar = elm_layout_add(m_mm_layout);
    elm_layout_file_set(m_current_tab_bar, m_edjFilePath.c_str(), "current_tab_layout");
    evas_object_size_hint_weight_set(m_current_tab_bar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(m_mm_layout, "current_tab_bar", m_current_tab_bar);

    Evas_Object* button = elm_button_add(m_current_tab_bar);
    elm_object_style_set(button, "hidden_button");
    evas_object_smart_callback_add(button, "clicked", _close_clicked, this);
    elm_object_part_content_set(m_current_tab_bar, "close_click", button);
    evas_object_show(button);
    elm_object_focus_set(button, EINA_TRUE);

    m_bookmarkButton = elm_button_add(m_mm_layout);
    elm_object_style_set(m_bookmarkButton, "hidden_button");
    evas_object_show(m_bookmarkButton);
    evas_object_smart_callback_add(m_bookmarkButton, "clicked", _bookmarkButton_clicked, this);

    m_bookmarkIcon = elm_icon_add(m_mm_layout);
    elm_object_part_content_set(m_current_tab_bar, "bookmark_ico", m_bookmarkIcon);
    elm_object_part_content_set(m_current_tab_bar, "star_click", m_bookmarkButton);
#if !PROFILE_MOBILE
    createFocusVector();
#endif
}

void MoreMenuUI::setFavIcon(tools::BrowserImagePtr favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(favicon && favicon->getSize() > 0) {
        if(m_icon)
            evas_object_del(m_icon);

        m_icon = favicon->getEvasImage(m_current_tab_bar);
        if(m_icon) {
            evas_object_size_hint_weight_set(m_icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(m_icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
            elm_object_part_content_set(m_current_tab_bar, "favicon", m_icon);
            evas_object_show(m_icon);
        }
    }
    else {
        setDocIcon();
    }
}

void MoreMenuUI::setDocIcon()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_icon)
        evas_object_del(m_icon);

    m_icon = elm_icon_add(m_mm_layout);
    elm_image_file_set(m_icon, m_edjFilePath.c_str(), "ico_url.png");
    evas_object_size_hint_weight_set(m_icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(m_current_tab_bar, "favicon", m_icon);
}

void MoreMenuUI::setWebTitle(const std::string& title)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, title.c_str());
    elm_object_part_text_set(m_current_tab_bar, "webpage_title", title.c_str());
}

void MoreMenuUI::setURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, url.c_str());

    boost::optional<bool> bookmark = isBookmark();
    if (bookmark) {
        if (!url.empty()) {
            elm_object_part_text_set(m_current_tab_bar, "webpage_url", url.c_str());
            if (*bookmark) {
                changeBookmarkStatus(true);
                enableAddToBookmarkButton(true);
            } else {
                changeBookmarkStatus(false);
                enableAddToBookmarkButton(true);
            }
        } else {
            elm_object_part_text_set(m_current_tab_bar, "webpage_url", "");
            elm_object_part_text_set(m_current_tab_bar, "webpage_title", "No Content");
            changeBookmarkStatus(false);
            enableAddToBookmarkButton(false);
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}

void MoreMenuUI::setHomePageInfo()
{
    setDocIcon();
    setURL("");
}

void MoreMenuUI::changeBookmarkStatus(bool data)
{
    if (data) {
        elm_object_part_text_set(m_current_tab_bar, "add_to_bookmark_text", "Remove Bookmark");
        elm_image_file_set(m_bookmarkIcon, m_edjFilePath.c_str(), "ic_add_bookmark.png");
    }
    else {
        elm_object_part_text_set(m_current_tab_bar, "add_to_bookmark_text", "Add to Bookmark");
        elm_image_file_set(m_bookmarkIcon, m_edjFilePath.c_str(), "ic_add_bookmark_new.png");
    }
}

void MoreMenuUI::enableAddToBookmarkButton(bool data)
{
    if (m_bookmarkButton) {
        elm_object_disabled_set(m_bookmarkButton, data ? EINA_FALSE : EINA_TRUE);
        elm_object_style_set(m_bookmarkButton, data ? "hidden_button" : "dimmed_button");
    }
}

void MoreMenuUI::createToastPopup(const char* text)
{
    m_toastPopup = elm_popup_add(m_mm_layout);
    elm_object_style_set(m_toastPopup, "toast");
    evas_object_size_hint_weight_set(m_toastPopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_toastPopup, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(m_current_tab_bar, "toast_popup", m_toastPopup);
    elm_object_part_text_set(m_current_tab_bar, "toast_text", text);
    evas_object_smart_callback_add(m_toastPopup, "timeout", _timeout, this);
    elm_popup_timeout_set(m_toastPopup, 3.0);
}

void MoreMenuUI::_timeout(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    MoreMenuUI *moreMenuUI = static_cast<MoreMenuUI*>(data);
    elm_object_part_text_set(moreMenuUI->m_current_tab_bar, "toast_text", "");
    evas_object_del(moreMenuUI->m_toastPopup);
}

void MoreMenuUI::_bookmarkButton_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        MoreMenuUI *moreMenuUI = static_cast<MoreMenuUI*>(data);
        moreMenuUI->bookmarkFlowClicked();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void MoreMenuUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        MoreMenuUI *moreMenuUI = static_cast<MoreMenuUI*>(data);
        moreMenuUI->closeMoreMenuClicked();
    }
}

void MoreMenuUI::addItems()
{
     BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
     for (int i = 0; i <= FIND_ON_PAGE; i++) {
         ItemType type = static_cast<ItemType>(i);
         if (type == ItemType::FIND_ON_PAGE && !m_shouldShowFindOnPage)
             continue;
         addItem(type);
     }
#else
     for (int i = 0; i <= EXIT_BROWSER; i++) {
         ItemType type = static_cast<ItemType>(i);
         // take proper image for desktop/mobile view
         if (type == ItemType::VIEW_DESKTOP_WEB && m_desktopMode)
             continue;
         if (type == ItemType::VIEW_MOBILE_WEB && !m_desktopMode)
             continue;
         addItem(type);
     }
#endif
}

void MoreMenuUI::addItem(ItemType type)
{
    MoreMenuItemData *itemData = new MoreMenuItemData();
    itemData->item = type;
    itemData->moreMenuUI = std::shared_ptr<tizen_browser::base_ui::MoreMenuUI>(this);
    Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, _thumbSelected, itemData);
    m_map_menu_views.insert(std::pair<ItemType, Elm_Object_Item*>(itemData->item, bookmarkView));
    elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
}

void MoreMenuUI::clearItems()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = m_map_menu_views.begin(); it != m_map_menu_views.end(); ++it) {
        Elm_Object_Item* bookmarkView = it->second;
        Evas_Object *button = elm_object_item_part_content_get(bookmarkView, "thumbbutton_item");
        evas_object_event_callback_del(button, EVAS_CALLBACK_MOUSE_IN, __cb_mouse_in);
        evas_object_event_callback_del(button, EVAS_CALLBACK_MOUSE_OUT, __cb_mouse_out);
    }

    elm_gengrid_clear(m_gengrid);
    m_map_menu_views.clear();
}

char* MoreMenuUI::_grid_text_get(void* data, Evas_Object*, const char* part)
{
    BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
    if (data && part) {
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
#if PROFILE_MOBILE
        const char *part_name = "thumbnail_text";
#else
        const char *part_name = "menu_label";
#endif
        static const int part_name_len = strlen(part_name);

        if (!strncmp(part_name, part, part_name_len)) {
            const char* item_name = NULL;
            switch (itemData->item) {
            case HISTORY:
                item_name = "History Manager";
                break;
            case BOOKMARK_MANAGER:
                item_name = "Bookmark Manager";
                break;
            case SETTINGS:
                item_name = _("IDS_BR_OPT_SETTINGS");
                break;
#if PROFILE_MOBILE
            case ADD_TO_BOOKMARK: {
                boost::optional<bool> bookmark = itemData->moreMenuUI->isBookmark();
                if (bookmark)
                    item_name = *bookmark ? "Edit Bookmark" : "Add to Bookmark";
                else
                    BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
                }
                break;
#ifdef READER_MODE_ENABLED
            case READER_MODE:
                item_name = _("IDS_BR_OPT_READER_MODE_TTS");
                break;
#endif
            case FIND_ON_PAGE:
                item_name = _("IDS_BR_OPT_FIND_ON_PAGE");
                break;
#else
#ifdef READER_MODE_ENABLED
            case READER_MODE:
                item_name = _("IDS_BR_OPT_READER_MODE_TTS");
                break;
#endif
            case SCREEN_ZOOM:
                item_name = "Screen Zoom";
                break;
#ifdef START_MINIBROWSER_ENABLED
            case START_MINIBROWSER:
                item_name = "Start Mini Browser";
                break;
#endif
            case VIEW_MOBILE_WEB:
                item_name = "View Mobile Web";
                break;
            case VIEW_DESKTOP_WEB:
                item_name = "View Desktop Web";
                break;
            case EXIT_BROWSER:
                item_name = "Exit Browser";
                break;
#endif
            default:
                item_name = "";
            }
            return strdup(item_name);
        }
    }
    return NULL;
}

static const char* getImageFileNameForType(ItemType type, bool focused, Eina_Bool bookmarked)
{
#if PROFILE_MOBILE
    M_UNUSED(focused);
#else
    M_UNUSED(bookmarked);
#endif
    const char* file_name = NULL;
    switch (type) {
#if PROFILE_MOBILE
        case ADD_TO_BOOKMARK:
            file_name = bookmarked == EINA_TRUE ? "moremenu_ic_01_edit.png" : "moremenu_ic_01.png";
            break;
#ifdef READER_MODE_ENABLED
        case READER_MODE:
            file_name = "moremenu_ic_02.png";
            break;
#endif
        case HISTORY:
            file_name = "moremenu_ic_04.png";
            break;
        case BOOKMARK_MANAGER:
            file_name = "moremenu_ic_05.png";
            break;
        case SETTINGS:
            file_name = "moremenu_ic_06.png";
            break;
        case FIND_ON_PAGE:
            file_name = "moremenu_ic_07.png";
            break;
#else
    #ifdef READER_MODE_ENABLED
        case READER_MODE:
            file_name = focused ? "ic_more_readermode_foc.png" : "ic_more_readermode_nor.png";
            break;
    #endif
        case BOOKMARK_MANAGER:
            file_name = focused ? "ic_more_bookmark_foc.png" : "ic_more_bookmark_nor.png";
            break;
        case HISTORY:
            file_name = focused ? "ic_more_history_foc.png" : "ic_more_history_nor.png";
            break;
        case SCREEN_ZOOM:
            file_name = focused ? "ic_more_zoom_foc.png" : "ic_more_zoom_nor.png";
            break;
    #ifdef START_MINIBROWSER_ENABLED
        case START_MINIBROWSER:
            file_name = focused ? "ic_more_minibrowser_foc.png" : "ic_more_minibrowser_nor.png";
            break;
    #endif
        case VIEW_MOBILE_WEB:
            file_name = focused ? "ic_more_mobileview_foc.png" : "ic_more_mobileview_nor.png";
            break;
        case VIEW_DESKTOP_WEB:
            file_name = focused ? "ic_more_desktopview_foc.png" : "ic_more_desktopview_nor.png";
            break;
        case SETTINGS:
            file_name = focused ? "ic_more_setting_foc.png" : "ic_more_setting_nor.png";
            break;
        case EXIT_BROWSER:
            file_name = focused ? "ic_more_exit_foc.png" : "ic_more_exit_nor.png";
            break;
#endif
    default:
        file_name = "";
    }
    return file_name;
}

Evas_Object * MoreMenuUI::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
    if (data && obj && part) {
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
#if PROFILE_MOBILE
        const char *part_name1 = "thumbnail_icon";
#else
        const char *part_name1 = "thumbnail_item";
#endif
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "thumbbutton_item";
        static const int part_name2_len = strlen(part_name2);

        if (!strncmp(part_name1, part, part_name1_len)) {
            Evas_Object* thumb_nail = elm_icon_add(obj);
            boost::optional<bool> bookmark = itemData->moreMenuUI->isBookmark();
            if (bookmark) {
                const char* file_name = getImageFileNameForType(itemData->item, false, *bookmark);
                elm_image_file_set(thumb_nail, itemData->moreMenuUI->m_edjFilePath.c_str(), file_name);
                return thumb_nail;
            } else
                BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
        }

        if (!strncmp(part_name2, part, part_name2_len)) {
            Evas_Object *thumbButton = elm_button_add(obj);
#if PROFILE_MOBILE
            elm_object_style_set(thumbButton, "invisible_button");
            evas_object_smart_callback_add(thumbButton, "clicked", _thumbSelected, data);
            elm_object_part_content_set(obj, "thumbnail_click", thumbButton);
#else
            elm_object_style_set(thumbButton, "clickButton");
            evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_IN, __cb_mouse_in, data);
            evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_OUT, __cb_mouse_out, data);
#endif
            return thumbButton;
        }
    }
    return NULL;
}

void MoreMenuUI::__cb_mouse_in(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    M_UNUSED(data);
    M_UNUSED(obj);
#else
    if (data && obj) {
        elm_object_focus_set(obj, EINA_TRUE);
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
        Elm_Object_Item *selected = itemData->moreMenuUI->m_map_menu_views[itemData->item];
        Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "thumbnail_item");
        boost::optional<bool> bookmark = itemData->moreMenuUI->isBookmark();
        if (bookmark) {
            const char *file_name = getImageFileNameForType(itemData->item, true, *bookmark);
            elm_image_file_set(thumb_nail, itemData->moreMenuUI->m_edjFilePath.c_str(), file_name);
        } else
            BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
    }
#endif
}

void MoreMenuUI::__cb_mouse_out(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    M_UNUSED(data);
    M_UNUSED(obj);
#else
    if (data && obj) {
        elm_object_focus_set(obj, EINA_FALSE);
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
        Elm_Object_Item *selected = itemData->moreMenuUI->m_map_menu_views[itemData->item];
        Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "thumbnail_item");
        boost::optional<bool> bookmark = itemData->moreMenuUI->isBookmark();
        if (bookmark) {
            const char *file_name = getImageFileNameForType(itemData->item, false, *bookmark);
            elm_image_file_set(thumb_nail, itemData->moreMenuUI->m_edjFilePath.c_str(), file_name);
        } else
            BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
    }
#endif
}


void MoreMenuUI::_thumbSelected(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
    BROWSER_LOGD("type: %d", itemData->item);
        switch (itemData->item) {
            case HISTORY:
                itemData->moreMenuUI->historyUIClicked();
                break;
            case SETTINGS:
                itemData->moreMenuUI->settingsClicked();
                break;
            case BOOKMARK_MANAGER:
                itemData->moreMenuUI->bookmarkManagerClicked();
                break;
#if PROFILE_MOBILE
            case FIND_ON_PAGE:
                itemData->moreMenuUI->closeMoreMenuClicked();
                itemData->moreMenuUI->findOnPageClicked();
                break;
            case ADD_TO_BOOKMARK:
                if (!itemData->moreMenuUI->m_blockThumbnails)
                    elm_object_focus_allow_set(itemData->moreMenuUI->m_gengrid, EINA_FALSE);
                    itemData->moreMenuUI->bookmarkFlowClicked();
                break;
#ifdef READER_MODE_ENABLED
            case READER_MODE:
                //TODO: Implement reader mode
                break;
#endif
#else
#ifdef READER_MODE_ENABLED
            case READER_MODE:
                //TODO: Implement reader mode
                break;
#endif
            case SCREEN_ZOOM:
                itemData->moreMenuUI->zoomUIClicked();
                break;
#ifdef START_MINIBROWSER_ENABLED
            case START_MINIBROWSER:
                //TODO: Implement minibrowser launching
                break;
#endif
            case VIEW_MOBILE_WEB:
                itemData->moreMenuUI->switchToMobileMode();
                itemData->moreMenuUI->m_desktopMode = false;
                itemData->moreMenuUI->closeMoreMenuClicked();
                break;
            case VIEW_DESKTOP_WEB:
                itemData->moreMenuUI->switchToDesktopMode();
                itemData->moreMenuUI->m_desktopMode = true;
                itemData->moreMenuUI->closeMoreMenuClicked();
                break;
            case EXIT_BROWSER:
                _exitClicked();
                break;
#endif
        default:
            BROWSER_LOGD("[%s:%d] Warning: Unhandled button.", __PRETTY_FUNCTION__, __LINE__);
            break;
        }
    }
}

void MoreMenuUI::_exitClicked()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    elm_exit();
}

void MoreMenuUI::setFocus(Eina_Bool focusable)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    elm_object_tree_focus_allow_set(getContent(), focusable);
    if (focusable == EINA_TRUE)
        elm_object_focus_set(elm_object_part_content_get(m_current_tab_bar, "close_click"), focusable);
}

#if PROFILE_MOBILE
void MoreMenuUI::shouldShowFindOnPage(bool show)
{
    m_shouldShowFindOnPage = show;
}

void MoreMenuUI::blockThumbnails(bool blockThumbnails)
{
    m_blockThumbnails = blockThumbnails;
}
#endif

#if !PROFILE_MOBILE
void MoreMenuUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(elm_object_part_content_get(m_current_tab_bar, "close_click"));
    m_focusManager.addItem(m_bookmarkButton);
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}
#endif

}
}
