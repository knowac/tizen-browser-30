/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <Elementary.h>

#include "AbstractUIComponent.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "SettingsPrettySignalConnector.h"
#include "NaviframeWrapper.h"

namespace tizen_browser{
namespace base_ui{

class SettingsUI;

struct ItemData {
    SettingsUI* sui;
    int id;
    std::string buttonText;
    std::string subText;
};

class SettingsUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::interfaces::AbstractRotatable
{
public:
    SettingsUI();
    virtual ~SettingsUI();
    virtual bool populateList(Evas_Object*) { return false; };
    virtual bool populateLayout(Evas_Object*) { return false; };
    virtual void updateButtonMap(){};
    virtual void orientationChanged();
    virtual void connectSignals(){};
    virtual void disconnectSignals(){};
    virtual Evas_Object* createOnOffCheckBox(Evas_Object* obj, ItemData*);
    virtual Evas_Object* createNormalCheckBox(Evas_Object* obj, ItemData*);
    virtual Evas_Object* createRadioButton(Evas_Object*, ItemData*) { return nullptr; }
    virtual void showContextMenu(){};

    void init(Evas_Object* parent);
    Evas_Object* getContent();
    void showUI();
    void hideUI();

    Elm_Gengrid_Item_Class* createItemClass(
            const char* style,
            Elm_Gen_Item_Text_Get_Cb text_cb = nullptr,
            Elm_Gen_Item_Content_Get_Cb con_cb = nullptr);
    Elm_Object_Item* appendGenlist(
            Evas_Object* genlist,
            Elm_Gengrid_Item_Class* it_class,
            const void *data,
            Evas_Smart_Cb func);
    Evas_Object* createActionBar(Evas_Object* settings_layout);
    Evas_Object* createSettingsMobilePage(Evas_Object* settings_layout);
    Evas_Object* createMainView(Evas_Object* settings_layout);
    Evas_Object* getGenlist() { return m_genlist; }
    std::map<SettingsDelPersDataOptions, bool> getOptions() const { return m_option; }
    bool getOption(const SettingsDelPersDataOptions& opt) const { return m_option.at(opt); }
    std::map<SettingsDelPersDataOptions, Evas_Object*> getCheckboxes() { return m_checkboxes; }
    void setOption(const SettingsDelPersDataOptions& option, bool value) { m_option[option] = value; }
    void setCheckboxes(const SettingsDelPersDataOptions& option, Evas_Object* check) { m_checkboxes[option] = check; }
    Evas_Object* getRadioGroup() { return m_radio; }

private:
    Evas_Object* createSettingsUILayout(Evas_Object* parent);
    static void close_clicked_cb(void* data, Evas_Object* obj, void* event_info);

    static void _home_page_cb(void *data, Evas_Object*obj , void* event_info);
    static void _search_engine_cb(void *data, Evas_Object*obj , void* event_info);
    static void _zoom_cb(void *data, Evas_Object*obj , void* event_info);
    static void _advanced_cb(void *data, Evas_Object*obj , void* event_info);

    static char* _gengrid_item_text_get(void*, Evas_Object*, const char*);
    static Evas_Object* _gengrid_item_content_onoff_get(void*, Evas_Object*, const char*);
    static Evas_Object* _gengrid_item_content_normal_get(void*, Evas_Object*, const char*);
    static Evas_Object* _gengrid_item_content_radio_get(void*, Evas_Object*, const char*);
    static void _language_changed(void *data, Evas_Object*obj , void*);

protected:
    SharedNaviframeWrapper m_naviframe;
    Evas_Object* m_genlist;
    std::string m_edjFilePath;
    Evas_Object* m_settings_layout;
    Evas_Object* m_items_layout;
    Evas_Object* m_parent;
    Evas_Object* m_layout;
    std::map<unsigned, ItemData> m_buttonsMap;
    std::map<SettingsDelPersDataOptions, bool> m_option;
    std::map<SettingsDelPersDataOptions,Evas_Object*> m_checkboxes;
    std::map<int, Elm_Object_Item*> m_genlistItems;
    Evas_Object* m_radio;
    Elm_Gengrid_Item_Class* m_setting_item_class;
    Elm_Gengrid_Item_Class* m_setting_double_item_class;
    Elm_Gengrid_Item_Class* m_setting_check_on_of_item_class;
    Elm_Gengrid_Item_Class* m_setting_check_normal_item_class;
    Elm_Gengrid_Item_Class* m_setting_check_radio_item_class;
};

}
}

#endif // SETTINGSUI_H
