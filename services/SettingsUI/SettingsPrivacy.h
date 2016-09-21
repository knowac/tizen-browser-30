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

#ifndef SETTINGSPRIVACY_MOB_H_
#define SETTINGSPRIVACY_MOB_H_

#include "SettingsUI.h"

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <Evas.h>
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "Tools/SettingsEnums.h"

namespace tizen_browser{
namespace base_ui{

enum SettingsPrivacyOptions {
    COOKIES,
    SIGNIN_INFO,
    DEL_PER_DATA
};

class SettingsPrivacy
    : public SettingsUI
{
public:
    SettingsPrivacy(){};
    SettingsPrivacy(Evas_Object* parent);
    virtual ~SettingsPrivacy();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    static void _cookies_cb(void *data, Evas_Object*obj , void* event_info);
    static void _signin_cb(void *data, Evas_Object*obj , void* event_info);
    static void _del_per_data_cb(void *data, Evas_Object*obj , void* event_info);
};

}
}

#endif /* SETTINGSPRIVACY_MOB_H_ */
