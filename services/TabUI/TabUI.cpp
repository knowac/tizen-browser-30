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
#include <vector>
#include <string.h>
#include <AbstractMainWindow.h>

#include "app_i18n.h"
#include "TabUI.h"
#include "TabId.h"
#include "BrowserLogger.h"
#include "BrowserImage.h"
#include "Config/Config.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(TabUI, "org.tizen.browser.tabui")

typedef struct _TabItemData
{
    basic_webengine::TabContentPtr item;
    std::shared_ptr<tizen_browser::base_ui::TabUI> tabUI;
} TabItemData;

TabUI::TabUI()
    : m_button_left(nullptr)
    , m_button_right(nullptr)
    , m_tab_layout(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
    , m_itemToShow(nullptr)
    , m_timer(nullptr)
    , m_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("TabUI/TabUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    createTabItemClass();
    editMode = false;
    onOtherDevicesSwitch = false;
}

TabUI::~TabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_item_class_free(m_item_class);
    evas_object_del(m_tab_layout);
}

void TabUI::createTabItemClass()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_item_class) {
        m_item_class = elm_gengrid_item_class_new();
        m_item_class->item_style = "tab_item";
        m_item_class->func.text_get = _grid_text_get;
        m_item_class->func.content_get =  _tab_grid_content_get;
        m_item_class->func.state_get = nullptr;
        m_item_class->func.del = _del_item;
    }
}

void TabUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_tab_layout);
    evas_object_show(m_tab_layout);
    evas_object_show(elm_layout_content_get(m_tab_layout, "action_bar"));
    evas_object_show(elm_layout_content_get(m_tab_layout, "top_bar"));
#if PROFILE_MOBILE
    evas_object_show(elm_layout_content_get(m_tab_layout, "tab_gengrid"));
    evas_object_show(elm_layout_content_get(m_tab_layout, "bottom_bar"));
    orientationChanged();
#else
    elm_object_focus_next_object_set(elm_layout_content_get(elm_object_part_content_get(m_tab_layout, "top_bar"), "openedtabs_click"),
                                     elm_object_part_content_get(elm_object_part_content_get(m_tab_layout, "action_bar"), "closetabs_click"),
                                     ELM_FOCUS_UP);
    elm_object_focus_next_object_set(elm_object_part_content_get(elm_object_part_content_get(m_tab_layout, "action_bar"), "closetabs_click"),
                                     elm_layout_content_get(elm_object_part_content_get(m_tab_layout, "top_bar"), "openedtabs_click"),
                                     ELM_FOCUS_DOWN);
    elm_object_focus_next_object_set(elm_object_part_content_get(elm_object_part_content_get(m_tab_layout, "action_bar"), "closetabs_click"),
                                     elm_object_part_content_get(elm_object_part_content_get(m_tab_layout, "action_bar"), "newincognitotab_click"),
                                     ELM_FOCUS_LEFT);
#endif
}

void TabUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_tab_layout);

    elm_gengrid_clear(m_gengrid);
    evas_object_hide(m_tab_layout);
    evas_object_hide(elm_layout_content_get(m_tab_layout, "action_bar"));
    evas_object_hide(elm_layout_content_get(m_tab_layout, "top_bar"));
#if PROFILE_MOBILE
    evas_object_hide(elm_layout_content_get(m_tab_layout, "tab_gengrid"));
    evas_object_hide(elm_layout_content_get(m_tab_layout, "bottom_bar"));
#endif
}

void TabUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* TabUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if(!m_tab_layout)
        createTabUILayout();
    return m_tab_layout;
}

