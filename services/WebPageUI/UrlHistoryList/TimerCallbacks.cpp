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

#include "TimerCallbacks.h"

#include "BrowserLogger.h"
#include "UrlHistoryList.h"
#include "GenlistManager.h"
#include "GenlistItemsManager.h"

namespace tizen_browser {
namespace base_ui {

AdjustGenlistHeight::AdjustGenlistHeight()
    : m_genlist(nullptr)
    , m_items_visible_number_max(0)
    , m_item_h(0)
{
}

void AdjustGenlistHeight::set(Evas_Object* genlist, int itemsVisibleNumberMax,
        int itemH)
{
    m_genlist = genlist;
    m_items_visible_number_max = itemsVisibleNumberMax;
    m_item_h = itemH;
}

void AdjustGenlistHeight::operator()() const
{
    if (!m_genlist)
        return;
    const int LIST_ITEMS_NUMBER = elm_genlist_items_count(m_genlist);
    if (LIST_ITEMS_NUMBER == 0)
        return;
    int historyItemsVisibleCurrent = m_items_visible_number_max;
    if (LIST_ITEMS_NUMBER < historyItemsVisibleCurrent)
        historyItemsVisibleCurrent = LIST_ITEMS_NUMBER;
    Evas_Coord w, h;
    evas_object_geometry_get(m_genlist, nullptr, nullptr, &w, nullptr);
    h = m_item_h * historyItemsVisibleCurrent;
    evas_object_resize(m_genlist, w, h);
}

GenlistFocused::GenlistFocused()
    : m_urlHistoryList(nullptr)
{
}

void GenlistFocused::set(UrlHistoryList* urlHistoryList)
{
    m_urlHistoryList = urlHistoryList;
}

void GenlistFocused::operator ()() const
{
    m_urlHistoryList->onListWidgetFocusChange(true);
}

}
}
