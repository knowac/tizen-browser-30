/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include "FocusManager.h"
#include "BrowserLogger.h"

FocusManager::FocusManager()
    : _rowTracker(0)
    , _prevRowTracker(0)
    , handlerDown(nullptr)
    , _gen(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

FocusManager::~FocusManager()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
}

void FocusManager::startFocusManager(Evas_Object* gengrid)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    handlerDown = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down_cb, this);
    if (gengrid)
        evas_object_smart_callback_add(gengrid, "item,focused", _row_tracker, this);
}

void FocusManager::stopFocusManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    verticalFocusVector.clear();
    ecore_event_handler_del(handlerDown);
}
Eina_Bool FocusManager::_key_down_cb(void* data, int, void* event)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    FocusManager* fm = static_cast<FocusManager*>(data);
    Ecore_Event_Key* ev = static_cast<Ecore_Event_Key*>(event);
    const std::string keyName = ev->keyname;

    if (!keyName.compare("Up")) {
        BROWSER_LOGD("[%s:%d] Keyname: %s", __PRETTY_FUNCTION__, __LINE__, ev->keyname);
        if (fm->_prevRowTracker) {
            fm->_prevRowTracker = 0;
            return EINA_FALSE;
        }
        if (fm->focusListIterator == fm->verticalFocusVector.begin()) {
            elm_object_focus_set(*(fm->focusListIterator), EINA_TRUE);
            return EINA_FALSE;
        }
        --(fm->focusListIterator);
        if (elm_object_disabled_get(*(fm->focusListIterator)) == EINA_TRUE)
            --(fm->focusListIterator);
        elm_object_focus_set(*(fm->focusListIterator), EINA_TRUE);
        return EINA_TRUE;
    }
    else if (!keyName.compare("Down")) {
        BROWSER_LOGD("[%s:%d] Keyname: %s", __PRETTY_FUNCTION__, __LINE__, ev->keyname);
        if (fm->focusListIterator == fm->verticalFocusVector.end()-1) {
            elm_object_focus_set(*(fm->focusListIterator), EINA_TRUE);
            return EINA_FALSE;
        }
        ++(fm->focusListIterator);
        if (elm_object_disabled_get(*(fm->focusListIterator)) == EINA_TRUE)
            ++(fm->focusListIterator);
        elm_object_focus_set(*(fm->focusListIterator), EINA_TRUE);
        return EINA_TRUE;
    }
    return EINA_FALSE;
}

void FocusManager::_row_tracker(void* data, Evas_Object*, void* event)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    FocusManager* fm = static_cast<FocusManager*>(data);
    Elm_Object_Item* it = static_cast<Elm_Object_Item*>(event);
    unsigned int x, y;
    elm_gengrid_item_pos_get(it, &x, &y);
    fm->_prevRowTracker = fm->_rowTracker;
    fm->_rowTracker = y;
    BROWSER_LOGD("[%s:%d] prev Y:%d, Actual Y:%d", __PRETTY_FUNCTION__, __LINE__, fm->_prevRowTracker, fm->_rowTracker);
}

void FocusManager::addItem(Evas_Object* it)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    verticalFocusVector.push_back(it);
}

void FocusManager::setIterator()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    focusListIterator = verticalFocusVector.begin();
}
