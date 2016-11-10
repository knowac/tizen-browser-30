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

#ifndef FOCUSMANAGER_H
#define FOCUSMANAGER_H

#include <Evas.h>
#include <Ecore.h>
#include <Elementary.h>
#include <vector>

class FocusManager
{
public:
    FocusManager();
    ~FocusManager();

    /**
     *  This function runs FocusManager. Set the gengrid as the parameter of this function to run _raw_tracker. If the view does not have gengrid, set nullptr.
     *  Before you call this function, add Evas_Object elements to verticalFocusVector using addItem(Evas_Object*) function and set focusListIterator using setIterator() function.
     */
    void startFocusManager(Evas_Object*);

    /**
     *  This function stops FocusManager.
     */
    void stopFocusManager();

    /**
     *  Use this function to add Evas_Object to FocusManager. The order matters.
     */
    void addItem(Evas_Object*);

    /**
     *  Use this function set focusListIterator. Call it after adding items. This function sets iterator at the beginnig of the verticalFocusVector.
     */
    void setIterator();
private:
    static Eina_Bool _key_down_cb(void *data, int type, void *event);
    static void _row_tracker(void *data, Evas_Object *obj, void *event);

    std::vector<Evas_Object*> verticalFocusVector;
    std::vector<Evas_Object*>::iterator focusListIterator;
    unsigned int _rowTracker, _prevRowTracker;
    Ecore_Event_Handler* handlerDown;
    Evas_Object* _gen;
};

#endif // FOCUSMANAGER_H

