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

#ifndef SETTINGS_HOME_PAGE_MOB_H_
#define SETTINGS_HOME_PAGE_MOB_H_

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

enum SettingsHomePageOptions {
    DEFAULT,
    CURRENT,
    QUICK_ACCESS,
    MOST_VIS,
    OTHER
};

class SettingsHomePage
    : public SettingsUI
{
public:
    SettingsHomePage(){};
    SettingsHomePage(Evas_Object* parent);
    std::string getCurrentPage();
    virtual ~SettingsHomePage();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    virtual void connectSignals(){};
    virtual void disconnectSignals(){};
    virtual Evas_Object* createRadioButton(Evas_Object* obj, ItemData*);
    void setRadioOnChange();
    static void _default_cb(void *data, Evas_Object*obj , void* event_info);
    static void _current_cb(void *data, Evas_Object*obj , void* event_info);
    static void _quick_cb(void *data, Evas_Object*obj , void* event_info);
    static void _most_visited_cb(void *data, Evas_Object*obj , void* event_info);
    static void _other_cb(void* data, Evas_Object* obj, void* event_info);
    static const std::string DEF_HOME_PAGE;
    static const std::string QUICK_PAGE;
    static const std::string MOST_VISITED_PAGE;
    static const std::string OTHER_PAGE;
    static const std::string CURRENT_PAGE;

private:
    std::map<SettingsHomePageOptions, Elm_Object_Item*> m_itemsMap;
    boost::optional<std::string> m_current;
};

}
}

#endif /* SETTINGS_HOME_PAGE_MOB_H_ */
