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

#ifndef AUTOFILLFORMITEM_H_
#define AUTOFILLFORMITEM_H_

#include <Eina.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

#define AUTO_FILL_FORM_ENTRY_MAX_COUNT 1024
#define PHONE_FIELD_VALID_ENTRIES "0123456789*#()/N,.;+ "

namespace tizen_browser{
namespace base_ui{

typedef enum _profileComposeMode
{
    profile_create = 0,
    profile_edit
} profileComposeMode;

typedef enum _profileEditErrorcode
{
    profile_edit_failed = 0,
    profile_already_exist,
    update_error_none
} profileEditErrorcode;

typedef enum _profileSaveErrorcode
{
    profile_create_failed = 0,
    duplicate_profile,
    save_error_none
} profileSaveErrorcode;

typedef struct _AutoFillFormItemData {
    unsigned profile_id;
    const char *name;
    const char *company;
    const char *primary_address;
    const char *secondary_address;
    const char *city_town;
    const char *state_province_region;
    const char *post_code;
    const char *country;
    const char *phone_number;
    const char *email_address;
    bool activation;
    profileComposeMode compose_mode;
} AutoFillFormItemData;

class AutoFillFormItem {
public:
    AutoFillFormItem(AutoFillFormItemData *item_data);
    ~AutoFillFormItem(void);

    friend bool operator==(AutoFillFormItem item1, AutoFillFormItem item2) {
    return ((!item1.getName() && !item2.getName()) && !strcmp(item1.getName(), item2.getName()));
    }

    unsigned getProfileId(void) { return m_itemData.profile_id; }
    const char *getName(void) { return m_itemData.name; }
    const char *getCompany(void) {return m_itemData.company; }
    const char *getPrimaryAddress(void) {return m_itemData.primary_address; }
    const char *getSecondaryAddress2(void) { return m_itemData.secondary_address; }
    const char *getCityTown(void) { return m_itemData.city_town; }
    const char *getStateProvince(void) { return m_itemData.state_province_region; }
    const char *getPostCode(void) { return m_itemData.post_code; }
    const char *getCountry(void) { return m_itemData.country; }
    const char *getPhoneNumber(void) { return m_itemData.phone_number; }
    const char *getEmailAddress(void) { return m_itemData.email_address; }
    Eina_Bool getActivation(void) { return (m_itemData.activation == true) ? EINA_TRUE : EINA_FALSE; }
    profileComposeMode getItemComposeMode(void) { return m_itemData.compose_mode; }

    void setName(const char *name) { m_itemData.name = name; }
    void setCompany(const char *company) {m_itemData.company = company; }
    void setPrimaryAddress(const char *primary_address) {m_itemData.primary_address = primary_address; }
    void setSecondaryAddress2(const char *secondary_address) { m_itemData.secondary_address = secondary_address; }
    void setCityTown(const char *city_town) { m_itemData.city_town = city_town; }
    void setStateProvince(const char *state_province_region) { m_itemData.state_province_region = state_province_region; }
    void setPostCode(const char *post_code) { m_itemData.post_code = post_code; }
    void setCountry(const char *country) { m_itemData.country = country; }
    void setPhoneNumber(const char *phone_number) { m_itemData.phone_number = phone_number; }
    void setEmailAddress(const char *email_address) { m_itemData.email_address = email_address; }
    void setActivation(Eina_Bool activation) { m_itemData.activation = (activation == EINA_TRUE) ? true : false;}

    profileSaveErrorcode saveItem(void);
    profileEditErrorcode updateItem(void);

private:
    AutoFillFormItemData m_itemData;
    Ewk_Context* m_ewkContext;
};

}
}

#endif /* AUTOFILLFORMITEM_H_ */
