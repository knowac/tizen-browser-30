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

#ifndef AUTOFILLFORMDELETEVIEW_H_
#define AUTOFILLFORMDELETEVIEW_H_

namespace tizen_browser{
namespace base_ui{

class AutoFillFormManager;
class AutoProfileDeleteView {

public:
    AutoProfileDeleteView(AutoFillFormManager* manager);
    ~AutoProfileDeleteView(void);
    Evas_Object* show(Evas_Object *parent, Evas_Object* action_bar = nullptr);
    void refreshView(void);
    void hide();

private:
    typedef struct _genlistCallbackData{
        unsigned int menu_index;
        AutoProfileDeleteView *user_data;
        Elm_Object_Item *it;
    } genlistCallbackData;

    Evas_Object *createMainLayout(Evas_Object *parent);
    Evas_Object *createGenlist(Evas_Object *parent);
    Eina_Bool appendGenlist(Evas_Object *genlist);

    const char *getEachItemFullName(unsigned int index);
    void deleteSelectedItems(void);
    void deleteAllItems(void);
    static char *__text_get_cb(void* data, Evas_Object* obj, const char *part);
    static Evas_Object *__content_get_cb(void* data, Evas_Object* obj, const char *part);
    static void __genlist_item_selected_cb(void* data, Evas_Object* obj, void* event_info);
    static void __select_all_checkbox_changed_cb(void* data, Evas_Object* obj, void* event_info);
    static void __delete_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __back_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __checkbox_changed_cb(void* data, Evas_Object* obj, void* event_info);

    AutoFillFormManager* m_manager;
    Evas_Object *m_parent;
    Evas_Object *m_back_button;
    Evas_Object *m_del_button;
    Evas_Object *m_checkbox;
    Evas_Object *m_mainLayout;
    Evas_Object *m_genlist;
    Evas_Object *m_action_bar;
    Elm_Genlist_Item_Class *m_itemClass;
    unsigned m_checked_count;
    std::map<Elm_Object_Item*, bool> m_checkbox_delete_state_map;

    std::string m_edjFilePath;
};

}
}

#endif /* AUTOFILLFORMDELETEVIEW_H_ */