void TabUI::createTabUILayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_tab_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_tab_layout, m_edjFilePath.c_str(), "tab-layout");
    evas_object_size_hint_weight_set(m_tab_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_tab_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    //Create button bars and put them to layout's swallows
    Evas_Object* topBar = createTopButtons(m_tab_layout);
    elm_object_part_content_set(m_tab_layout, "top_bar", topBar);

    Evas_Object* buttonBar = createActionBar(m_tab_layout);
    elm_object_part_content_set(m_tab_layout, "action_bar", buttonBar);

#if PROFILE_MOBILE
    m_gengrid_layout = elm_layout_add(m_tab_layout);
    elm_layout_file_set(m_gengrid_layout, m_edjFilePath.c_str(), "gengrid_layout");
    evas_object_size_hint_weight_set(m_gengrid_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_gengrid_layout);

    m_gengrid = createGengrid(m_gengrid_layout);
    elm_object_part_content_set(m_gengrid_layout, "gengrid_swallow", m_gengrid);

    elm_object_part_content_set(m_tab_layout, "tab_gengrid", m_gengrid_layout);

    buttonBar = createBottomBar(m_tab_layout);
    elm_object_part_content_set(m_tab_layout, "bottom_bar", buttonBar);

    elm_object_signal_emit(topBar, "mouse,clicked,1", "opened_tab_border");
    elm_object_signal_emit(topBar, "clicked", "openedtabs_click");
#else
    m_gengrid = createGengrid(m_tab_layout);
    elm_object_part_content_set(m_tab_layout, "tab_gengrid", m_gengrid);
#endif
}

Evas_Object* TabUI::createBottomBar(Evas_Object* parent)
{
    M_ASSERT(parent);

    Evas_Object* box = elm_box_add(parent);
    elm_box_horizontal_set(box, EINA_TRUE);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_button_left = elm_button_add(box);
    elm_object_style_set(m_button_left, "tab_button");
    evas_object_size_hint_weight_set(m_button_left, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_left, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_button_left, "elm.text", "New Tab");
    elm_box_pack_end(box, m_button_left);
    evas_object_smart_callback_add(m_button_left, "clicked", _newtab_clicked, this);
#if PROFILE_MOBILE
    elm_object_signal_emit(m_button_left, "enabled", "ui");
#endif
    evas_object_show(m_button_left);

    m_button_right = elm_button_add(box);
    elm_object_style_set(m_button_right, "tab_button");
    evas_object_size_hint_weight_set(m_button_right, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_right, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_button_right, "elm.text", "New incognito Tab");
    elm_box_pack_end(box, m_button_right);
    evas_object_smart_callback_add(m_button_right, "clicked", _newincognitotab_clicked, this);
    elm_object_signal_emit(m_button_right, "visible", "ui");
#if PROFILE_MOBILE
    elm_object_signal_emit(m_button_right, "enabled", "ui");
#endif
    evas_object_show(m_button_right);

    evas_object_show(box);
    return box;
}

Evas_Object* TabUI::createGengrid(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);

    Evas_Object* gengrid = elm_gengrid_add(parent);
    elm_gengrid_align_set(gengrid, 0.5, 0.0);
    elm_gengrid_select_mode_set(gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(gengrid, EINA_FALSE);

#if PROFILE_MOBILE
    elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
    elm_scroller_page_size_set(gengrid, ELM_SCALE_SIZE(720), ELM_SCALE_SIZE(0));
    elm_gengrid_item_size_set(gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
    elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_bounce_set(gengrid, EINA_FALSE, EINA_FALSE);
#else
    elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
    elm_scroller_page_size_set(gengrid, ELM_SCALE_SIZE(0), ELM_SCALE_SIZE(327));
    elm_gengrid_item_size_set(gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
    elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
    evas_object_event_callback_add(gengrid, EVAS_CALLBACK_MOUSE_IN, _focus_in, this);
#endif
    elm_gengrid_highlight_mode_set(gengrid, EINA_TRUE);
    evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return gengrid;
}

Evas_Object* TabUI::createActionBar(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    Evas_Object* actionBarLayout = elm_layout_add(parent);

    elm_layout_file_set(actionBarLayout, m_edjFilePath.c_str(), "action_bar_layout");
    evas_object_size_hint_weight_set(actionBarLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBarLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(actionBarLayout);

    Evas_Object* button = elm_button_add(actionBarLayout);

#if !PROFILE_MOBILE
    elm_object_style_set(button, "invisible_button");
    evas_object_smart_callback_add(button, "clicked", _closetabs_clicked, this);
    elm_object_part_content_set(actionBarLayout, "closetabs_click", button);

    button = elm_button_add(actionBarLayout);
    elm_object_style_set(button, "invisible_button");
    evas_object_smart_callback_add(button, "clicked", _newincognitotab_clicked, this);
    elm_object_part_content_set(actionBarLayout, "newincognitotab_click", button);

    button = elm_button_add(actionBarLayout);
    elm_object_style_set(button, "invisible_button");
    evas_object_smart_callback_add(button, "clicked", _newtab_clicked, this);
    elm_object_part_content_set(actionBarLayout, "newtab_click", button);

    button = elm_button_add(actionBarLayout);
#endif

    elm_object_style_set(button, "invisible_button");
    evas_object_smart_callback_add(button, "clicked", _close_clicked, this);
    elm_object_part_content_set(actionBarLayout, "close_click", button);

    return actionBarLayout;
}

void TabUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI * tabUI = static_cast<TabUI*>(data);
        tabUI->closeTabUIClicked();
        tabUI->editMode = false;
        tabUI->onOtherDevicesSwitch = false;
    }
}

void TabUI::onBackKey()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    editMode = false;
    elm_layout_text_set(elm_layout_content_get(m_tab_layout, "action_bar"), "closetabs_text", "Close Tabs");
}

