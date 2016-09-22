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

#ifndef AUTOFILLFORMCOMPOSEVIEW_H_
#define AUTOFILLFORMCOMPOSEVIEW_H_

#include "AutoFillFormItem.h"

namespace tizen_browser{
namespace base_ui{

class AutoFillFormManager;
class AutoFillFormComposeView {
public:
    AutoFillFormComposeView(AutoFillFormManager* manager, AutoFillFormItem *item = NULL);
    ~AutoFillFormComposeView(void);

    Evas_Object* show(Evas_Object *parent, Evas_Object* action_bar = nullptr);
    void hide();
    void rotateLandscape();
    void rotatePortrait();
private:
    enum menu_type
    {
        profile_composer_title_full_name = 0,
        profile_composer_title_company_name,
        profile_composer_title_address_line_1,
        profile_composer_title_address_line_2,
        profile_composer_title_city_town,
        profile_composer_title_country,
        profile_composer_title_post_code,
        profile_composer_title_country_region,
        profile_composer_title_phone,
        profile_composer_title_email,
        profile_composer_menu_end
    };

    struct genlistCallbackData {
        menu_type type;
        void *user_data;
        Evas_Object* editfield;
        Evas_Object* entry;
        Evas_Object* it;
    };

    Evas_Object* createScroller(Evas_Object* parent);
    void addItems();
    Eina_Bool isEntryHasOnlySpace(const char *);
    Eina_Bool applyEntryData(void);
    void createInputLayout(Evas_Object* parent, char* fieldName, genlistCallbackData* cb_data);

    static void __done_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __cancel_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_changed_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_next_key_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_clear_button_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void __editfield_changed_cb(void* data, Evas_Object* obj, void* event_info);
    void hide_action_bar();
    AutoFillFormItem *m_itemForCompose;
    AutoFillFormManager *m_manager;

    Evas_Object *m_mainLayout;
    Evas_Object *m_parent;
    Evas_Object *m_scroller;
    Evas_Object *m_box;
    Evas_Object *m_doneButton;
    Evas_Object *m_cancelButton;
    Evas_Object *m_action_bar;

    profileEditErrorcode m_editErrorcode;
    profileSaveErrorcode m_saveErrorcode;
    Elm_Genlist_Item_Class *m_editFieldItemClass;
    Elm_Entry_Filter_Limit_Size m_entryLimitSize;

    genlistCallbackData m_fullNameItemCallbackData;
    genlistCallbackData m_companyNameItemCallbackData;
    genlistCallbackData m_addressLine1ItemCallbackData;
    genlistCallbackData m_addressLine2ItemCallbackData;
    genlistCallbackData m_cityTownItemCallbackData;
    genlistCallbackData m_countryItemCallbackData;
    genlistCallbackData m_postCodeItemCallbackData;
    genlistCallbackData m_countryRegionItemCallbackData;
    genlistCallbackData m_phoneItemCallbackData;
    genlistCallbackData m_emailItemCallbackData;
    std::string m_edjFilePath;
};

}
}

#endif /* AUTOFILLFORMCOMPOSEVIEW_H_ */
