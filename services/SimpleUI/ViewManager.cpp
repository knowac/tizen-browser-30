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

/*
 * ViewManager.cpp
 *
 *  Created on: Sep 11, 2015
 *      Author: m.lapinski@samsung.com
 */

#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <string>

#include "ViewManager.h"
#include "core/BrowserLogger.h"
#include "core/ServiceManager/Debug/BrowserAssert.h"

namespace tizen_browser{
namespace base_ui{

ViewManager::ViewManager()
   : m_mainLayout(nullptr)
   , m_conformant(nullptr)
   , m_parentWindow(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void ViewManager::init(Evas_Object* parentWindow)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parentWindow);

    m_conformant = elm_conformant_add(parentWindow);
     //tmp for TSAM-5664 rotation resize issue
    //elm_win_indicator_mode_set(parentWindow, ELM_WIN_INDICATOR_SHOW);
    evas_object_size_hint_weight_set(m_conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(m_conformant);
    elm_win_resize_object_add(parentWindow, m_conformant);

    Evas_Object* bx = elm_box_add(parentWindow);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_mainLayout = elm_layout_add(bx);
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_mainLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_mainLayout);
    elm_box_pack_end(bx, m_mainLayout);

    Eina_Bool ret = elm_layout_file_set(m_mainLayout,
                                        (std::string(EDJE_DIR)
                                        + std::string("SimpleUI/ViewManager.edj")).c_str(),
                                        "main_layout");
    if (!ret)
        BROWSER_LOGD("[%s:%d]  elm_layout_file_set falied !!!",__PRETTY_FUNCTION__, __LINE__);

    elm_object_content_set(m_conformant, bx);
}

ViewManager::~ViewManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_del(m_mainLayout);
}

void ViewManager::popStackTo(interfaces::AbstractUIComponent* view)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(view);
    interfaces::AbstractUIComponent* previousView = m_viewStack.top();

    while(!m_viewStack.empty() && m_viewStack.top() != view)
    {
        m_viewStack.pop();
    }
    updateLayout(previousView);
    BROWSER_LOGD("[%s:%d] new top: %p", __PRETTY_FUNCTION__, __LINE__, topOfStack());
}

void ViewManager::popTheStack()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_viewStack.empty())
    {
        interfaces::AbstractUIComponent* previousView = m_viewStack.top();
        m_viewStack.pop();
        updateLayout(previousView);
    }
    BROWSER_LOGD("[%s:%d] new top: %p", __PRETTY_FUNCTION__, __LINE__, topOfStack());
}

void ViewManager::pushViewToStack(interfaces::AbstractUIComponent* view)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(view);
    if (topOfStack() == view)
    {
       BROWSER_LOGD("[%s:%d] View %p is already on stack !!!",
                          __PRETTY_FUNCTION__, __LINE__, view);
       return;
    }
    interfaces::AbstractUIComponent* previousView = topOfStack();
    m_viewStack.push(view);
    updateLayout(previousView);
    BROWSER_LOGD("[%s:%d] new top: %p", __PRETTY_FUNCTION__, __LINE__, topOfStack());
}


void ViewManager::updateLayout(interfaces::AbstractUIComponent* previousView)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* swallowed = elm_layout_content_get(m_mainLayout, "content");
    if (!m_viewStack.empty())
    {
        if (topOfStack()->getContent() == swallowed)
        {
            BROWSER_LOGD("[%s:%d] Top of stack is already visible!!!",
                         __PRETTY_FUNCTION__, __LINE__);
            return;
        }
        elm_layout_content_unset(m_mainLayout, "content");
        if (previousView)
        {
            previousView->hideUI();
            evas_object_hide(previousView->getContent());
        }
        elm_layout_content_set(m_mainLayout, "content", topOfStack()->getContent());
        evas_object_show(elm_layout_content_get(m_mainLayout, "content"));

        topOfStack()->showUI();
    }
    else
    {
        BROWSER_LOGD("[%s:%d] Stack is empty!!!",__PRETTY_FUNCTION__, __LINE__);
        elm_layout_content_unset(m_mainLayout, "content");
        if (previousView)
        {
             previousView->hideUI();
             evas_object_hide(previousView->getContent());
        }
        elm_layout_content_set(m_mainLayout, "content", nullptr);
    }
}

Evas_Object* ViewManager::getContent()
{
    M_ASSERT(m_mainLayout);
    return m_mainLayout;
}

Evas_Object* ViewManager::getConformant() {
    return m_conformant;
}

interfaces::AbstractUIComponent* ViewManager::topOfStack()
{
    if(!m_viewStack.empty())
        return m_viewStack.top();
    else
        return nullptr;
}


}//namespace base_ui
}//names1pace tizen_browser
