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

#include "AutoFillFormManager.h"
#include "AutoFillFormComposeView.h"
#include "AutoFillFormListView.h"
#include "BrowserLogger.h"
#include "app_i18n.h"

#include <Ecore.h>
#include <Elementary.h>
#include <Evas.h>

#define TRIM_SPACE " \t\n\v"

namespace tizen_browser{
namespace base_ui{

inline std::string _trim(std::string& s, const std::string& drop = TRIM_SPACE)
{
    std::string r = s.erase(s.find_last_not_of(drop) + 1);
    return r.erase(0, r.find_first_not_of(drop));
}

AutoFillFormComposeView::AutoFillFormComposeView(AutoFillFormManager* manager, AutoFillFormItem *item)
    : m_itemForCompose(nullptr)
    , m_manager(manager)
    , m_mainLayout(nullptr)
    , m_parent(nullptr)
    , m_scroller(nullptr)
    , m_doneButton(nullptr)
    , m_cancelButton(nullptr)
    , m_action_bar(nullptr)
    , m_editErrorcode(update_error_none)
    , m_saveErrorcode(save_error_none)
    , m_editFieldItemClass(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (item && item->getItemComposeMode() == profile_edit)
        m_itemForCompose = item;
    else
        m_itemForCompose = new AutoFillFormItem(nullptr);

    m_entryLimitSize.max_char_count = 0;
    m_entryLimitSize.max_byte_count = AUTO_FILL_FORM_ENTRY_MAX_COUNT;
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");
}

AutoFillFormComposeView::~AutoFillFormComposeView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_smart_callback_del(m_cancelButton, "clicked", __cancel_button_cb);
    evas_object_smart_callback_del(m_doneButton, "clicked", __done_button_cb);

    if (m_editFieldItemClass)
        elm_genlist_item_class_free(m_editFieldItemClass);

    if (m_itemForCompose && m_itemForCompose->getItemComposeMode() == profile_create)
        delete m_itemForCompose;
    m_itemForCompose = nullptr;

    if (m_mainLayout) {
        evas_object_hide(m_mainLayout);
        evas_object_del(m_mainLayout);
    }
    m_mainLayout = nullptr;
    m_scroller = nullptr;
}

Evas_Object* AutoFillFormComposeView::show(Evas_Object* parent, Evas_Object* action_bar)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_action_bar = action_bar;
    elm_object_signal_emit(m_action_bar, "show,buttons,signal", "but_vis");
    elm_object_signal_emit(m_action_bar, "hide,close,icon", "del_but");

    m_mainLayout = elm_layout_add(parent);
    if (!m_mainLayout) {
        BROWSER_LOGE("createMainLayout failed");
        return nullptr;
    }
    elm_layout_file_set(m_mainLayout, m_edjFilePath.c_str(), "affcv-layout");
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_mainLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(m_mainLayout, "cancel_text", "IDS_BR_SK_CANCEL");
    elm_object_translatable_part_text_set(m_mainLayout, "done_text", "IDS_BR_SK_DONE");

    m_scroller = createScroller(m_mainLayout);
    evas_object_show(m_scroller);
    elm_object_part_content_set(m_mainLayout, "affcv_genlist", m_scroller);
    evas_object_show(m_mainLayout);

    if (m_itemForCompose->getItemComposeMode() == profile_edit)
        elm_object_translatable_part_text_set(m_action_bar, "settings_title", "Edit info");
    else
        elm_object_translatable_part_text_set(m_action_bar, "settings_title", "Add info");

    m_cancelButton = elm_button_add(m_action_bar);
    if (!m_cancelButton) {
        BROWSER_LOGE("Failed to create m_cancelButton");
        return nullptr;
    }
    elm_object_style_set(m_cancelButton, "basic_button");
    evas_object_smart_callback_add(m_cancelButton, "clicked", __cancel_button_cb, this);
    elm_object_part_content_set(m_action_bar, "cancel_button", m_cancelButton);

    m_doneButton = elm_button_add(m_action_bar);
    if (!m_doneButton) {
        BROWSER_LOGE("Failed to create m_doneButton");
        return nullptr;
    }
    elm_object_style_set(m_doneButton, "basic_button");
    evas_object_smart_callback_add(m_doneButton, "clicked", __done_button_cb, this);
    elm_object_part_content_set(m_action_bar, "done_button", m_doneButton);
    elm_object_signal_emit(m_action_bar, "dim,done,button,signal", "");

