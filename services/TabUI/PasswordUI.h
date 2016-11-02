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

#ifndef PASWORDUI_H
#define PASWORDUI_H

#include <Evas.h>
#include <memory>
#include "AbstractInterfaces/AbstractUIComponent.h"
#include "NaviframeWrapper.h"
#include "TabEnums.h"

namespace tizen_browser {
namespace base_ui {

class PasswordUI : public interfaces::AbstractUIComponent
{
public:
    PasswordUI();
    virtual ~PasswordUI();

    void init(Evas_Object* parent) override;
    Evas_Object* getContent() override;
    void showUI() override;
    void hideUI() override;
    void setState(PasswordState state) { m_state = state; }
    void setAction(PasswordAction action) { m_action = action; }

    boost::signals2::signal<void (std::string, std::string)> setDBStringParamValue;
    boost::signals2::signal<std::string (std::string)> getDBStringParamValue;
    boost::signals2::signal<void (std::string, bool)> setDBBoolParamValue;
    boost::signals2::signal<bool (std::string)> getDBBoolParamValue;
    boost::signals2::signal<void ()> changeEngineState;

    static const std::string PASSWORD_FIELD;
    static const std::string DECISION_MADE;

private:
    struct PasswordUIData {
        std::string text;
        PasswordUI* passwordUI;
    };

    void createGenlistItemClasses();
    Elm_Genlist_Item_Class* createGenlistItemClass(const char* style,
        Elm_Gen_Item_Text_Get_Cb text_cb = nullptr, Elm_Gen_Item_Content_Get_Cb content_cb = nullptr);
    void createLayout();
    void changeState(PasswordState state);
    std::string getDBPassword();
    static bool checkIfStringContainsLetter(const std::string& s);

    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);
    static void _entry_focused(void* data, Evas_Object*, void*);
    static void _entry_unfocused(void* data, Evas_Object*, void*);
    static void _entry_submited(void* data, Evas_Object*, void*);
    static void _show_password_state_changed(void* data, Evas_Object*, void*);
    static void _show_password_clicked(void* data, Evas_Object*, void*);
    static void _use_password_clicked(void* data, Evas_Object*, void*);
    static void _change_password_clicked(void* data, Evas_Object*, void*);

    //Genlist items create callbacks
    static char* _genlist_item_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* _genlist_password_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* _genlist_checkbox_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* _genlist_check_on_off_content_get(void *data, Evas_Object *obj, const char *part);
    Evas_Object* m_parent;
    std::unique_ptr<NaviframeWrapper> m_naviframe;
    Evas_Object* m_genlist;
    Evas_Object* m_entry;
    Evas_Object* m_checkbox;

    Elm_Object_Item *m_password_item;

    Elm_Genlist_Item_Class *m_password_item_class;
    Elm_Genlist_Item_Class *m_checkbox_item_class;
    Elm_Genlist_Item_Class *m_check_on_of_item_class;
    Elm_Genlist_Item_Class *m_text_item_class;

    std::string m_edjFilePath;
    std::string m_not_confirmed_hash;

    static const int PASSWORD_MINIMUM_CHARACTERS;

    PasswordAction m_action;
    PasswordState m_state;
    std::vector<std::shared_ptr<PasswordUIData> > m_genlistItemData;
};

}
}

#endif // PASWORDUI_H
