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

#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <memory>
#include <Elementary.h>
#include <Ecore.h>
#include <Edje.h>
#include <Evas.h>
#include <string>
namespace tizen_browser
{
namespace base_ui
{


class MenuButton
{
public:

    struct ListSize {
        unsigned int width;
        unsigned int height;
    };

    MenuButton(std::shared_ptr<Evas_Object> mainWindow, Evas_Object* parentButton);
    virtual ~MenuButton();
    void showPopup();
    void hidePopup();
    /**
     * @brief Return content of the popup
     *
     * @return Evas_Object*
     */
    virtual Evas_Object *getContent() = 0;

    /**
     * @brief return size of displayed content.
     */
    virtual ListSize calculateSize() = 0;

    /**
     * @brief Returns the first item to switch focus to.
     *
     * If you have only elm_list, just return the list.
     *
     * @return Evas_Object*
     */
    virtual Evas_Object *getFirstFocus() = 0;

    /**
     * @brief Event fuction will be called just after popup is shown.
     *
     * Default implementation is empty.
     *
     * @return void
     */
    virtual void onPopupShow();

    virtual void onPopupDismissed();

    virtual bool canBeDismissed();

    bool isShown();

protected:
    static void dismissedCtxPopup(void *data, Evas_Object *obj, void *event_info);
    static MenuButton *previousPopup;
    Evas_Object *m_ctxPopup;
    std::string m_ctxPopupStyle;
    std::shared_ptr<Evas_Object> m_window;
    Evas_Object *m_parentButton;
private:
    void realShow(Evas_Object* popup);
    void unbindFocus();
    bool m_isShown;
};

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */

#endif // MENUBUTTON_H
