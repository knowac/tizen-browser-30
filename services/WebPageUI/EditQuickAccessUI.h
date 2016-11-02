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

#ifndef EDITQUICKACCESSUI
#define EDITQUICKACCESSUI

#include <Evas.h>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "NaviframeWrapper.h"
#include "QuickAccess.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT EditQuickAccessUI
    : public tizen_browser::interfaces::AbstractUIComponent
{
public:
    EditQuickAccessUI();
    ~EditQuickAccessUI();
    //AbstractUIComponent interface implementation
    void showUI();
    void hideUI();
    void init(Evas_Object *parent);
    Evas_Object* getContent();
    void backPressed();
    void setMVSelectedItems(int count);

    boost::signals2::signal<Evas_Object* ()> requestQuickAccessGengrid;
    boost::signals2::signal<Evas_Object* ()> requestMostVisitedGengrid;
    boost::signals2::signal<QuickAccessState ()> requestQuickAccessState;
    boost::signals2::signal<void ()> editingFinished;
    boost::signals2::signal<void ()> deleteSelectedMostVisitedItems;

private:

    static void _cancel_clicked(void *data, Evas_Object *, void *);
    static void _done_clicked(void *data, Evas_Object *, void *);

    void createEditLayout();
    SharedNaviframeWrapper m_naviframe;
    Evas_Object *m_parent;
    Evas_Object *m_layout;
    std::shared_ptr<QuickAccess> m_quickAccess;
    QuickAccessState m_editState;
};

}   // namespace tizen_browser
}   // namespace base_ui

#endif // EDITQUICKACCESSUI

