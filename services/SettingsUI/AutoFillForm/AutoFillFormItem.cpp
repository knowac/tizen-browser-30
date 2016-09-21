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

#include "AutoFillFormItem.h"
#include "BrowserLogger.h"
#include "../SettingsUI.h"

#include <Ecore.h>
#include <Elementary.h>
#include <Evas.h>

namespace tizen_browser{
namespace base_ui{

AutoFillFormItem::AutoFillFormItem(AutoFillFormItemData *item_data)
    : m_ewkContext(ewk_context_default_get())
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    memset(&m_itemData, 0x00, sizeof(m_itemData));
    m_itemData.profile_id = -1;

    if (item_data) {
        m_itemData.profile_id= item_data->profile_id;
        m_itemData.name = item_data->name;
        m_itemData.company = item_data->company;
        m_itemData.primary_address = item_data->primary_address;
        m_itemData.secondary_address = item_data->secondary_address;
        m_itemData.city_town = item_data->city_town;
        m_itemData.state_province_region = item_data->state_province_region;
        m_itemData.post_code = item_data->post_code;
        m_itemData.country = item_data->country;
        m_itemData.phone_number = item_data->phone_number;
        m_itemData.email_address = item_data->email_address;
        m_itemData.activation = item_data->activation;
        m_itemData.compose_mode = item_data->compose_mode;
    }
}

AutoFillFormItem::~AutoFillFormItem(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

profileSaveErrorcode AutoFillFormItem::saveItem(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto profile = ewk_autofill_profile_new();
    if (!profile) {
        BROWSER_LOGE("Failed to ewk_autofill_profile_new");
        return profile_create_failed;
    }

    if (m_itemData.name)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_NAME, m_itemData.name);

    if (m_itemData.company)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_COMPANY, m_itemData.company);

    if (m_itemData.primary_address)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS1, m_itemData.primary_address);

    if (m_itemData.secondary_address)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS2, m_itemData.secondary_address);

    if (m_itemData.city_town)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_CITY_TOWN, m_itemData.city_town);

    if (m_itemData.state_province_region)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_STATE_PROVINCE_REGION, m_itemData.state_province_region);

    if (m_itemData.post_code)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_ZIPCODE, m_itemData.post_code);

    if (m_itemData.country)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_COUNTRY, m_itemData.country);

    if (m_itemData.phone_number)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_PHONE, m_itemData.phone_number);

    if (m_itemData.email_address)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_EMAIL, m_itemData.email_address);

    if (ewk_context_form_autofill_profile_add(m_ewkContext, profile) == EINA_FALSE) {
        BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_add");
        ewk_autofill_profile_delete(profile);
        return duplicate_profile;
    }
    m_itemData.profile_id = ewk_autofill_profile_id_get(profile);
    ewk_autofill_profile_delete(profile);

    return save_error_none;
}

profileEditErrorcode AutoFillFormItem::updateItem(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    /* Find profile with id */
    Ewk_Autofill_Profile *profile = ewk_context_form_autofill_profile_get(m_ewkContext, m_itemData.profile_id);
    if (!profile) {
        BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_get with ID [%d]", m_itemData.profile_id);
        return profile_edit_failed;
    }

    if (m_itemData.name)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_NAME, m_itemData.name);
    if (m_itemData.company)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_COMPANY, m_itemData.company);
    if (m_itemData.primary_address)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS1, m_itemData.primary_address);
    if (m_itemData.secondary_address)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS2, m_itemData.secondary_address);
    if (m_itemData.city_town)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_CITY_TOWN, m_itemData.city_town);
    if (m_itemData.state_province_region)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_STATE_PROVINCE_REGION, m_itemData.state_province_region);
    if (m_itemData.post_code)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_ZIPCODE, m_itemData.post_code);
    if (m_itemData.country)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_COUNTRY, m_itemData.country);
    if (m_itemData.phone_number)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_PHONE, m_itemData.phone_number);
    if (m_itemData.email_address)
        ewk_autofill_profile_data_set(profile, EWK_PROFILE_EMAIL, m_itemData.email_address);

    if (ewk_context_form_autofill_profile_set(m_ewkContext, m_itemData.profile_id, profile) == EINA_FALSE) {
        BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_set with ID [%d]", m_itemData.profile_id);
        return profile_already_exist;
    }

    return update_error_none;
}

}
}
