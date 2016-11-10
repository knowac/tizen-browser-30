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

#ifndef GENLISTMANAGERCALLBACKS_H_
#define GENLISTMANAGERCALLBACKS_H_

#include "GenlistManager.h"
#include <Elementary.h>
#include <Evas.h>

namespace tizen_browser {
namespace base_ui {

class GenlistManagerCallbacks
{
public:
    GenlistManagerCallbacks();
    virtual ~GenlistManagerCallbacks();

    /**
     * Handles keyboard events: up and down keys.
     */
    static Eina_Bool _object_event(void* data, Evas_Object* obj,
            Evas_Object* src, Evas_Callback_Type type, void* event_info);
    static void _item_selected(void* data, Evas_Object* obj, void* event_info);
    static void setGenlistManager(GenlistManager* genlistManager)
    {
        GenlistManagerCallbacks::genlistManager = genlistManager;
    }
private:
    static GenlistManager* genlistManager;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* GENLISTMANAGERCALLBACKS_H_ */
