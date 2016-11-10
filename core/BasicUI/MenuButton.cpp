/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
 *
 *
 */

#include "MenuButton.h"

#include "BrowserLogger.h"

namespace tizen_browser
{
namespace base_ui
{

MenuButton * MenuButton::previousPopup = 0;

MenuButton::MenuButton(std::shared_ptr< Evas_Object > mainWindow, Evas_Object* parentButton)
    : m_ctxPopup(0)
    , m_window(mainWindow)
    , m_parentButton(parentButton)
    , m_isShown(false)
{

}

MenuButton::~MenuButton()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hidePopup();
}


void MenuButton::showPopup()
{
    BROWSER_LOGD("[%s:%d] this: %x ", __PRETTY_FUNCTION__, __LINE__, this);
    hidePopup();

    if (isShown()){
        m_isShown = false;
        unbindFocus();
        evas_object_hide(m_ctxPopup);
        onPopupDismissed();
        return;
    }

    if(!m_ctxPopup) {
        m_ctxPopup = elm_ctxpopup_add(m_window.get());
        BROWSER_LOGD("[%s:%d] - new popup: %x ", __PRETTY_FUNCTION__, __LINE__, m_ctxPopup);
        evas_object_smart_callback_add(m_ctxPopup, "dismissed", dismissedCtxPopup, this);
        elm_object_content_set(m_ctxPopup, getContent());
        elm_ctxpopup_direction_priority_set(m_ctxPopup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN);

        Evas_Coord w,h,x,y;
        evas_object_geometry_get(m_window.get(), &x, &y, &w, &h);
        evas_object_size_hint_max_set(m_ctxPopup, w, h);
        evas_object_move(m_ctxPopup, 1650, 130);

        elm_object_style_set(m_ctxPopup, "message_popup");
    }
    realShow(m_ctxPopup);
    previousPopup = this;
}

void MenuButton::realShow(Evas_Object* popup)
{
    elm_object_focus_next_object_set(m_parentButton, getFirstFocus(), ELM_FOCUS_DOWN);
    elm_object_focus_next_object_set(getFirstFocus(), m_parentButton, ELM_FOCUS_UP);

    ListSize listSize = calculateSize();
    evas_object_size_hint_min_set(m_ctxPopup, listSize.width, listSize.height);
    evas_object_size_hint_max_set(m_ctxPopup, listSize.width, listSize.height);

    evas_object_show(popup);
    m_isShown=true;
    onPopupShow();
}

void MenuButton::unbindFocus()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_focus_next_object_set(m_parentButton, NULL, ELM_FOCUS_DOWN);
}

bool MenuButton::isShown()
{
    return m_isShown;
}

void MenuButton::hidePopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(previousPopup && previousPopup->isShown() && previousPopup->canBeDismissed()) {
        elm_ctxpopup_dismiss(previousPopup->m_ctxPopup);
        return;
    }
    elm_ctxpopup_dismiss(m_ctxPopup);
    m_isShown = false;
}


void MenuButton::dismissedCtxPopup(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d]: data*: %x", __PRETTY_FUNCTION__, __LINE__, data);
    MenuButton* self = reinterpret_cast<MenuButton*>(data);
    self->m_isShown = false;
    self->unbindFocus();
    evas_object_hide(self->m_ctxPopup);
    self->onPopupDismissed();
}


void MenuButton::onPopupShow()
{
    //default implementation does nothing
}

void MenuButton::onPopupDismissed()
{
    //default implementation does nothing
}

bool MenuButton::canBeDismissed()
{
    return true;
}

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */
