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

#ifndef SETTINGSMANAGER_H_
#define SETTINGSMANAGER_H_

#include "AbstractService.h"
#include "service_macros.h"
#include "SettingsUI.h"
#include "SettingsMain.h"
#include "SettingsHomePage.h"
#include "SettingsPrivacy.h"
#include "SettingsAdvanced.h"
#include "SettingsDelPersData.h"
#include "SettingsAFProfile.h"
#include "SettingsAFCreator.h"
#include "SettingsUserAgent.h"
#include "service_macros.h"
#include <string>
#include <map>
#include <Evas.h>
#include <Elementary.h>
#include <memory>

namespace tizen_browser{
namespace base_ui{

using SetPtr = std::shared_ptr<SettingsUI>;
class BROWSER_EXPORT SettingsManager
    : public tizen_browser::core::AbstractService
{
public:
    SettingsManager();
    ~SettingsManager();
    void init(Evas_Object* parent);
    SetPtr& getView(const SettingsMainOptions& s);
    void connectOpenSignals();
    std::shared_ptr<SettingsUI>& addView(const SettingsMainOptions& s);
    std::string getName();
    SetPtr operator[](const SettingsMainOptions& s){ return m_settingsViews[s];};
    void showSettingsBaseUI();
    void showSettingsHomePageUI();
    void showSettingsAutofillUI();
    void showSettingsAutofillCreatorUI(bool);
    void showSettingsPrivacyUI();
    void showSettingsUserAgentUI();
    void showSettingsAdvancedUI();
    void showSettingsDelPrivDataUI();

private:
    std::map<SettingsMainOptions,SetPtr> m_settingsViews;
    Evas_Object* m_parent;
};
}
}

#endif /* SETTINGSMANAGER_H_ */
