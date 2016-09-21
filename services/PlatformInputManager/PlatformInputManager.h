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
 */

#ifndef PLATFORMINPUTMANAGER_H
#define PLATFORMINPUTMANAGER_H

#include <boost/signals2/signal.hpp>
#include <boost/thread/thread.hpp>
#include <Ecore.h>
#include <Eina.h>
#include <Elementary.h>

#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser
{
namespace services
{

class BROWSER_EXPORT PlatformInputManager : public tizen_browser::core::AbstractService
{
public:
    /**
     * @brief Default constructor with variable initialization.
     */
    PlatformInputManager();

    /**
     * @brief Initialization of the object, adding event filter, setting cursor to be always visible.
     */
    void init(Evas_Object *mainWindow);

    /**
     * Signals emitted after certain button on keyboard/remote controller press.
     */
    boost::signals2::signal<void ()> menuPressed;
    boost::signals2::signal<void ()> returnPressed;
    boost::signals2::signal<void ()> enterPressed;
    boost::signals2::signal<void ()> leftPressed;
    boost::signals2::signal<void ()> rightPressed;
    boost::signals2::signal<void ()> backPressed;
    boost::signals2::signal<void ()> escapePressed;
#if PROFILE_MOBILE
    boost::signals2::signal<void ()> XF86BackPressed;
    boost::signals2::signal<void ()> menuButtonPressed;
#else
    boost::signals2::signal<void ()> redPressed;
    boost::signals2::signal<void ()> greenPressed;
    boost::signals2::signal<void ()> yellowPressed;
    boost::signals2::signal<void ()> bluePressed;
#endif
    boost::signals2::signal<void (int, int)> mouseClicked;

    /**
     * @brief Returns current service's name.
     */
    virtual std::string getName();

#if PROFILE_MOBILE
    void unregisterHWKeyCallback(Evas_Object* view);
    void registerHWKeyCallback(Evas_Object* view);
#endif

private:
    /**
     * @brief Struct holding parameters of mouse movement.
     * It is used in pointer mode to simulate mouse move after pressing arrows.
     */
    struct MouseMovementParams {
        bool moveMousePointer;
        int xMod, yMod;
        int counter, speed;
    };

#if PROFILE_MOBILE
    bool m_HWKeyCallbackRegistered;
    static void onHWBack(void* data, Evas_Object*, void*);
    static void onHWMore(void* data, Evas_Object*, void*);
#endif

    /**
     * @brief It process every input event and handles it if necessary.
     */
    static Eina_Bool __filter(void */*data*/, void */*loop_data*/, int /*type*/, void */*event*/);
};

}
}

#endif // PLATFORMINPUTMANAGER_H
