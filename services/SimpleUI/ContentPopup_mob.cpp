#include "ContentPopup_mob.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"
#include "Tools/EflTools.h"

namespace tizen_browser
{

namespace base_ui
{

ContentPopup* ContentPopup::createPopup(Evas_Object* parent)
{
    return new ContentPopup(parent);
}

ContentPopup* ContentPopup::createPopup(Evas_Object* parent, const std::string& title)
{
    return new ContentPopup(parent, title);
}

ContentPopup::~ContentPopup()
{
    buttonClicked.disconnect_all_slots();
    evas_object_del(m_layout);
}

ContentPopup::ContentPopup(Evas_Object* parent)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_buttons_box(nullptr)
    , m_content(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/ContentPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    createLayout();
}

ContentPopup::ContentPopup(Evas_Object* parent,
                     const std::string& title)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_buttons_box(nullptr)
    , m_title(title)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/ContentPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    createLayout();
}

void ContentPopup::show()
{
    evas_object_show(m_layout);
    elm_object_part_content_set(m_parent, "popup_content", m_layout);
    popupShown(this);
}

void ContentPopup::dismiss()
{
    popupDismissed(this);
}

void ContentPopup::onBackPressed()
{
    dismiss();
}

void ContentPopup::_response_cb(void* data,
                                  Evas_Object* obj,
                                  void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<ContentPopup*>(data);
    auto it = std::find_if(self->m_buttons.begin(),
                        self->m_buttons.end(),
                        [obj] (const Button& i)
                        { return i.m_evasObject == obj; }
                        );

    if (it == self->m_buttons.end()) {
        BROWSER_LOGW("[%s:%d] Button not found!", __PRETTY_FUNCTION__, __LINE__);
    } else {
        self->buttonClicked(it->m_type);
    }
    if (it->m_dismissOnClick)
        self->dismiss();
}

void ContentPopup::setTitle(const std::string& title)
{
    this->m_title = title;
    elm_object_translatable_part_text_set(m_layout, "title_text", m_title.c_str());
}

void ContentPopup::setContent(Evas_Object* content)
{
    m_content = content;
    elm_object_content_set(m_scroller, content);
}

void ContentPopup::addButton(const PopupButtons& button, bool dismissOnClick)
{
    auto btn = elm_button_add(m_buttons_box);
    m_buttons.push_back(Button(button, dismissOnClick, btn));
    elm_object_style_set(btn, "text-popup-button");
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(btn, "elm.text", _(buttonsTranslations[button].c_str()));
    elm_box_pack_end(m_buttons_box, btn);
    evas_object_smart_callback_add(btn, "clicked", _response_cb, (void*) this);

    evas_object_show(btn);
    if (m_buttons.size() > 1)      // separator for more buttons than one
        elm_object_signal_emit(btn, "visible", "ui");
}

void ContentPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_layout = elm_layout_add(m_parent);
    elm_object_tree_focus_allow_set(m_layout, EINA_FALSE);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "text_popup_layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_translatable_part_text_set(m_layout, "title_text", m_title.c_str());
    evas_object_event_callback_add(m_layout, EVAS_CALLBACK_RESIZE, _layout_resize_cb, this);

    m_buttons_box = elm_box_add(m_layout);
    elm_box_horizontal_set(m_buttons_box, EINA_TRUE);
    evas_object_size_hint_weight_set(m_buttons_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_buttons_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_buttons_box);
    elm_object_part_content_set(m_layout, "buttons_swallow", m_buttons_box);

    m_scroller = elm_scroller_add(m_layout);
    elm_object_part_content_set(m_layout, "content_swallow", m_scroller);
    evas_object_show(m_scroller);
}

void ContentPopup::_layout_resize_cb(void* data, Evas* /*e*/, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto self = static_cast<ContentPopup*>(data);
    int w;
    evas_object_geometry_get(self->m_layout, NULL, NULL, &w, NULL);
    w -= 2 * Z3_SCALE_SIZE(MARGIN);
    elm_label_wrap_width_set(self->m_content, w);
}


}

}