    if (m_itemForCompose->getItemComposeMode() == profile_create)
        elm_object_disabled_set(m_doneButton, EINA_TRUE);

    elm_layout_content_set(parent, "autofill_comp_swallow", m_mainLayout);
    m_parent = parent;
    return m_mainLayout;
}

void AutoFillFormComposeView::createInputLayout(Evas_Object* parent, char* fieldName,
                                                        genlistCallbackData* cb_data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* layout = elm_layout_add(parent);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "affcv_item");
    elm_object_part_text_set(layout, "field_name", fieldName);

    Evas_Object* editfield = elm_layout_add(layout);
    elm_layout_file_set(editfield, m_edjFilePath.c_str(), "edit-field");
    evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
    evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

    Evas_Object* entry = elm_entry_add(editfield);
    elm_object_style_set(entry, "entry_style");
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);

    cb_data->user_data = this;
    cb_data->editfield = editfield;
    cb_data->entry = entry;
    cb_data->it = layout;
#if defined(HW_MORE_BACK_KEY)
    eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif
    evas_object_smart_callback_add(entry, "preedit,changed", __entry_changed_cb, cb_data);
    evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, cb_data);
    evas_object_smart_callback_add(entry, "changed", __editfield_changed_cb, editfield);
    evas_object_smart_callback_add(entry, "activated", __entry_next_key_cb, cb_data);
    evas_object_smart_callback_add(entry, "clicked", __entry_clicked_cb, cb_data);
    elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_NEXT);
    elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &(m_entryLimitSize));

    elm_object_part_content_set(editfield, "editfield_entry", entry);

    Evas_Object *button = elm_button_add(editfield);
    elm_object_style_set(button, "basic_button");
    evas_object_smart_callback_add(button, "clicked", __entry_clear_button_clicked_cb, entry);
    elm_object_part_content_set(editfield, "entry_clear_button", button);

    if (!elm_entry_is_empty(entry)) {
        BROWSER_LOGE("entry is empty");
        elm_object_signal_emit(editfield, "show,clear,button,signal", "");
    }

    elm_object_part_content_set(layout, "entry_swallow", editfield);
    evas_object_show(layout);
}

