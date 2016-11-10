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

#ifndef SETTINGSAFPROFILE_MOB_H_
#define SETTINGSAFPROFILE_MOB_H_

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

#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace tizen_browser{
namespace base_ui{

class SettingsAFProfile
    : public SettingsUI
{
public:
    SettingsAFProfile(){};
    SettingsAFProfile(Evas_Object* parent);
    virtual ~SettingsAFProfile();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    static void _select_profile_cb(void* data, Evas_Object*, void*);
protected:
    Ewk_Autofill_Profile* m_profile;
    std::shared_ptr<ItemData> m_itemData;
    std::string m_profileName;
};

}
}
#endif /* SETTINGSAFPROFILE_MOB_H_ */
