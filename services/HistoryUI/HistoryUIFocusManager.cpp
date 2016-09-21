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

#include "HistoryUIFocusManager.h"
#include "HistoryDaysListManager/HistoryDaysListManager.h"

namespace tizen_browser{
namespace base_ui{

HistoryUIFocusManager::HistoryUIFocusManager(
        HistoryDaysListManagerPtr historyDaysListManager)
    : m_historyDaysListManager(historyDaysListManager)
{
}

HistoryUIFocusManager::~HistoryUIFocusManager()
{
}

void HistoryUIFocusManager::setFocusObj(Evas_Object* obj)
{
    m_focusObject = obj;
}

#if !PROFILE_MOBILE
void HistoryUIFocusManager::refreshFocusChain()
{
    elm_object_focus_custom_chain_unset(m_focusObject);

    m_historyDaysListManager->setFocusChain(m_focusObject);

    elm_object_focus_custom_chain_append(m_focusObject, m_buttonClear, NULL);
    elm_object_focus_custom_chain_append(m_focusObject, m_buttonClose, NULL);
    elm_object_focus_set(m_buttonClose, EINA_TRUE);
}

void HistoryUIFocusManager::unsetFocusChain()
{
    elm_object_focus_custom_chain_unset(m_focusObject);
}
#endif

void HistoryUIFocusManager::setHistoryUIButtons(Evas_Object* buttonClose, Evas_Object* buttonClear)
{
    m_buttonClose = buttonClose;
    m_buttonClear = buttonClear;
}

}
}