#if PROFILE_MOBILE
void TabUI::orientationChanged()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> landscape = isLandscape();
    if (landscape) {
        if (*landscape) {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH_LANDSCAPE), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT_LANDSCAPE));
            elm_object_signal_emit(m_gengrid_layout, "switch_landscape", "ui");
        } else {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
            elm_object_signal_emit(m_gengrid_layout, "switch_vertical", "ui");
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}
#endif

Evas_Object* TabUI::createTopButtons(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);

    Evas_Object* topLayout = elm_layout_add(parent);
    elm_layout_file_set(topLayout, m_edjFilePath.c_str(), "top_buttons_layout");
    evas_object_size_hint_weight_set(topLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(topLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object *button = elm_button_add(topLayout);
    elm_object_style_set(button, "invisible_button");
    evas_object_smart_callback_add(button, "clicked", _openedtabs_clicked, this);
    evas_object_show(button);
    elm_layout_content_set(topLayout, "openedtabs_click", button);

#if ON_OTHER_DEVICES
    button = elm_button_add(topLayout);
    elm_object_style_set(button, "invisible_button");
    evas_object_smart_callback_add(button, "clicked", _onotherdevices_clicked, this);
    evas_object_show(button);
    elm_layout_content_set(topLayout, "onotherdevices_click", button);
#endif

    return topLayout;
}

void TabUI::_newtab_clicked(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        tabUI->newTabClicked();
        tabUI->editMode = false;
        tabUI->onOtherDevicesSwitch = false;
    }

}

void TabUI::_openedtabs_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        if(tabUI->onOtherDevicesSwitch) {
            tabUI->onOtherDevicesSwitch = false;
            evas_object_show(tabUI->m_gengrid);
            elm_layout_text_set(elm_layout_content_get(tabUI->m_tab_layout, "action_bar"), "closetabs_text", "Close Tabs");
            tabUI->editMode = false;
        }
    }
}

#if ON_OTHER_DEVICES
void TabUI::_onotherdevices_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        if(!tabUI->onOtherDevicesSwitch) {
            tabUI->onOtherDevicesSwitch = true;
            evas_object_hide(tabUI->m_gengrid);
            elm_layout_text_set(elm_layout_content_get(tabUI->m_tab_layout, "action_bar"), "closetabs_text", "Close Tabs");
            tabUI->editMode = false;
        }
    }
}
#endif

void TabUI::_newincognitotab_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        tabUI->newIncognitoTabClicked();
        tabUI->editMode = false;
        tabUI->onOtherDevicesSwitch = false;
    }
}