void AutoFillFormComposeView::addItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    // full name
    m_fullNameItemCallbackData.type = profile_composer_title_full_name;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_FULL_NAME_ABB")), &m_fullNameItemCallbackData);
    elm_box_pack_end(m_box, m_fullNameItemCallbackData.it);
    if (m_itemForCompose->getName() && strlen(m_itemForCompose->getName()))
        elm_object_part_text_set(m_fullNameItemCallbackData.entry, "elm.text", m_itemForCompose->getName());

    // company_name
    m_companyNameItemCallbackData.type = profile_composer_title_company_name;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_COMPANY_NAME_ABB")), &m_companyNameItemCallbackData);
    elm_box_pack_end(m_box, m_companyNameItemCallbackData.it);
    if (m_itemForCompose->getCompany() && strlen(m_itemForCompose->getCompany()))
        elm_object_part_text_set(m_companyNameItemCallbackData.entry, "elm.text", m_itemForCompose->getCompany());

    // address 1
    m_addressLine1ItemCallbackData.type = profile_composer_title_address_line_1;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_ADDRESS_LINE_1_ABB")), &m_addressLine1ItemCallbackData);
    elm_box_pack_end(m_box, m_addressLine1ItemCallbackData.it);
    if (m_itemForCompose->getPrimaryAddress() && strlen(m_itemForCompose->getPrimaryAddress()))
        elm_object_part_text_set(m_addressLine1ItemCallbackData.entry, "elm.text", m_itemForCompose->getPrimaryAddress());

    // address 2
    m_addressLine2ItemCallbackData.type = profile_composer_title_address_line_2;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_ADDRESS_LINE_2_ABB")), &m_addressLine2ItemCallbackData);
    elm_box_pack_end(m_box, m_addressLine2ItemCallbackData.it);
    if (m_itemForCompose->getSecondaryAddress2() && strlen(m_itemForCompose->getSecondaryAddress2()))
        elm_object_part_text_set(m_addressLine2ItemCallbackData.entry, "elm.text", m_itemForCompose->getSecondaryAddress2());

    // city town
    m_cityTownItemCallbackData.type = profile_composer_title_city_town;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_CITY_TOWN_ABB")), &m_cityTownItemCallbackData);
    elm_box_pack_end(m_box, m_cityTownItemCallbackData.it);
    if (m_itemForCompose->getCityTown() && strlen(m_itemForCompose->getCityTown()))
        elm_object_part_text_set(m_cityTownItemCallbackData.entry, "elm.text", m_itemForCompose->getCityTown());

    // country
    m_countryItemCallbackData.type = profile_composer_title_country;
    createInputLayout(m_box, strdup("Country"), &m_countryItemCallbackData);
    elm_box_pack_end(m_box, m_countryItemCallbackData.it);
    if (m_itemForCompose->getCountry() && strlen(m_itemForCompose->getCountry()))
        elm_object_part_text_set(m_countryItemCallbackData.entry, "elm.text", m_itemForCompose->getCountry());

    // post code
    m_postCodeItemCallbackData.type = profile_composer_title_post_code;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_POSTCODE_ABB")), &m_postCodeItemCallbackData);
    elm_box_pack_end(m_box, m_postCodeItemCallbackData.it);
    Elm_Entry_Filter_Limit_Size m_entryLimitSize;
    Elm_Entry_Filter_Accept_Set m_entry_accept_set;
    m_entryLimitSize.max_char_count = 10;
    m_entry_accept_set.accepted = "0123456789";
    m_entry_accept_set.rejected = NULL;
    elm_entry_markup_filter_append(m_postCodeItemCallbackData.entry, elm_entry_filter_limit_size, &m_entryLimitSize);
    elm_entry_markup_filter_append(m_postCodeItemCallbackData.entry, elm_entry_filter_accept_set, &m_entry_accept_set);
    if (m_itemForCompose->getPostCode() && strlen(m_itemForCompose->getPostCode()))
        elm_object_part_text_set(m_postCodeItemCallbackData.entry, "elm.text", m_itemForCompose->getPostCode());
    elm_entry_input_panel_layout_set(m_postCodeItemCallbackData.entry, ELM_INPUT_PANEL_LAYOUT_NUMBERONLY);
    elm_entry_prediction_allow_set(m_postCodeItemCallbackData.entry, EINA_FALSE);

    // country region
    m_countryRegionItemCallbackData.type = profile_composer_title_country_region;
    createInputLayout(m_box, strdup(_("IDS_BR_MBODY_COUNTRY_REGION")), &m_countryRegionItemCallbackData);
    elm_box_pack_end(m_box, m_countryRegionItemCallbackData.it);
    if (m_itemForCompose->getStateProvince() && strlen(m_itemForCompose->getStateProvince()))
        elm_object_part_text_set(m_countryRegionItemCallbackData.entry, "elm.text", m_itemForCompose->getStateProvince());

    // phone
    m_phoneItemCallbackData.type = profile_composer_title_phone;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_PHONE")), &m_phoneItemCallbackData);
    elm_box_pack_end(m_box, m_phoneItemCallbackData.it);
    if (m_itemForCompose->getPhoneNumber() && strlen(m_itemForCompose->getPhoneNumber()))
        elm_object_part_text_set(m_phoneItemCallbackData.entry, "elm.text", m_itemForCompose->getPhoneNumber());
    Elm_Entry_Filter_Accept_Set entry_accept_set;
    entry_accept_set.accepted = PHONE_FIELD_VALID_ENTRIES;
    entry_accept_set.rejected = NULL;
    elm_entry_markup_filter_append(m_phoneItemCallbackData.entry, elm_entry_filter_accept_set, &entry_accept_set);
    elm_entry_input_panel_layout_set(m_phoneItemCallbackData.entry, ELM_INPUT_PANEL_LAYOUT_PHONENUMBER);
    elm_entry_prediction_allow_set(m_phoneItemCallbackData.entry, EINA_FALSE);

    // email
    m_emailItemCallbackData.type = profile_composer_title_email;
    createInputLayout(m_box, strdup(_("IDS_BR_OPT_SENDURLVIA_EMAIL")), &m_emailItemCallbackData);
    elm_box_pack_end(m_box, m_emailItemCallbackData.it);
    if (m_itemForCompose->getEmailAddress() && strlen(m_itemForCompose->getEmailAddress()))
        elm_object_part_text_set(m_emailItemCallbackData.entry, "elm.text", m_itemForCompose->getEmailAddress());
    elm_entry_input_panel_return_key_type_set(m_emailItemCallbackData.entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
    evas_object_smart_callback_add(m_emailItemCallbackData.entry, "activated", __done_button_cb, this);
    elm_entry_input_panel_layout_set(m_emailItemCallbackData.entry, ELM_INPUT_PANEL_LAYOUT_EMAIL);
    elm_entry_prediction_allow_set(m_emailItemCallbackData.entry, EINA_FALSE);
}

