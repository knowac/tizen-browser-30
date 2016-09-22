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

#ifndef AUTOFILLFORMLISTVIEW_H_
#define AUTOFILLFORMLISTVIEW_H_

namespace tizen_browser{
namespace base_ui{

class AutoFillFormComposeView;
class AutoFillFormListView;
class AutoProfileDeleteView;
class AutoFillFormItem;
class AutoFillFormManager;

class AutoFillFormListView {
public:
    AutoFillFormListView(AutoFillFormManager *affm);
    ~AutoFillFormListView(void);

    Evas_Object* show(Evas_Object *parent, Evas_Object* action_bar = nullptr);
    void hide();
    void refreshView(void);
    void rotateLandscape();
    void rotatePortrait();
private:
    typedef struct _genlistCallbackData {
        unsigned int menu_index;
        void *user_data;
        Elm_Object_Item *it;
    } genlistCallbackData;

    Evas_Object *createMainLayout(Evas_Object *parent);
    Evas_Object *createGenlist(Evas_Object *parent);
    Eina_Bool createGenlistStyleItems(void);
    const char *getEachItemFullName(unsigned int index);
    Eina_Bool appendGenlist(Evas_Object *genlist);
    AutoProfileDeleteView *getDeleteView(void);
    static char *__text_get_cb(void* data, Evas_Object* obj, const char *part);
    static Evas_Object *__content_get_cb(void* data, Evas_Object* obj, const char *part);
    static void __check_changed_cb(void* data, Evas_Object* obj, void* event_info);
    static void __add_profile_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __back_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __genlist_item_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void __delete_profile_button_cb(void* data, Evas_Object* obj, void* event_info);

    AutoFillFormManager *m_manager;
    Evas_Object *m_parent;
    Evas_Object *m_mainLayout;
    Evas_Object *m_add_btn;
    Evas_Object *m_del_btn;
    Evas_Object *m_genlist;
    Evas_Object *m_action_bar;
    Elm_Genlist_Item_Class *m_itemClass;
    std::string m_edjFilePath;
};

}
}

#endif /* AUTOFILLFORMLISTVIEW_H_ */
