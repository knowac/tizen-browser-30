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
 * ViewManager.h
 *
 *  Created on: Sep 11, 2015
 *      Author: m.lapinski@samsung.com
 */

#ifndef VIEWMANAGER_H_
#define VIEWMANAGER_H_

#include <Evas.h>

#include <stack>
#include <boost/signals2/signal.hpp>

#include "core/AbstractInterfaces/AbstractUIComponent.h"

namespace tizen_browser{
namespace base_ui{

using AUI = interfaces::AbstractUIComponent;
using sAUI = std::shared_ptr<AUI>;

/**
 * @brief This class simplifies UI component management. It is a views stack.
 * It handles all widget framework issues related to changing active view.
 */
class ViewManager
{
public:
/**
 * @brief constructor.
 */
    ViewManager();

/**
 * @brief initialization method
 *
 * @param A window which will contatin ViewManager's main layout.
 */
    void init(Evas_Object* parentWindow);

/**
 * @brief destructor
 *
 */
    ~ViewManager();

/**
 * @brief Pops stack to specified view. Hides actual view (if there is any) and
 *        make specified view visible. Does nothing if stack is empty. If view
 *        is not in the stack, it pops whole stack.
 *
 * @param A view which stack should be popped to. Do not use nullptr.
 */
    void popStackTo(const sAUI& view);

/**
 * @brief Pops actual view from the stack, hides it and if there is any view
 *        under it makes it visible.
 */
    void popTheStack();

/**
 * @brief Pushes view to the stack, hides if any wiew was visible hides it and
 *        new one visible visible.
 *
 * @param View pushed to stack. Do not use nullptr.
 */
    void pushViewToStack(const sAUI& view);

/**
 * @brief Function returns elm layout used in view management. It's parent is
 * specified in constructor.
 *
 * @return ViewManager's main layout.
 */
    Evas_Object* getContent();

/**
 * @brief Function returns conformant used in view management.
 *
 * @return ViewManager's conformant.
 */
    Evas_Object* getConformant();

/**
 * @brief Returns actual top of stack which is dispalyed. It stack is empty
 * returns null.
 *
 * @return actual dispalyed view
 */
    sAUI& topOfStack();

/**
 * @brief Signal checks if browser is in landscape mode.
 *
 * @return Returnes true when app is in landscape mode.
 */
    boost::signals2::signal<bool ()> isLandscape;

private:
    void updateLayout(const sAUI& previousView);
private:
    Evas_Object* m_mainLayout;
    Evas_Object* m_conformant;
    Evas_Object* m_parentWindow;
    std::stack<sAUI> m_viewStack;

    const int BG_COLOR_R = 61;
    const int BG_COLOR_G = 184;
    const int BG_COLOR_B = 204;
};

}//namespace base_ui
}//namespace tizen_browser
#endif //VIEWMANAGER_H_