Evas_Object *AutoFillFormComposeView::createScroller(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *scroller = elm_scroller_add(parent);
    if (!scroller) {
        BROWSER_LOGE("elm_scroller_add failed");
        return nullptr;
    }

    m_box = elm_box_add(scroller);
    evas_object_size_hint_weight_set(m_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_align_set(m_box, 0.0, 0.0);
    elm_object_content_set(scroller, m_box);

    addItems();

    return scroller;
}

Eina_Bool AutoFillFormComposeView::isEntryHasOnlySpace(const char* field)
{
    Eina_Bool only_has_space = EINA_FALSE;
    unsigned int space_count = 0;
    unsigned int str_len = strlen(field);

    for (unsigned int i = 0 ; i < str_len ; i++) {
        if (field[i] == ' ')
            space_count++;
    }
    if (space_count == str_len)
        only_has_space = EINA_TRUE;

    return only_has_space;
}

Eina_Bool AutoFillFormComposeView::applyEntryData(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    const char *full_name = elm_entry_entry_get(m_fullNameItemCallbackData.entry);

    if (!full_name)
        return EINA_FALSE;

    std::string full_name_str = std::string(full_name);
    full_name_str = _trim(full_name_str);
    full_name = full_name_str.c_str();

    if (full_name && strlen(full_name) && !isEntryHasOnlySpace(full_name))
        m_itemForCompose->setName(full_name);
    else {
        elm_object_focus_set(m_cancelButton, EINA_TRUE); // Closing virtual keyboard by changing the focus*/
        return EINA_FALSE;
    }
    const char *company_name = elm_entry_entry_get(m_companyNameItemCallbackData.entry);
    m_itemForCompose->setCompany(company_name);
    const char *primary_address = elm_entry_entry_get(m_addressLine1ItemCallbackData.entry);
    m_itemForCompose->setPrimaryAddress(primary_address);
    const char *secondary_address = elm_entry_entry_get(m_addressLine2ItemCallbackData.entry);
    m_itemForCompose->setSecondaryAddress2(secondary_address);
    const char *city_town = elm_entry_entry_get(m_cityTownItemCallbackData.entry);
    m_itemForCompose->setCityTown(city_town);
    const char *country = elm_entry_entry_get(m_countryItemCallbackData.entry);
    m_itemForCompose->setCountry(country);
    const char *post_code = elm_entry_entry_get(m_postCodeItemCallbackData.entry);
    m_itemForCompose->setPostCode(post_code);
    const char *region = elm_entry_entry_get(m_countryRegionItemCallbackData.entry);
    m_itemForCompose->setStateProvince(region);
    const char *phone = elm_entry_entry_get(m_phoneItemCallbackData.entry);
    m_itemForCompose->setPhoneNumber(phone);
    const char *email = elm_entry_entry_get(m_emailItemCallbackData.entry);
    m_itemForCompose->setEmailAddress(email);

    if (m_itemForCompose->getItemComposeMode() == profile_edit) {

        m_editErrorcode = m_itemForCompose->updateItem();
        if (m_editErrorcode == profile_edit_failed || m_editErrorcode == profile_already_exist) {
            BROWSER_LOGD("Update failed!");
            return EINA_FALSE;
        }
    } else {
        m_saveErrorcode = m_itemForCompose->saveItem();
        if (m_saveErrorcode != profile_create_failed && m_saveErrorcode != duplicate_profile)
            m_manager->addItemToList(m_itemForCompose);
    }

    return EINA_TRUE;
}

void AutoFillFormComposeView::__editfield_changed_cb(void* data, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *editfield = static_cast<Evas_Object*>(data);

    if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
        elm_object_signal_emit(editfield, "show,clear,button,signal", "");
    else
        elm_object_signal_emit(editfield, "hide,clear,button,signal", "");
}

void AutoFillFormComposeView::__done_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    AutoFillFormComposeView *view = static_cast<AutoFillFormComposeView*>(data);
    if (view->applyEntryData() == EINA_FALSE)
        return;

#if !PROFILE_MOBILE
    elm_object_focus_set(view->m_cancelButton, EINA_TRUE);
    evas_object_hide(view->m_mainLayout);
#endif
    view->m_manager->refreshListView();
    view->hide_action_bar();
}