void TabUI::_closetabs_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        if (!tabUI->editMode && !tabUI->onOtherDevicesSwitch) {
            tabUI->editMode = true;
            BROWSER_LOGD("[%s:%d] --------> edit mode: %d ", __PRETTY_FUNCTION__, __LINE__, tabUI->editMode);
            elm_object_translatable_part_text_set(elm_layout_content_get(tabUI->m_tab_layout, "action_bar"), "closetabs_text", "IDS_BR_OPT_CLOSE_ALL");
        } else if (tabUI->editMode && !tabUI->onOtherDevicesSwitch) {
            tabUI->editMode = false;
            tabUI->closeAllTabs();
            elm_gengrid_realized_items_update(tabUI->m_gengrid);
        }
    }
}

void TabUI::addTabItem(basic_webengine::TabContentPtr hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TabItemData *itemData = new TabItemData();
    itemData->item = hi;
    itemData->tabUI = std::shared_ptr<tizen_browser::base_ui::TabUI>(this);
    Elm_Object_Item* tabView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, _thumbSelected, itemData);
    // Check if item_object was created successfully
    if (tabView) {
        // unselect by default
        elm_gengrid_item_selected_set(tabView, EINA_FALSE);
        setEmptyGengrid(false);
    }
}

void TabUI::addTabItems(std::vector<basic_webengine::TabContentPtr>& items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_layout_text_set(elm_layout_content_get(m_tab_layout, "action_bar"), "closetabs_text", "Close Tabs");
    editMode = false;
    for (auto it = items.begin(); it < items.end(); ++it) {
        addTabItem(*it);
    }

#if PROFILE_MOBILE
    if (tabsCount() >= boost::any_cast<int>(tizen_browser::config::Config::getInstance().get("TAB_LIMIT"))) {
        elm_object_signal_emit(m_button_left, "dissabled", "ui");
        elm_object_signal_emit(m_button_right, "dissabled", "ui");
    } else {
        elm_object_signal_emit(m_button_left, "enabled", "ui");
        elm_object_signal_emit(m_button_right, "enabled", "ui");
    }
#endif
}

char* TabUI::_grid_text_get(void *data, Evas_Object*, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        TabItemData *itemData = static_cast<TabItemData*>(data);
        const char *part_name1 = "tab_title";
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "tab_url";
        static const int part_name2_len = strlen(part_name2);

        if (!strncmp(part_name1, part, part_name1_len)) {
            return strdup(itemData->item->getTitle().c_str());
        }
        if (!strncmp(part_name2, part, part_name2_len)) {
            //return strdup(itemData->item->getUrl().c_str());
            return strdup("");
        }
    }
    return strdup("");
}

Evas_Object * TabUI::_tab_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
    if (data && obj && part) {
        TabItemData *itemData = static_cast<TabItemData*>(data);
        const char *part_name1 = "tab_thumbnail";
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "tab_thumbButton";
        static const int part_name2_len = strlen(part_name2);
        const char *part_name3 = "tab_incognitoButton";
        static const int part_name3_len = strlen(part_name3);
        const char *part_name4 = "tab_selectedButton";
        static const int part_name4_len = strlen(part_name4);
        if (!strncmp(part_name1, part, part_name1_len)) {
            if (itemData->item->getThumbnail()) {
                return itemData->item->getThumbnail()->getEvasImage(obj);
            }
        }
        if (!strncmp(part_name2, part, part_name2_len)) {
            Evas_Object* button = elm_button_add(obj);
            elm_object_style_set(button, "invisible_button");
            evas_object_smart_callback_add(button, "clicked", _close_tab_clicked, data);
            return button;
        }
        if (!strncmp(part_name3, part, part_name3_len)) {
            boost::optional<bool> isIncognito = itemData->tabUI->isIncognito(itemData->item->getId());
            if (isIncognito && *isIncognito) {
#if PROFILE_MOBILE
                Evas_Object* button = elm_button_add(obj);
                elm_object_style_set(button, "tab_incognitoButton");
                return button;
#else
                Evas_Object *incognitoImage = elm_image_add(obj);
                if (elm_image_file_set(incognitoImage,itemData->tabUI->m_edjFilePath.c_str() , "incognito_icon")) {
                    return incognitoImage;
                }
#endif
            }
        }
        if (!strncmp(part_name4, part, part_name4_len)) {
            Evas_Object* button = elm_button_add(obj);
            elm_object_style_set(button, "invisible_button");
            evas_object_smart_callback_add(button, "clicked", _itemSelected, data);
            return button;
        }
    }
    return nullptr;
}

