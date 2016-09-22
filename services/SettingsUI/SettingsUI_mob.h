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

#ifndef SETTINGSUI_H
#define SETTINGSUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractRotatable.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "../../core/AbstractWebEngine/WebEngineSettings.h"

namespace tizen_browser{
namespace base_ui{

class SettingsUI;

struct ItemData {
    std::string buttonText;
};

class AutoFillFormManager;
class BROWSER_EXPORT SettingsUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::interfaces::AbstractRotatable
        , public tizen_browser::core::AbstractService
{
public:
    SettingsUI();
    ~SettingsUI();
    void init(Evas_Object* parent);
    void updateButtonMap();
    Evas_Object* getContent();
    void showUI();
    void hideUI();
    void onBackKey();
    bool isSubpage();
    virtual std::string getName();
    virtual void orientationChanged();
    Evas_Object* createActionBar(Evas_Object* settings_layout);
    Evas_Object* createBackActionBar(Evas_Object* settings_layout);
    Evas_Object* createSettingsMobilePage(Evas_Object* settings_layout);
    Evas_Object* createMainView(Evas_Object* settings_layout);
    Evas_Object* createInfoField(const char* name, const char* text, Evas_Object* parent);
    Evas_Object* createDelDataMobilePage(Evas_Object* settings_layout);
    Evas_Object* createDelDataMobileCheckBoxes(Evas_Object* parent);
    Evas_Object* createRemoveMostVisitedMobilePage(Evas_Object* settings_layout);
    Evas_Object* createRemoveBrowserDataMobilePage(Evas_Object* settings_layout);
    Evas_Object* createContentSettingsPage(Evas_Object* settings_layout);
    Evas_Object* createPrivacyPage(Evas_Object* settings_layout);
    Evas_Object* createDeveloperOptionsMobilePage(Evas_Object* settings_layout);
    Evas_Object* createUserAgentGenList(Evas_Object* settings_layout);

    boost::signals2::signal<void ()> resetBrowserClicked;
    boost::signals2::signal<void ()> resetMostVisitedClicked;
    boost::signals2::signal<void (std::string&)> deleteSelectedDataClicked;
    boost::signals2::signal<void (const std::string&)> userAgentItemClicked;
    boost::signals2::signal<void ()> closeSettingsUIClicked;
    boost::signals2::signal<bool (basic_webengine::WebEngineSettings)> getWebEngineSettingsParam;
    boost::signals2::signal<void (basic_webengine::WebEngineSettings, bool)> setWebEngineSettingsParam;

private:
    Evas_Object* createSettingsUILayout(Evas_Object* parent);
    void resetItemsLayoutContent();
    void initializeAutoFillManager();
    void destroyAutoFillManager();
    static void close_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void back_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static Evas_Object* createCheckBox(Evas_Object* layout, const std::string name, Edje_Signal_Cb func, void* data);
    static void __checkbox_label_click_cb(void* data, Evas_Object* obj, const char* emission, const char* source);
    static void __checkbox_content_settings_label_click_cb(void* data, Evas_Object* obj, const char* emission, const char* source);
    static void __checkbox_privacy_label_click_cb(void* data, Evas_Object* obj, const char* emission, const char* source);

    static void _auto_fill_data_menu_clicked_cb(void* data, Evas_Object* obj, void* event_info);

    static void _del_selected_data_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void _del_selected_data_menu_clicked_cb(void* data, Evas_Object* obj, void* event_info);

    static void _reset_mv_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void _reset_mv_menu_clicked_cb(void* data, Evas_Object* obj, void* event_info);

    static void _reset_browser_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void _reset_browser_menu_clicked_cb(void* data, Evas_Object* obj, void* event_info);

    static void _content_settings_menu_clicked_cb(void* data, Evas_Object* obj, void* event_info);

    static void _privacy_menu_clicked_cb(void* data, Evas_Object* obj, void * event_info);

    static void _override_useragent_clicked_cb(void* data, Evas_Object* obj, void * event_info);
    static void _developer_menu_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void _useragent_item_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static char* _ua_text_get_cb(void* data, Evas_Object* obj, const char *part);

    static char* _gengrid_item_text_get(void* /*data*/, Evas_Object* obj, const char* /*part*/);
    static void _language_changed(void *data, Evas_Object*obj , void*);

    std::unique_ptr<AutoFillFormManager> m_autoFillManager;
    Evas_Object* m_settings_layout;
    Evas_Object* m_actionBar;
    Evas_Object* m_subpage_layout;
    Evas_Object* m_items_layout;
    Evas_Object* m_parent;
    Evas_Object* m_checkbox_layout;

    std::string m_edjFilePath;
    Elm_Gengrid_Item_Class* m_setting_item_class;
    Elm_Gengrid_Item_Class* m_setting_parent_item_class;
    std::map<unsigned, ItemData> m_buttonsMap;
};

}
}

#endif // BOOKMARKSUI_H
