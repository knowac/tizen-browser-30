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

#ifndef SETTINGSMAIN_MOB_H_
#define SETTINGSMAIN_MOB_H_

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

enum SettingsMainOptions {
    BASE,
    HOME,
    SEARCH,
    AUTO_FILL_PROFILE,
    AUTO_FILL_CREATOR_WITHOUT_PROFILE,
    AUTO_FILL_CREATOR_WITH_PROFILE,
    PRIVACY,
    ZOOM,
    ADVANCED,
    DEL_PERSONAL_DATA
};

class SettingsMain
    : public SettingsUI
{
public:
    SettingsMain(){};
    SettingsMain(Evas_Object* parent);
    virtual ~SettingsMain();
    virtual bool populateList(Evas_Object* genlist) override;
    Evas_Object* createOnOffCheckBox(Evas_Object* obj, ItemData* itd);
    Eina_Bool getOriginalZoomState();
    std::string getHomePage();
    virtual void updateButtonMap() override;
    virtual void connectSignals() override {};
    virtual void disconnectSignals() override {};
    static void _home_page_cb(void *data, Evas_Object*obj , void* event_info);
    static void _search_engine_cb(void *data, Evas_Object*obj , void* event_info);
    static void _zoom_cb(void *data, Evas_Object*obj , void* event_info);
    static void _advanced_cb(void *data, Evas_Object*obj , void* event_info);
    static void _auto_fill_cb(void* data, Evas_Object* obj, void* event_info);
    static void _privacy_cb(void* data, Evas_Object* obj, void * event_info);
    static void grid_item_check_changed(void* data, Evas_Object* obj, void*);
    void setSearchEngineSubText(int button);
    void setHomePageSubText();
};

}
}

#endif /* SETTINGSMAIN_MOB_H_ */
