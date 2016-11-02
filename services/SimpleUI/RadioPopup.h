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

#ifndef __RADIO_POPUP_H__
#define __RADIO_POPUP_H__ 1

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <memory>

#include "AbstractPopup.h"
#include "app_i18n.h"
#include "Tools/SettingsEnums.h"

namespace tizen_browser
{

namespace base_ui
{

class RadioPopup : public interfaces::AbstractPopup
{
public:
    static std::map<RadioButtons, std::string> createTranslations();
    static RadioButtons translateButtonState(const std::string& name);
    static RadioPopup* createPopup(Evas_Object* parent);

    void show() override;
    void dismiss() override { popupDismissed(this); }
    void onBackPressed() override { dismiss(); }
    void orientationChanged() override {}

    void setTitle(const std::string& title);
    void addRadio(RadioButtons button);
    void setButtons(RadioButtons rb, Evas_Object* button);
    void setState(RadioButtons state);
    Evas_Object* createItem(Evas_Object* parent, RadioButtons button);

    boost::signals2::signal<void (RadioButtons)> radioButtonClicked;

    ~RadioPopup();

private:
    RadioPopup(Evas_Object* parent);

    static void _response_cb(void* data, Evas_Object* obj, void* event_info);
    Evas_Object* addRadioToGenlist(const RadioButtons& button, Evas_Object* obj);

    static std::map<RadioButtons, std::string> s_buttonsTranslations;
    Evas_Object* m_parent;
    std::map<RadioButtons, Evas_Object*> m_buttons;
    std::string m_title;
    Evas_Object* m_popup;
    Evas_Object* m_radioGroup;
    Evas_Object* m_box;
    int m_radioState;
};

}

}

#endif //__RADIO_POPUP_H__