void AutoFillFormComposeView::__cancel_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    AutoFillFormComposeView *view = static_cast<AutoFillFormComposeView*>(data);
    view->hide_action_bar();
}

void AutoFillFormComposeView::hide_action_bar()
{
    elm_object_signal_emit(m_action_bar, "hide,buttons,signal", "but_vis");
    elm_object_signal_emit(m_action_bar, "show,close,icon", "del_but");
    elm_object_signal_emit(m_action_bar, "back,icon,change", "del_but");
    elm_object_translatable_part_text_set(m_action_bar, "settings_title", "IDS_BR_BODY_AUTO_FILL_FORMS_T_TTS");
    hide();
}

void AutoFillFormComposeView::__entry_changed_cb(void* data, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *cb_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormComposeView *view = static_cast<AutoFillFormComposeView*>(cb_data->user_data);
    const char* text = elm_entry_entry_get(obj);
    if (text && strlen(text) > 0) {
        elm_object_signal_emit(cb_data->editfield, "show,clear,button,signal", "");
    }
    else {
        elm_object_signal_emit(cb_data->editfield, "hide,clear,button,signal", "");
    }

    if (!elm_entry_is_empty(view->m_fullNameItemCallbackData.entry)) {
        elm_object_signal_emit(view->m_action_bar, "show,buttons,signal", "but_vis");
        elm_object_disabled_set(view->m_doneButton, EINA_FALSE);
    } else {
        elm_object_signal_emit(view->m_action_bar, "dim,done,button,signal", "but_vis");
        elm_object_disabled_set(view->m_doneButton, EINA_TRUE);
    }

}

void AutoFillFormComposeView::__entry_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    Elm_Object_Item *item = callback_data->it;
    Evas_Object *entry = elm_object_item_part_content_get(item, "entry_swallow");
    elm_object_focus_set(entry, EINA_TRUE);
}

void AutoFillFormComposeView::__entry_clear_button_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *entry = static_cast<Evas_Object*>(data);
    elm_entry_entry_set(entry, "");
}

void AutoFillFormComposeView::__entry_next_key_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormComposeView *self = static_cast<AutoFillFormComposeView*>(callback_data->user_data);
    AutoFillFormComposeView::menu_type type = callback_data->type;
    Evas_Object *entry = nullptr;

    if (type == profile_composer_title_full_name) {
        entry = self->m_companyNameItemCallbackData.entry;
    } else if (type == profile_composer_title_company_name) {
        entry = self->m_addressLine1ItemCallbackData.entry;
    } else if (type == profile_composer_title_address_line_1) {
        entry = self->m_addressLine2ItemCallbackData.entry;
    } else if (type == profile_composer_title_address_line_2) {
        entry = self->m_cityTownItemCallbackData.entry;
    } else if (type == profile_composer_title_city_town) {
        entry = self->m_countryItemCallbackData.entry;
    } else if (type == profile_composer_title_country) {
        entry = self->m_postCodeItemCallbackData.entry;
    } else if (type == profile_composer_title_post_code) {
        entry = self->m_countryRegionItemCallbackData.entry;
    } else if (type == profile_composer_title_country_region) {
        entry = self->m_phoneItemCallbackData.entry;
    } else if (type == profile_composer_title_phone) {
        entry = self->m_emailItemCallbackData.entry;
    } else if (type == profile_composer_title_email) {
        BROWSER_LOGD("[%s:%d] It's last item to go", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    elm_object_focus_set(entry, EINA_TRUE);
    elm_entry_cursor_end_set(entry);
}

void AutoFillFormComposeView::hide()
{
    if (m_manager)
        m_manager->deleteComposer();
}

}
}