void TabUI::_del_item(void* /*data*/, Evas_Object* obj)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_smart_callback_del(elm_object_part_content_get(obj, "tab_thumbButton"), "clicked", _close_tab_clicked);
    evas_object_smart_callback_del(elm_object_part_content_get(obj, "tab_selectedButton"), "clicked", _itemSelected);
}

void TabUI::_itemSelected(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    TabItemData *itemData = static_cast<TabItemData*>(data);
    itemData->tabUI->tabClicked(itemData->item->getId());
}

void TabUI::_thumbSelected(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
#if !PROFILE_MOBILE
        TabItemData *itemData = static_cast<TabItemData*>(data);
        if (!itemData->tabUI->editMode) {
            _itemSelected(data, nullptr, nullptr);
        } else {
            _close_tab_clicked(data, nullptr, nullptr);
        }
#endif
    }
}

Eina_Bool TabUI::_ready(void *data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto* self = static_cast<TabUI*>(data);

    elm_gengrid_item_selected_set(self->m_itemToShow, EINA_TRUE);
    elm_gengrid_item_show(self->m_itemToShow, ELM_GENGRID_ITEM_SCROLLTO_TOP);

    ecore_timer_del(self->m_timer);
    self->m_itemToShow = nullptr;
    self->m_timer = nullptr;

    return ECORE_CALLBACK_CANCEL;
}

void TabUI::_close_tab_clicked(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabItemData* itemData = static_cast<TabItemData*>(data);

        Elm_Object_Item* it = elm_gengrid_selected_item_get(itemData->tabUI->m_gengrid);
        auto prev = elm_gengrid_item_prev_get(it);
        auto next = elm_gengrid_item_next_get(it);
        elm_object_item_del(it);

        if (prev || next) {
            itemData->tabUI->m_itemToShow = prev ? prev : next;
            // Timer added because of issue http://suprem.sec.samsung.net/jira/browse/TWF-831
            // The body of the timer should be added here if elm_object_item_del will be
            // synchronous or much faster.
            itemData->tabUI->m_timer = ecore_timer_add(0.1, _ready, itemData->tabUI.get());
        }

        elm_gengrid_realized_items_update(itemData->tabUI->m_gengrid);

        itemData->tabUI->closeTabsClicked(itemData->item->getId());

#if PROFILE_MOBILE
        elm_object_signal_emit(itemData->tabUI->m_button_left, "enabled", "ui");
        elm_object_signal_emit(itemData->tabUI->m_button_right, "enabled", "ui");
#endif
    }
}

Evas_Object* TabUI::createNoHistoryLabel()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *label = elm_label_add(m_parent);
    elm_object_text_set(label, "No favorite websites.");
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    return label;
}

void TabUI::closeAllTabs()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* it = elm_gengrid_first_item_get(m_gengrid);
    while (it) {
        TabItemData *item = (TabItemData *)elm_object_item_data_get(it);
        item->tabUI->closeTabsClicked(item->item->getId());
        it = elm_gengrid_item_next_get(it);
    }
    closeTabUIClicked();
}

void TabUI::setEmptyGengrid(bool setEmpty)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoHistoryLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", nullptr);
    }
}

void TabUI::_focus_in(void* data, Evas*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Event_Mouse_In* ee  = (Evas_Event_Mouse_In*)event_info;
    int x, y;
    TabUI* tabUI = static_cast<TabUI*>(data);
    Elm_Object_Item* it = elm_gengrid_at_xy_item_get(tabUI->m_gengrid, ee->canvas.x, ee->canvas.y, &x, &y);
    if(it && tabUI->editMode)
        elm_object_item_signal_emit(it, "selected", "over3");
}

bool TabUI::isEditMode()
{
    return editMode;
}

}
}
