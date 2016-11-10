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

#ifndef URLHISTORYLIST_TIMER_CALLBACKS_H_
#define URLHISTORYLIST_TIMER_CALLBACKS_H_

#include <Elementary.h>

namespace tizen_browser {
namespace base_ui {

class UrlHistoryList;
class GenlistManager;

/**
 * Used with EcoreTimerCaller. Resizes genlist according to the number of its
 * elements.
 */
class AdjustGenlistHeight
{
public:
    AdjustGenlistHeight();
    void set(Evas_Object* genlist, int itemsVisibleNumberMax, int itemH);
    void operator()() const;
private:
    Evas_Object* m_genlist;
    int m_items_visible_number_max;
    int m_item_h;
};

/**
 * Used with EcoreTimerCaller. Callback invoked when UrlHistoryList genlist
 * gains focus.
 */
class GenlistFocused
{
public:
    GenlistFocused();
    void set(UrlHistoryList* urlHistoryList);
    void operator()() const;
private:
    UrlHistoryList* m_urlHistoryList;
};

}
}

#endif /* URLHISTORYLIST_TIMER_CALLBACKS_H_ */
