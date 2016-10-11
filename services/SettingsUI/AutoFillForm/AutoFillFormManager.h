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

#ifndef AUTOFILLFORM_H_
#define AUTOFILLFORM_H_

#include <string>
#include <vector>
#include <boost/signals2/signal.hpp>
#include <Ecore.h>
#include <Elementary.h>
#include <Evas.h>

#include <EWebKit.h>
#include <EWebKit_internal.h>

#define AUTO_FILL_FORM_ENTRY_MAX_COUNT 1024
#define PHONE_FIELD_VALID_ENTRIES "0123456789*#()/N,.;+ "

namespace tizen_browser{
namespace base_ui{

class AutoFillFormComposeView;
class AutoFillFormListView;
class AutoProfileDeleteView;
class AutoFillFormItem;

struct _AutoFillFormItemData;
typedef _AutoFillFormItemData AutoFillFormItemData;

class AutoFillFormManager
{
public:
    AutoFillFormManager(void);
    ~AutoFillFormManager(void);

    void init(Evas_Object* parent, Evas_Object* action_bar) { m_parent = parent; m_action_bar = action_bar;}
    void refreshListView();
    Eina_Bool saveAutoFillFormItem(AutoFillFormItemData *item_data);
    Eina_Bool deleteAutoFillFormItem(AutoFillFormItem *item);
    Eina_Bool deleteAllAutoFillFormItems(void);
    unsigned int getAutoFillFormItemCount(void);
    AutoFillFormItem *createNewAutoFillFormItem(Ewk_Autofill_Profile *profile = NULL);
    Evas_Object* showListView(void);
    Evas_Object* showDeleteView(void);
    Evas_Object* showComposeView(AutoFillFormItem *item = NULL);
    AutoFillFormListView *getListView(void) { return m_listView; }
    AutoFillFormComposeView *getComposeView(void) { return m_composer; }
    Eina_Bool deleteListView(void);
    Eina_Bool deleteDeleteView(void);
    Eina_Bool deleteComposer(void);
    std::vector<AutoFillFormItem *> getItemList(void) { return m_AutoFillFormItemList; }
    AutoFillFormItem* getItem(unsigned it){ return m_AutoFillFormItemList[it];}
    std::vector<AutoFillFormItem *> loadEntireItemList(void);
    Eina_Bool addItemToList(AutoFillFormItem *item);
    void rotateLandscape();
    void rotatePortrait();

    static Eina_Bool load_list_timer(void *data);

    boost::signals2::signal<void ()> listViewBackClicked;

    /* test */
    void seeAllData(void);
private:
    Evas_Object* m_parent;
    Evas_Object* m_action_bar;
    std::vector<AutoFillFormItem *> m_AutoFillFormItemList;
    AutoFillFormListView *m_listView;
    AutoFillFormComposeView *m_composer;
    AutoProfileDeleteView *m_deleteView;
    Ewk_Context* m_ewkContext;
    Ecore_Timer* m_timer;
};

}
}

#endif
