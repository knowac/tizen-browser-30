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

#ifndef FIND_ON_PAGE_H
#define FIND_ON_PAGE_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser{
namespace base_ui{

struct FindData {
    const char* input_str;
    bool forward;
    Evas_Smart_Cb func;
    void* data;
};

class BROWSER_EXPORT FindOnPageUI
    : public tizen_browser::interfaces::AbstractUIComponent
    , public tizen_browser::core::AbstractService
{
public:
    virtual std::string getName();

    FindOnPageUI();
    ~FindOnPageUI(void);

    Evas_Object* getContent();
    void init(Evas_Object* parent);
    Evas_Object *get_layout(void) { return m_fop_layout; }
    Evas_Object* createFindOnPageUILayout();
    void show();
    void showUI();
    void hideUI();
    bool isVisible() { return m_isVisible; }

    void show_ime(void);
    void clear_text(void);
    void set_text(const char *text);
    Evas_Object* get_entry(void) { return m_entry; }
    void unset_focus(void);

    boost::signals2::signal<void ()> closeFindOnPageUIClicked;
    boost::signals2::signal<void (const struct FindData& )> startFindingWord;

private:
    void _set_count(int index, int total);
    void _disable_down_button(Eina_Bool disable);
    void _disable_up_button(Eina_Bool disable);

    static void __clear_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void __down_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void __up_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void __enter_key_cb(void *data, Evas_Object *obj, void *event_info);
    static void __entry_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
    static void __text_found_cb(void *data, Evas_Object *obj, void *event_info);
    static void __close_clicked_cb(void *data, Evas_Object *obj, void *event_info);

    Evas_Object *m_fop_layout;
    Evas_Object *m_entry;
    Evas_Object *m_down_button;
    Evas_Object *m_up_button;
    Evas_Object *m_clear_button;
    Evas_Object *m_parent;

    int m_total_count;
    int m_current_index;
    const char *m_input_word;
    std::string m_edjFilePath;
    bool m_isVisible;
};

}
}
#endif /* FIND_ON_PAGE_H */

