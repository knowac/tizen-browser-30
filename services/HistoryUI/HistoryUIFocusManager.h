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

#ifndef HISTORYUIFOCUSMANAGER_H_
#define HISTORYUIFOCUSMANAGER_H_

#include <memory>
#include <Elementary.h>

namespace tizen_browser{
namespace base_ui{

class HistoryDaysListManager;
using HistoryDaysListManagerPtr = std::shared_ptr<HistoryDaysListManager>;

class HistoryUIFocusManager
{
public:
    HistoryUIFocusManager(HistoryDaysListManagerPtr historyDaysListManager);
    virtual ~HistoryUIFocusManager();
    void setFocusObj(Evas_Object* obj);
    void refreshFocusChain();
    void unsetFocusChain();
    void setHistoryUIButtons(Evas_Object* buttonClose, Evas_Object* buttonClear);

private:
    Evas_Object* m_focusObject{nullptr};
    HistoryDaysListManagerPtr m_historyDaysListManager;

    Evas_Object* m_buttonClose{nullptr};
    Evas_Object* m_buttonClear{nullptr};
};

}
}

#endif /* HISTORYUIFOCUSMANAGER_H_ */
