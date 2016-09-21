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

/*
 * AbstractContextMenu.h
 *
 *  Created on: May 20, 2016
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef __ABSTRACT_CONTEXT_MENU_H__
#define __ABSTRACT_CONTEXT_MENU_H__

#include <efl_extension.h>

namespace tizen_browser
{
namespace interfaces
{

/**
 * @brief This interface defines rotatable object,
 * which reacts on mobile device oriantation change.
 */
class AbstractContextMenu
{
public:
    /**
     * @brief Abstract method for showing context menu.
     */
    virtual void showContextMenu() = 0;

    /**
     * @brief Boost signal, returns main window pointer.
     * Class which implements AbstractContextMenu has to connect this signal in SimpleUI class.
     */
    boost::signals2::signal<Evas_Object* ()> getWindow;

protected:
    static void _cm_dismissed(void *, Evas_Object * obj, void *)
    {
        evas_object_del(obj);
        obj = nullptr;
    }

    /**
     * @brief Method for creating ctxpopup before appending items.
     */
    void createContextMenu(Evas_Object* parent)
    {
        if (m_ctxpopup)
            _cm_dismissed(nullptr, m_ctxpopup, nullptr);

        m_ctxpopup = elm_ctxpopup_add(parent);
        elm_object_style_set(m_ctxpopup, "more/default");

        eext_object_event_callback_add(m_ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, nullptr);
        eext_object_event_callback_add(m_ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, nullptr);
        evas_object_smart_callback_add(m_ctxpopup, "dismissed", _cm_dismissed, nullptr);
    }

    /**
     * @brief Method for aligning ctxpopup after appending items.
     */
    void alignContextMenu(Evas_Object* parent)
    {
        Evas_Coord w, h;
        evas_object_geometry_get(parent, NULL, NULL, &w, &h);

        elm_ctxpopup_direction_priority_set(m_ctxpopup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UP,
                ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UP);
        evas_object_move(m_ctxpopup, w/2, h);
        evas_object_show(m_ctxpopup);
    }

    Evas_Object *m_ctxpopup;
};

}//namespace interfaces
}//namespace tizen_browser

#endif /* __ABSTRACT_CONTEXT_MENU_H__ */
