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

#ifndef SETTINGSADVANCED_MOB_H_
#define SETTINGSADVANCED_MOB_H_

#include "SettingsUI.h"

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <Evas.h>
#include <vconf.h>
#include "BrowserLogger.h"
#include "../SimpleUI/RadioPopup.h"
#include "Tools/EflTools.h"
#include "Tools/SettingsEnums.h"

namespace tizen_browser{
namespace base_ui{

enum SettingsAdvancedOptions
{
    ENABLE_JS,
    BLOCK_POPUPS,
    SAVE_CONTENT,
};

enum struct SettingsStorageType : int
{
    DEVICE = 0,
    SD_CARD
};

class SettingsAdvanced
    : public SettingsUI
{
public:
    SettingsAdvanced(Evas_Object* parent);
    virtual ~SettingsAdvanced();
    bool populateList(Evas_Object* genlist) override;
    void updateButtonMap() override;
    Evas_Object* createOnOffCheckBox(Evas_Object* obj, ItemData*) override;
    Eina_Bool getOriginalState(int id);
    void changeGenlistStorage();
    static void _realized_cb(void* data, Evas_Object*, void* event_info);
    static void _check_cb(void* data, Evas_Object* obj, void* event_info);
    static void _enable_js_cb(void *data, Evas_Object*obj , void* event_info);
    static void _block_popups_cb(void *data, Evas_Object*obj , void* event_info);
    static void _save_content_cb(void *data, Evas_Object*obj , void* event_info);
    static void grid_item_check_changed(void *data, Evas_Object *obj, void *event_info);
    static void notifyStorageChange(keynode_t *key, void* data);
    void setContentDestination(int button);
    bool setStorageType(SettingsStorageType type);
private:
    RadioPopup* m_popup;
};

}
}

#endif /* SETTINGSADVANCED_MOB_H_ */
